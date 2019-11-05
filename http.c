#include "types.h"
#include "system.h"
#include "http.h"
#include "stdio.h"
#include "tcpip.h"
#include "version.h"
#include "utils.h"
#include "msxdos.h"
#include <string.h>


static byte automaton_state;
static char* error_buffer;
static byte data_buffer[256+1];
static byte* data_buffer_pointer;
static int data_buffer_length;
static bool skipping_data;
static int last_system_timer_value;
static int ticks_without_data;
static byte output_data_buffer[512];
static int output_data_length;
static int server_port;
static bool server_verbose_mode;
static int client_inactivity_timeout;
static char filename_buffer[MAX_FILE_PATH_LEN*2];
static char* files_base_directory;
static byte file_handle;
static fileInfoBlock file_fib;
static fileInfoBlock dir_list_fib;
static bool send_as_attachment;
static bool has_if_modified_since;
static dateTime if_modified_since_date;
static bool directory_listing_enabled;
static byte base_directory_length;
static byte buffer[64];
static byte buffer2[32];

static char num_buffer[11];
static char content_length_buffer[32];

static const char* default_document = "\\INDEX.HTM";
static const char* connection_lost_str = "Connection lost\r\n";
static const char* connection_closed_by_client_str = "Connection closed by client\r\n";

//Note that this one has the chunked transfer size hardcoded at the beginning.
//Don't do this at home, kids.
static const char* dir_list_header_1 = 
    "1B1\r\n"
    "<html>"
    "<head>"
    "<style type='text/css'>"
    "body {font-family: sans-serif; color: white; background-color: blue;} "
    "p {font-size: small; color: lightgray; font-style: italic; margin-top: 20px;} "
    "table {font-family: monospace;} "
    "td {border: 0px solid black;} "
    "td:nth-of-type(1) {padding-right: 30px; font-weight: bold;} "
    "td:nth-of-type(2) {text-align: right;} "
    "td:nth-of-type(3) {padding-left: 30px;} "
    "a { color: white; text-decoration: none;}"
    "</style>"
    "\r\n";

static const char* dir_list_header_2 = 
    "<title>%s - NestorHTTP</title>"
    "</head>"
    "<body>"
    "<h1>Directory of %s</h1>"
    "<table>";

static const char* dir_list_footer = 
    "</table>"
    "<p>NestorHTTP " VERSION "</p>"
    "</body>"
    "</html>";

static const char* dir_list_entry =
    "<tr>"
    "<td><a href=\"%s%s\">%s%s</td>"
    "<td>%s</td>"
    "<td>%s</td>"
    "</tr>";

static const char* empty_str = "";

static void InitializeDataBuffer();
static void OpenServerConnection();
static void HandleIncomingConnectionIfAvailable();
static void CloseConnectionToClient();
static void ContinueReadingRequest();
static bool ProcessRequestLine();
static bool ContinueReadingLine();
static void ResetInactivityCounter();
static void UpdateInactivityCounter();
static void ContinueReadingHeaders();
static void ProcessHeader();
static void SendLineToClient(char* line);
static void SendResponseStart(int statusCode, char* statusMessage);
static void SendHtmlResponseToClient(int statusCode, char* statusMessage, char* content);
static void SendErrorResponseToClient(int statusCode, char* statusMessage, char* detailedMessage);
static char* ConvertRequestToFilename(char** query_string_start);
static void SendNotFoundError();
static void SendInternalError();
static void ProcessFileOrDirectoryRequest();
static void StartSendingFile();
static void SendContentLengthHeader(ulong length);
static void ContinueSendingFile();
static void StartSendingDirectory();
static void ContinueSendingDirectory();
static void FinishSendingDirectory();
static void PrepareChunkedData(char* data);

extern void _ultoa(long val, char* buffer, char base);

#define PrintUnlessSilent(s) { if(server_verbose_mode > VERBOSE_MODE_SILENT) printf(s); }


static void InitializeDataBuffer()
{
    data_buffer_pointer = data_buffer; 
    data_buffer_length = 0;
    skipping_data = 0;
    ResetInactivityCounter();
}


void InitializeHttpAutomaton(char* base_directory, char* http_error_buffer, uint port, byte verbose_mode, int inactivity_timeout_in_ticks, bool enable_directory_listing)
{
    server_port = port;
    server_verbose_mode = verbose_mode;
    client_inactivity_timeout = inactivity_timeout_in_ticks;
    directory_listing_enabled = enable_directory_listing;

    files_base_directory = base_directory;
    base_directory_length = strlen(base_directory) - 1;
    error_buffer = http_error_buffer;
    error_buffer[0] = '\0';

    file_handle = 0;
    automaton_state = HTTPA_NONE;
}


void CleanupHttpAutomaton()
{
    server_verbose_mode = VERBOSE_MODE_SILENT;
    CloseConnectionToClient();
}


void DoHttpServerAutomatonStep()
{
    if(ticks_without_data >= client_inactivity_timeout)
    {
        PrintUnlessSilent("Closing connection for client inactivity\r\n");

        CloseConnectionToClient();
    }

    switch(automaton_state)
    {
        case HTTPA_NONE:
            OpenServerConnection();
            break;
        case HTTPA_LISTENING:
            HandleIncomingConnectionIfAvailable();
            break;
        case HTTPA_READING_REQUEST:
            ContinueReadingRequest();
            break;
        case HTTPA_READING_HEADERS:
            ContinueReadingHeaders();
            break;
        case HTTPA_SENDING_FILE_CONTENTS:
            ContinueSendingFile();
            break;
        case HTTPA_SENDING_DIRECTORY_LISTING_HEADER_1:
        case HTTPA_SENDING_DIRECTORY_LISTING_HEADER_2:
            StartSendingDirectory();
            break;
        case HTTPA_SENDING_DIRECTORY_LISTING_ENTRIES:
            ContinueSendingDirectory();
            break;
        case HTTPA_SENDING_DIRECTORY_LISTING_FOOTER:
            FinishSendingDirectory();
            break;
    }
}


static void OpenServerConnection()
{
    byte error;

    error = OpenPassiveTcpConnection(server_port);
    if(error == ERR_NO_NETWORK)
    {
        sprintf(error_buffer, "No network connection");
        return;
    }
    else if(error != 0)
    {
        sprintf(error_buffer, "Unexpected error when opening TCP connection: %i", error);
        return;
    }

    automaton_state = HTTPA_LISTENING;
}


static void HandleIncomingConnectionIfAvailable()
{
    byte status;

    status = GetSimplifiedTcpConnectionState();
    switch(status)
    {
        case TCP_STATE_CLOSED:
            CloseConnectionToClient();
            break;
        
        case TCP_STATE_ESTABLISHED:
            PrintUnlessSilent("Connection received!\r\n");

            InitializeDataBuffer();
            automaton_state = HTTPA_READING_REQUEST;
            break;
        
        case TCP_STATE_CLOSE_WAIT:
            //That would be weird, but can happen if the client
            //opens and closes the connection in quick succession
            CloseConnectionToClient();
            break;
    }
}


static void CloseConnectionToClient()
{
    ResetInactivityCounter();
    automaton_state = HTTPA_NONE;

    if(CloseTcpConnection())
        PrintUnlessSilent("I have closed the connection with the client\r\n");

    if(file_handle != 0)
    {
        CloseFile(file_handle);
        file_handle = 0;
    }

    has_if_modified_since = false;
}


static void ContinueReadingRequest()
{
    byte status;
    bool lineComplete;

    status = GetSimplifiedTcpConnectionState();
    switch(status)
    {
        case TCP_STATE_CLOSED:
            PrintUnlessSilent(connection_lost_str);
            CloseConnectionToClient();
            return;
        
        case TCP_STATE_CLOSE_WAIT:
            PrintUnlessSilent(connection_closed_by_client_str);
            CloseConnectionToClient();
            return;
    }


    lineComplete = ContinueReadingLine();
    if(!lineComplete)
        return;

    if(ProcessRequestLine())
    {
        InitializeDataBuffer();
        automaton_state = HTTPA_READING_HEADERS;
    }
    else
    {
        CloseConnectionToClient();
    }
}


static bool ProcessRequestLine()
{
    char* converted_filename;
    char* query_string;

    if(server_verbose_mode > VERBOSE_MODE_SILENT)
        printf("Request: %s\r\n", data_buffer);

    if(strlen(data_buffer) == sizeof(data_buffer)-1)
    {
        PrintUnlessSilent("ERROR: Request line too long, connection refused\r\n");
        return false;
    }

    if(!strncmpi(data_buffer, "GET", 3))
    {
        SendErrorResponseToClient(405, "Method Not Allowed", "Sorry, this is a GET only server for now");
        return false;
    }

    converted_filename = ConvertRequestToFilename(&query_string);
    send_as_attachment = query_string && strncmpi(query_string, "a=1", 3);

    if(converted_filename)
    {
        strcpy(filename_buffer, files_base_directory);
        strcat(filename_buffer, converted_filename);
        return true;
    }
    else
    {
        SendNotFoundError();
        return false;
    }
}


static bool ContinueReadingLine()
{
    byte datum;

    while(EnsureIncomingTcpDataIsAvailable())
    {
        ResetInactivityCounter();
        datum = GetIncomingTcpByte();

        if(datum == '\n')
            continue;

        if(datum != '\r')
        {
            if(!skipping_data)
            {
                *data_buffer_pointer++ = datum;
                data_buffer_length++;

                if(data_buffer_length >= sizeof(data_buffer)-1)
                {
                    skipping_data = true;
                    PrintUnlessSilent("* WARNING: Line too long, truncating\r\n");
                }
            }

            continue;
        }

        *data_buffer_pointer = '\0';
        skipping_data = false;
        return true;
    }

    UpdateInactivityCounter();
    return false;
}


static void ResetInactivityCounter()
{
    last_system_timer_value = SYSTEM_TIMER;
    ticks_without_data = 0;
}


static void UpdateInactivityCounter()
{
    if(SYSTEM_TIMER == last_system_timer_value)
        return;
    
    ticks_without_data++;
}


static void ContinueReadingHeaders()
{
    bool lineComplete;
    bool status;

    status = GetSimplifiedTcpConnectionState();
    switch(status)
    {
        case TCP_STATE_CLOSED:
            PrintUnlessSilent(connection_lost_str);
            automaton_state = HTTPA_NONE;
            return;
        
        case TCP_STATE_CLOSE_WAIT:
            PrintUnlessSilent(connection_closed_by_client_str);
            CloseTcpConnection();
            automaton_state = HTTPA_NONE;
            return;
    }

    lineComplete = ContinueReadingLine();
    if(!lineComplete)
        return;

    if(data_buffer_length > 0)
        ProcessHeader();
    else
        ProcessFileOrDirectoryRequest();
}


static void ProcessHeader()
{
    if(server_verbose_mode > VERBOSE_MODE_CONNECTIONS)
        printf("<-- %s\r\n", data_buffer);

    if(strncmpi(data_buffer, "If-Modified-Since:", 18))
    {
        has_if_modified_since = ParseVerboseDateTime(&data_buffer[18], &if_modified_since_date);
    }

    InitializeDataBuffer();
}


static void SendLineToClient(char* line)
{
    sprintf(data_buffer, "%s\r\n", line);
    if(server_verbose_mode > VERBOSE_MODE_CONNECTIONS)
        printf("--> %s", data_buffer);

    SendStringToTcpConnection(data_buffer);
}


static void SendResponseStart(int statusCode, char* statusMessage)
{
    char buffer[64];

    if(server_verbose_mode > VERBOSE_MODE_SILENT)
        printf("Response: %i %s\r\n", statusCode, statusMessage);

    sprintf(buffer, "HTTP/1.1 %i %s", statusCode, statusMessage);
    SendLineToClient(buffer);
    SendLineToClient("Connection: close");
    SendLineToClient("Server: NestorHTTP/" VERSION " (MSX-DOS)");
}


static void SendHtmlResponseToClient(int statusCode, char* statusMessage, char* content)
{
    SendResponseStart(statusCode, statusMessage);
    if(statusCode > 300 && statusCode < 400)
    {
        SendLineToClient(empty_str);
        return;
    }

    SendContentLengthHeader(content ? strlen(content) : 0);

    if(content)
    {
        SendLineToClient("Content-Type: text/html");
        SendLineToClient(empty_str);
        if(server_verbose_mode > VERBOSE_MODE_CONNECTIONS)
            printf("--> (HTML response)\r\n");
        SendStringToTcpConnection(content);
    }
    else
    {
        SendLineToClient(empty_str);
    }
}


static void SendErrorResponseToClient(int statusCode, char* statusMessage, char* detailedMessage)
{
    if(!detailedMessage)
    {
        SendHtmlResponseToClient(statusCode, statusMessage, null);
        return;
    }

    sprintf(
        output_data_buffer,
            "<html>"
            "<head>"
            "<title>NestorHTTP - error</title>"
            "<style type='text/css'>body {font-family: sans-serif;} .footer {font-size: small; color: gray; font-style: italic;}</style>"
            "</head>"
            "<body>"
            "<h1>%i %s</h1>"
            "<p>%s</p>"
            "<p class='footer'>NestorHTTP " VERSION "</p>"
            "</body>"
            "</html>",
        statusCode,
        statusMessage,
        detailedMessage
        );
    
    SendHtmlResponseToClient(statusCode, statusMessage, output_data_buffer);
}


static char* ConvertRequestToFilename(char** query_string_start)
{
    char* pointer;
    char* start_pointer;
    char ch;
    bool last_char_was_dot;

    pointer = data_buffer;
    *query_string_start = null;

    //Skip the method and the initial "/"
    while(*pointer != ' ') pointer++;
    while(*pointer == ' ') pointer++;
    if(*pointer == '/') pointer++;

    start_pointer = pointer;

    last_char_was_dot = false;
    while((ch = *pointer) > ' ' && ch != QUERY_STRING_SEPARATOR)
    {
        if(ch == '/')
        {
            *pointer = '\\';
        }
        else if(ch == '.')
        {
            //Don't allow ".." to prevent access to outside the base directory
            if(last_char_was_dot)
                return null;
            
            last_char_was_dot = true;
        }
        else
        {
            last_char_was_dot = false;
        }
        
        pointer++;
    }

    if(ch == QUERY_STRING_SEPARATOR)
        *query_string_start = pointer+1;

    if(pointer[-1] == '\\')
        pointer--;

    *pointer = '\0';
    
    return start_pointer == pointer && !directory_listing_enabled ? default_document+1 : start_pointer;
}


static void SendNotFoundError()
{
    SendErrorResponseToClient(404, "Not Found", "The requested resource was not found in this server");
}


static void SendInternalError()
{
    SendErrorResponseToClient(500, "Internal Server Error", "Sorry, something went wrong. It's not you, it's me.");
}


static void ProcessFileOrDirectoryRequest()
{
    byte error;
    bool is_directory;

    is_directory = false;

    error = SearchFile(filename_buffer, &file_fib, true);

    if(error == 0 && (file_fib.attributes & FILE_ATTR_DIRECTORY))
    {
        if(directory_listing_enabled)
        {
            is_directory = true;
        }
        else
        {
            strcat(filename_buffer, default_document);
            error = SearchFile(filename_buffer, &file_fib, false);
        }
    }

    if(error == ERR_NOFIL || error == ERR_NODIR)
    {
        SendNotFoundError();
        CloseConnectionToClient();
        return;
    }
    else if(error >= MIN_DISK_ERROR_CODE || error == ERR_NHAND || error == ERR_NORAM)
    {
        if(server_verbose_mode > VERBOSE_MODE_SILENT)
            printf("*** Error searching file: %xh\r\n", error);

        SendInternalError();
        CloseConnectionToClient();
        return;
    }
    else if(error != 0)
    {
        SendErrorResponseToClient(400, "Bad Request", "Sorry, I'm not able to process your request.");
        CloseConnectionToClient();
        return;
    }

    if(is_directory)
    {
        PrintUnlessSilent("Sending directory listing...\r\n");
        SendResponseStart(200, "Ok");
        SendLineToClient("Content-Type: text/html");
        SendLineToClient("Transfer-Encoding: chunked");
        SendLineToClient(empty_str);

        automaton_state = HTTPA_SENDING_DIRECTORY_LISTING_HEADER_1;
        output_data_length = 0;
        StartSendingDirectory();
    }
    else
    {
        StartSendingFile();
    }
}


static void StartSendingFile()
{
    byte error;
    dateTime date_time;

    if(file_fib.dateOfModification != 0)
    {
        ParseFibDateTime(&file_fib, &date_time);

        if(has_if_modified_since && CompareDates(&date_time, &if_modified_since_date) <= 0)
        {
            SendErrorResponseToClient(304, "Not Modified", null);
            CloseConnectionToClient();
            return;
        }
    }

    error = OpenFile(&file_fib, &file_handle);
    if(error)
    {
        if(server_verbose_mode > VERBOSE_MODE_SILENT)
            printf("*** Error opening file: %xh\r\n", error);

        SendInternalError();
        CloseConnectionToClient();
        return;
    }

    SendResponseStart(200, "Ok");
    SendContentLengthHeader(file_fib.fileSize);
    
    if(send_as_attachment)
    {
        sprintf(buffer, "Content-Disposition: attachment; filename=\"%s\"", file_fib.filename);
        SendLineToClient(buffer);
    }

    if(file_fib.dateOfModification != 0)
    {
        //Note that we already ran ParseFibDateTime earlier
        ToVerboseDate(buffer2, &date_time);
        sprintf(buffer, "Last-Modified: %s", buffer2);
        SendLineToClient(buffer);
    }

    SendLineToClient(empty_str);

    output_data_length = 0;
    PrintUnlessSilent("Sending file contents...\r\n");
    automaton_state = HTTPA_SENDING_FILE_CONTENTS;

    ContinueSendingFile();
}


static void SendContentLengthHeader(ulong length)
{
    _ultoa(length, num_buffer, 10);
    sprintf(content_length_buffer, "Content-Length: %s", num_buffer);
    SendLineToClient(content_length_buffer);
}


static void ContinueSendingFile()
{
    byte error;

    while(true) //Continue while there's more data to send AND the TCP connection has room for more output data
    {
        if(GetSimplifiedTcpConnectionState() == TCP_STATE_CLOSED)
        {
            PrintUnlessSilent(connection_lost_str);
            CloseConnectionToClient();
            return;
        }

        if(output_data_length == 0)
        {
            output_data_length = sizeof(output_data_buffer);
            error = ReadFromFile(file_handle, output_data_buffer, &output_data_length);
            if(error != 0)
            {
                if(error != ERR_EOF && server_verbose_mode > VERBOSE_MODE_SILENT)
                    printf("*** Error reading file: %xh\r\n", error);

                CloseConnectionToClient();
                return;
            }
        }

        if(SendDataToTcpConnection(output_data_buffer, output_data_length))
            output_data_length = 0;
        else
            return;
    }
}


static void StartSendingDirectory()
{
    //We send the header in two steps because TCP/IP UNAPI implementations
    //aren't required to be able to accept more than 512 bytes of
    //outgoing TCP data in one go.

    char* dir_name_pointer;
    byte error;

    if(automaton_state == HTTPA_SENDING_DIRECTORY_LISTING_HEADER_1)
    {
        if(output_data_length == 0)
        {
            output_data_length = strlen(dir_list_header_1);
        }

        if(SendDataToTcpConnection(dir_list_header_1, output_data_length))
        {
            output_data_length = 0;
            automaton_state = HTTPA_SENDING_DIRECTORY_LISTING_HEADER_2;
        }
        else
            return;
    }

    if(output_data_length == 0)
    {
        dir_name_pointer = &filename_buffer[base_directory_length];
        sprintf(data_buffer, dir_list_header_2, dir_name_pointer, dir_name_pointer);
        PrepareChunkedData(data_buffer);
    }

    if(SendDataToTcpConnection(output_data_buffer, output_data_length))
    {
        output_data_length = 0;
        automaton_state = HTTPA_SENDING_DIRECTORY_LISTING_ENTRIES;

        dir_list_fib.alwaysFF = 0;
        ContinueSendingDirectory();
    }
}


#define dir_list_date if_modified_since_date

static void ContinueSendingDirectory()
{
    byte error;
    bool is_directory;

    while(true)
    {
        if(GetSimplifiedTcpConnectionState() == TCP_STATE_CLOSED)
        {
            PrintUnlessSilent(connection_lost_str);
            CloseConnectionToClient();
            return;
        }

        if(output_data_length == 0)
        {
            if(dir_list_fib.alwaysFF == 0)
                error = SearchFile(&file_fib, &dir_list_fib, true);
            else
                error = SearchNextFile(&dir_list_fib);

            if(error == ERR_NOFIL)
            {
                output_data_length = 0;
                automaton_state = HTTPA_SENDING_DIRECTORY_LISTING_FOOTER;
                FinishSendingDirectory();
                return;
            }
            else if(error != 0)
            {
                if(server_verbose_mode > VERBOSE_MODE_SILENT)
                    printf("*** Error retrieving directory contents: %xh\r\n", error);
                CloseConnectionToClient();
                return;
            }

            if(dir_list_fib.filename[0] == '.')
                continue;

            is_directory = (dir_list_fib.attributes & FILE_ATTR_DIRECTORY) != 0;

            if(!is_directory)
                _ultoa(dir_list_fib.fileSize, buffer, 10);

            if(dir_list_fib.dateOfModification != 0)
            {
                ParseFibDateTime(&dir_list_fib, &dir_list_date);
                ToIsoDate(buffer2, &dir_list_date);
            }
            else
            {
                *buffer2 = '\0';
            }

            sprintf(data_buffer, dir_list_entry,
                dir_list_fib.filename, is_directory ? "/" : "?a=1",
                dir_list_fib.filename,
                is_directory ? "/" : empty_str,
                is_directory ? "&lt;DIR&gt;" : buffer,
                buffer2);
            
            PrepareChunkedData(data_buffer);
        }

        if(SendDataToTcpConnection(output_data_buffer, output_data_length))
            output_data_length = 0;
        else
            return;
    }
}


static void FinishSendingDirectory()
{
    if(output_data_length == 0)
    {
        PrepareChunkedData(dir_list_footer);
        strcat(output_data_buffer, "0\r\n\r\n");
        output_data_length = strlen(output_data_buffer);
    }
    
    if(SendDataToTcpConnection(output_data_buffer, output_data_length))
    {
        CloseConnectionToClient();
    }
}


static void PrepareChunkedData(char* data)
{
    sprintf(output_data_buffer, "%x\r\n%s\r\n", strlen(data), data);
    output_data_length = strlen(output_data_buffer);
}