#include "types.h"
#include "system.h"
#include "http.h"
#include "stdio.h"
#include "tcpip.h"
#include "version.h"
#include "utils.h"
#include "msxdos.h"
#include "proc.h"
#include "cgi.h"
#include "buffers.h"
#include <string.h>

#define MAX_CACHE_SECS_FOR_DIRECTORY_LISTING "3600"

extern applicationState state;
extern char http_error_buffer[80];
extern fileInfoBlock file_fib;
extern byte buffer[64];
extern byte buffer2[64];

byte* output_data_buffer;
byte* output_data_pointer;
byte file_handle;
byte automaton_state;
int output_data_length;
byte data_buffer[256+1];
char filename_buffer[MAX_FILE_PATH_LEN*2];
bool request_is_get;
bool request_is_head;
char raw_request[256+1];

static byte* data_buffer_pointer;
static int data_buffer_length;
static bool skipping_data;
static int last_system_timer_value;
static int ticks_without_data;
static bool base_directory_is_root_of_drive;
static fileInfoBlock dir_list_fib;
static bool send_as_attachment;
static bool has_if_modified_since;
static dateTime fib_date;
static dateTime if_modified_since_date;
static byte base_directory_length;
static byte requested_resource[MAX_FILE_PATH_LEN+1];
static int requested_resource_length;
static bool must_run_cgi;
static byte error_code_from_cgi;

static char num_buffer[11];
static char content_length_buffer[32];

static const char* default_document = "\\INDEX.HTM";
const char* connection_lost_str = "Connection lost\r\n";
static const char* connection_closed_by_client_str = "Connection closed by client\r\n";

//Note that this one has the chunked transfer size hardcoded at the beginning.
//Don't do this at home, kids.
static const char* dir_list_header_1 = 
    "1C2\r\n"
    "<html>"
    "<head>"
    "<style type='text/css'>"
    "body {font-family: sans-serif; color: white; background-color: blue;} "
    "p a {font-size: small; color: lightgray; font-style: italic; margin-top: 20px;} "
    "table {font-family: monospace;} "
    "td {border: 0px solid black;} "
    "td:nth-of-type(1) {padding-right: 30px; font-weight: bold;} "
    "td:nth-of-type(2) {text-align: right;} "
    "td:nth-of-type(3) {padding-left: 30px;} "
    "a { color: white; text-decoration: none; display:block;}"
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
    "<p><a href=\"http://github.com/Konamiman/NestorHTTP\" target=\"_blank\">NestorHTTP " VERSION "</a></p>"
    "</body>"
    "</html>";

static const char* dir_list_entry =
    "<tr>"
    "<td><a href=\"%s%s\">%s%s</td>"
    "<td>%s</td>"
    "<td>%s</td>"
    "</tr>";

extern const char* empty_str;

static void InitializeDataBuffer();
static void OpenServerConnection();
static void HandleIncomingConnectionIfAvailable();
static void ContinueReadingRequest();
static bool ProcessRequestLine();
static bool ContinueReadingLine();
static bool SetupCgiRequestPointers();
static void ResetInactivityCounter();
static void UpdateInactivityCounter();
static void ContinueReadingHeaders();
static void ProcessHeader();
static void SendHtmlResponseToClient(int statusCode, char* statusMessage, char* content);
static char* ConvertRequestToFilename(char** query_string_start, char** resource_start, bool is_local_redirect);
static void StartSendingFile();
static bool CheckIfModifiedSince();
static void SendLastModified();
static void ContinueSendingFile();
static void StartSendingDirectory();
static void ContinueSendingDirectoryHeaders();
static void StandarizeSlashes(char* path);
static void ContinueSendingDirectory();
static void PrepareHtmlForDirectoryEntry();
static void SendParentDirectoryLink(char* dir_name);
static void RemoveLastPartOfPath(char* path);
static void FinishSendingDirectory();
static void PrepareChunkedData(char* data);

extern void _ultoa(long val, char* buffer, char base);

#define PrintUnlessSilent(s) { if(state.verbosityLevel > VERBOSE_MODE_SILENT) printf(s); }


static void InitializeDataBuffer()
{
    data_buffer_pointer = data_buffer; 
    data_buffer_length = 0;
    skipping_data = false;
    ResetInactivityCounter();
}


void InitializeHttpAutomatonData()
{
    base_directory_length = strlen(state.baseDirectory) - 1;
    base_directory_is_root_of_drive = base_directory_length < 3;
    http_error_buffer[0] = '\0';
    file_handle = 0;
    output_data_buffer = OUTPUT_DATA_BUFFER_START;
    ResetInactivityCounter();
}


void InitializeHttpAutomaton()
{
    InitializeHttpAutomatonData();
    CloseConnectionToClient();
}


void ReinitializeHttpAutomaton()
{
    InitializeHttpAutomatonData();
}


void CleanupHttpAutomaton()
{
    state.verbosityLevel = VERBOSE_MODE_SILENT;
    CloseConnectionToClient();
}


void DoHttpServerAutomatonStep()
{
    if(ticks_without_data >= state.inactivityTimeout)
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
            ContinueSendingDirectoryHeaders();
            break;
        case HTTPA_SENDING_DIRECTORY_LISTING_ENTRIES:
            ContinueSendingDirectory();
            break;
        case HTTPA_SENDING_DIRECTORY_LISTING_FOOTER:
            FinishSendingDirectory();
            break;
        case HTTPA_SENDING_CGI_RESULT_HEADERS:
            ContinueSendingCgiResultHeaders();
            break;
    }
}


static void OpenServerConnection()
{
    byte error;

    error = OpenPassiveTcpConnection(state.tcpPort);
    if(error == ERR_NO_NETWORK)
    {
        sprintf(http_error_buffer, "No network connection");
        return;
    }
    else if(error != 0)
    {
        sprintf(http_error_buffer, "Unexpected error when opening TCP connection: %i", error);
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


void CloseConnectionToClient()
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
}


static bool ProcessRequestLine()
{
    if(state.verbosityLevel > VERBOSE_MODE_SILENT)
        printf("Request: %s\r\n", data_buffer);

    if(strlen(data_buffer) == 256)
    {
        PrintUnlessSilent("ERROR: Request line too long, connection refused\r\n");
        return false;
    }

    request_is_get = false;
    request_is_head = false;
    if(StringStartsWith(data_buffer, "GET "))
    {
        request_is_get = true;
    }
    else if(StringStartsWith(data_buffer, "HEAD "))
    {
        request_is_head = true;
    }
    else
    {
        SendMethodNotAllowedError(false);
        return false;
    }

    return ProcessRequestedResource(false);
}


bool ProcessRequestedResource(bool is_local_redirect)
{
    char* converted_filename;
    char* query_string;
    char* resource_name_start;
    char* pointer;
    bool is_cgi_file;
    bool can_run_cgi;
    char ch;

    strcpy(raw_request, data_buffer);

    converted_filename = ConvertRequestToFilename(&query_string, &resource_name_start, is_local_redirect);
    send_as_attachment = query_string && StringStartsWith(query_string, "a=1");

    if(converted_filename)
    {
        is_cgi_file = StringStartsWith(resource_name_start, "CGI-BIN\\");
        can_run_cgi = !is_local_redirect && state.cgiEnabled;
        if(!(is_cgi_file && !can_run_cgi))
        {
            must_run_cgi = can_run_cgi && is_cgi_file;

            if(must_run_cgi)
            {
                pointer = converted_filename + 8;
                while(true)
                {
                    ch = *pointer;
                    if(ch == '\\')
                    {
                        *pointer = '\0';
                        break;
                    }
                    else if(ch == '?' || ch == '\0')
                        break;

                    pointer++;
                }
            }

            strcpy(filename_buffer, state.baseDirectory);
            strcat(filename_buffer, converted_filename);

            return must_run_cgi ? SetupRequestDependantEnvItems() : true;
        }
    }

    SendNotFoundError();
    CloseConnectionToClient();
    return false;
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

                if(data_buffer_length >= 256)
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
    if(state.verbosityLevel > VERBOSE_MODE_CONNECTIONS)
        printf("<-- %s\r\n", data_buffer);

    if(StringStartsWith(data_buffer, "If-Modified-Since:"))
    {
        has_if_modified_since = ParseVerboseDateTime(&data_buffer[18], &if_modified_since_date);
    }

    InitializeDataBuffer();
}


bool SendLineToClient(char* line)
{
    sprintf(TCP_INPUT_DATA_BUFFER_START, "%s\r\n", line);
    return SendCrlfTerminatedLineToClient(TCP_INPUT_DATA_BUFFER_START);
}


bool SendCrlfTerminatedLineToClient(char* line)
{
    if(state.verbosityLevel > VERBOSE_MODE_CONNECTIONS)
        printf("--> %s", line);

    return SendStringToTcpConnection(line);
}


void SendResponseStart(int statusCode, char* statusMessage)
{
    if(state.verbosityLevel > VERBOSE_MODE_SILENT)
        printf("Response: %i %s\r\n", statusCode, statusMessage);

    sprintf(TCP_INPUT_DATA_BUFFER_START, "HTTP/1.1 %i %s\r\n", statusCode, statusMessage);
    SendCrlfTerminatedLineToClient(TCP_INPUT_DATA_BUFFER_START);
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
        if(state.verbosityLevel > VERBOSE_MODE_CONNECTIONS)
            printf("--> (HTML response)\r\n");
        SendStringToTcpConnection(content);
    }
    else
    {
        SendLineToClient(empty_str);
    }
}


void SendErrorResponseToClient(int statusCode, char* statusMessage, char* detailedMessage)
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
            "<style type='text/css'>"
            "body {font-family: sans-serif;} .footer {font-size: small; color: gray; font-style: italic;}"
            "a {font-size: small; color: gray; text-decoration: none;}"
            "</style>"
            "</head>"
            "<body>"
            "<h1>%i %s</h1>"
            "<p>%s</p>"
            "<p class='footer'><a href=\"http://github.com/Konamiman/NestorHTTP\" target=\"_blank\">NestorHTTP " VERSION "</a></p>"
            "</body>"
            "</html>",
        statusCode,
        statusMessage,
        detailedMessage
        );
    
    SendHtmlResponseToClient(statusCode, statusMessage, output_data_buffer);
}


static char* ConvertRequestToFilename(char** query_string_start, char** resource_start, bool is_local_redirect)
{
    char* pointer;
    char* start_pointer;
    char* requested_resource_pointer;

    char ch;
    bool last_char_was_dot;

    pointer = data_buffer;
    *query_string_start = null;
    
    if(is_local_redirect)
    {
        pointer++;
    }
    else
    {
        while(*pointer != ' ') pointer++;
        while(*pointer == ' ') pointer++;
        if(*pointer == '/') pointer++;
    }

    *resource_start = pointer;

    start_pointer = pointer;
    requested_resource_pointer = requested_resource;
    requested_resource_length = 0;

    last_char_was_dot = false;
    while((ch = *pointer) > ' ' && ch != QUERY_STRING_SEPARATOR)
    {
        if(requested_resource_length < MAX_FILE_PATH_LEN)
        {
            *requested_resource_pointer++ = ch;
            requested_resource_length++;
        }

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

    *requested_resource_pointer = '\0';

    if(ch == QUERY_STRING_SEPARATOR)
        *query_string_start = pointer+1;

    if(pointer[-1] == '\\')
        pointer--;

    *pointer = '\0';
    
    return start_pointer == pointer && !state.directoryListingEnabled ? default_document+1 : start_pointer;
}


void SendBadRequestError()
{
    SendErrorResponseToClient(400, "Bad Request", "Sorry, I'm not able to process your request.");
}


void SendNotFoundError()
{
    SendErrorResponseToClient(404, "Not Found", "The requested resource was not found in this server");
}


void SendMethodNotAllowedError(bool fromCgi)
{
    SendErrorResponseToClient(405, "Method Not Allowed",
      fromCgi ?
      "The specified HTTP method can't be handled by this script" :
      "Sorry, this is a GET only server for now");
}


void SendInternalError()
{
    SendErrorResponseToClient(500, "Internal Server Error", "Sorry, something went wrong. It's not you, it's me.");
}


void ProcessFileOrDirectoryRequest()
{
    byte error;
    bool is_directory;
    dateTime date_time;

    //We need to treat requesting the root resource + the base directory being the root of the drive
    //as a special case, since in this case we can't search for the directory itself
    //(doing that would return the first file in the directory instead).
    if(state.directoryListingEnabled && requested_resource_length == 0 && base_directory_is_root_of_drive)
    {
        file_fib.alwaysFF = 0;
        StartSendingDirectory();
        return;
    }

    is_directory = false;

    error = SearchFile(filename_buffer, &file_fib, true);

    if(error == 0 && (file_fib.attributes & FILE_ATTR_DIRECTORY))
    {
        if(state.directoryListingEnabled)
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
        if(state.verbosityLevel > VERBOSE_MODE_SILENT)
            printf("*** Error searching file: %xh\r\n", error);

        SendInternalError();
        CloseConnectionToClient();
        return;
    }
    else if(error != 0)
    {
        SendBadRequestError();
        CloseConnectionToClient();
        return;
    }

    if(is_directory)
        StartSendingDirectory();
    else if(must_run_cgi && !(state.directoryListingEnabled && send_as_attachment))
        RunCgi();
    else
        StartSendingFile();
}


static void StartSendingFile()
{
    byte error;

    if(CheckIfModifiedSince())
        return;

    error = OpenFile(&file_fib, &file_handle, FILE_OPEN_NO_WRITE);
    if(error)
    {
        if(state.verbosityLevel > VERBOSE_MODE_SILENT)
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

    SendLastModified();
    SendLineToClient(empty_str);

    if(request_is_head)
    {
        CloseConnectionToClient();
        return;
    }

    output_data_length = 0;
    PrintUnlessSilent("Sending file contents...\r\n");
    automaton_state = HTTPA_SENDING_FILE_CONTENTS;

    ContinueSendingFile();
}


void SendContentLengthHeader(ulong length)
{
    _ultoa(length, num_buffer, 10);
    sprintf(content_length_buffer, "Content-Length: %s", num_buffer);
    SendLineToClient(content_length_buffer);
}


static bool CheckIfModifiedSince()
{
    if(file_fib.dateOfModification != 0)
    {
        ParseFibDateTime(&file_fib, &fib_date);

        if(has_if_modified_since && CompareDates(&fib_date, &if_modified_since_date) <= 0)
        {
            SendErrorResponseToClient(304, "Not Modified", null);
            CloseConnectionToClient();
            return true;
        }
    }

    return false;
}


static void SendLastModified()
{
    if(file_fib.dateOfModification != 0)
    {
        //Note that we assume that fib_date has been already filled
        ToVerboseDate(buffer2, &fib_date);
        sprintf(buffer, "Last-Modified: %s", buffer2);
        SendLineToClient(buffer);
    }
}


static void ContinueSendingFile()
{
    byte error;
    int size_of_data_chunk_to_send;

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
            output_data_length = OUTPUT_DATA_BUFFER_LENGTH;
            output_data_pointer = output_data_buffer;
            error = ReadFromFile(file_handle, output_data_buffer, &output_data_length);
            if(error != 0)
            {
                if(error != ERR_EOF && state.verbosityLevel > VERBOSE_MODE_SILENT)
                    printf("*** Error reading file: %xh\r\n", error);

                CloseConnectionToClient();
                return;
            }
        }

        size_of_data_chunk_to_send = output_data_length;
        if(size_of_data_chunk_to_send > TCP_UNAPI_OUTPUT_BUFFER_SIZE)
            size_of_data_chunk_to_send = TCP_UNAPI_OUTPUT_BUFFER_SIZE;

        if(SendDataToTcpConnection(output_data_pointer, size_of_data_chunk_to_send))
        {
            output_data_pointer += size_of_data_chunk_to_send;
            output_data_length -= size_of_data_chunk_to_send;
        }
        else
            return;
    }
}


static void StartSendingDirectory()
{
    //If a request for a directory doesn't end with a '/', redirect to
    //the versin with a '/' so that relative links in the files list
    //work as expected.

    if(requested_resource_length > 0 && requested_resource[requested_resource_length-1] != '/')
    {
        SendResponseStart(308, "Moved Permanently");
        sprintf(buffer, "Location: http://%i.%i.%i.%i:%u/%s/",
            state.localIp[0], state.localIp[1], state.localIp[2], state.localIp[3],
            state.tcpPort,
            requested_resource);
        SendLineToClient(buffer);
        SendLineToClient(empty_str);
        CloseConnectionToClient();
    }
    else
    {
        PrintUnlessSilent("Sending directory listing...\r\n");
        SendResponseStart(200, "Ok");
        SendLineToClient("Content-Type: text/html");
        SendLineToClient("Transfer-Encoding: chunked");
        SendLineToClient("Cache-Control: private, max-age=" MAX_CACHE_SECS_FOR_DIRECTORY_LISTING);
        SendLineToClient(empty_str);

        if(request_is_head)
        {
            CloseConnectionToClient();
            return;
        }

        automaton_state = HTTPA_SENDING_DIRECTORY_LISTING_HEADER_1;
        output_data_length = 0;
        ContinueSendingDirectoryHeaders();
    }
}


static void ContinueSendingDirectoryHeaders()
{
    //We send the header in two steps because TCP/IP UNAPI implementations
    //aren't required to be able to accept more than 512 bytes of
    //outgoing TCP data in one go.

    char* dir_name_pointer;
    char* pointer;
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

        StandarizeSlashes(dir_name_pointer);

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


static void StandarizeSlashes(char* path)
{
    while(*path)
    {
        if(*path == '\\')
            *path = '/';

        path++;
    }
}


#define dir_list_date if_modified_since_date
#define requesting_root_resource_on_root_of_drive (file_fib.alwaysFF == 0)

static void ContinueSendingDirectory()
{
    byte error;
    bool is_directory;
    char* dir_name;

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
            {
                error = SearchFile(
                    requesting_root_resource_on_root_of_drive ? (void*)state.baseDirectory : (void*)&file_fib, 
                    &dir_list_fib, true);

                dir_name = &filename_buffer[base_directory_length];
                if(strlen(dir_name) != 1)
                    SendParentDirectoryLink(dir_name);
            }
            else
            {
                error = SearchNextFile(&dir_list_fib);
            }

            if(error == ERR_NOFIL)
            {
                output_data_length = 0;
                automaton_state = HTTPA_SENDING_DIRECTORY_LISTING_FOOTER;
                FinishSendingDirectory();
                return;
            }
            else if(error != 0)
            {
                if(state.verbosityLevel > VERBOSE_MODE_SILENT)
                    printf("*** Error retrieving directory contents: %xh\r\n", error);
                CloseConnectionToClient();
                return;
            }

            if(dir_list_fib.filename[0] == '.')
                continue;

            PrepareHtmlForDirectoryEntry();
        }

        if(SendDataToTcpConnection(output_data_buffer, output_data_length))
            output_data_length = 0;
        else
            return;
    }
}


static void PrepareHtmlForDirectoryEntry()
{
    bool is_directory;

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


static void SendParentDirectoryLink(char* dir_name)
{
    strcpy(buffer, dir_name);
    RemoveLastPartOfPath(buffer);

    sprintf(data_buffer, dir_list_entry,
        buffer, empty_str,
        "../", empty_str,
        empty_str,
        empty_str);

    PrepareChunkedData(data_buffer);
    SendDataToTcpConnection(output_data_buffer, output_data_length);
    output_data_length = 0;
}


static void RemoveLastPartOfPath(char* path)
{
    char* pointer;

    pointer = path + strlen(path);
    while(*pointer-- != '/');
    pointer[2] = '\0';
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
