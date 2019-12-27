#include "cgi.h"
#include "types.h"
#include "http.h"
#include "msxdos.h"
#include "proc.h"
#include "tcpip.h"
#include "system.h"
#include "buffers.h"
#include "utils.h"
#include "stdlib.h"
#include "version.h"
#include <stdio.h>
#include <string.h>
#include "auth.h"

extern applicationState state;
extern fileInfoBlock file_fib;
extern byte buffer[64];
extern byte buffer2[64];
extern const char* empty_str;
extern char temp_directory[64];
extern byte* output_data_buffer;
extern byte file_handle;
extern byte automaton_state;
extern const char* connection_lost_str;
extern int output_data_length;
extern byte* output_data_pointer;
extern byte data_buffer[256+1];
extern char filename_buffer[MAX_FILE_PATH_LEN*2];
extern char raw_request[MAX_FILE_PATH_LEN+1];
extern long input_content_length;
extern bool input_content_length_received;
extern bool request_is_get;
extern bool request_is_head;
extern dateTime fib_date;
extern dateTime if_modified_since_date;
extern char current_user[];
extern char current_password[];

static byte error_code_from_cgi;
static char* cgi_header_pointer;
static int cgi_header_length;
static int status_code;
static char* status_message;
static bool must_send_cgi_out_content;
static bool must_send_cgi_contents_file;
static int total_output_data_length;

static char temp_in_filename[128];
static char temp_out_filename[128];
static char cgi_contents_filename[128];
static char script_command_line[MAX_COMMAND_LINE_LENGTH+1];
static char header_name_buffer[128];

#define PrintUnlessSilent(s) { if(state.verbosityLevel > VERBOSE_MODE_SILENT) printf(s); }


static bool GetOutputHeaderLine();
static bool ProcessFirstHeaderOfCgiResult();
static void SetupScriptCommandLine(char* queryString);


static const char* ok_str = "Ok";
static const char* found_str = "Found";
static const char* http_prefix_str = "HTTP_";

static const char* env_gateway_interface = "GATEWAY_INTERFACE";
static const char* env_server_name = "SERVER_NAME";
static const char* env_server_port = "SERVER_PORT";
static const char* env_server_software = "SERVER_SOFTWARE";
static const char* env_request_method = "REQUEST_METHOD";
static const char* env_script_name = "SCRIPT_NAME";
static const char* env_path_info = "PATH_INFO";
static const char* env_query_string = "QUERY_STRING";
static const char* env_server_protocol = "SERVER_PROTOCOL";
static const char* env_path_translated = "PATH_TRANSLATED";
static const char* env_remote_addr = "REMOTE_ADDR";
static const char* env_content_type = "CONTENT_TYPE";
static const char* env_content_length = "CONTENT_LENGTH";
static const char* env_base_directory = "_NHTTP_BASE_DIR";
const char* env_remote_user = "REMOTE_USER";
const char* env_remote_password = "REMOTE_PASSWORD";

static void CreateTempFilePaths()
{
    strcpy(temp_in_filename, temp_directory);
    strcat(temp_in_filename, "_NHTTPIN.$");
    strcpy(temp_out_filename, temp_directory);
    strcat(temp_out_filename, "_NHTTPOUT.$");
}


static void InitializeDataBuffers()
{
    strcpy(header_name_buffer, http_prefix_str);
}


static void InitializeFixedEnvItems()
{
    SetEnvironmentItem(env_gateway_interface, "CGI/1.1");
    
    FormatIpAddress(data_buffer, state.localIp);
    SetEnvironmentItem(env_server_name, data_buffer);

    sprintf(data_buffer, "%i", state.tcpPort);
    SetEnvironmentItem(env_server_port, data_buffer);

    SetEnvironmentItem(env_server_software, "NestorHTTP/" VERSION);

    SetEnvironmentItem(env_base_directory, state.baseDirectory);
}


void InitializeCgiEngine()
{
    CreateTempFilePaths();
    InitializeFixedEnvItems();
    InitializeDataBuffers();
    state.stdoutFileHandleCopy = 0xFF;
    state.stdinFileHandleCopy = 0xFF;
}


void RestoreStandardFileHandles()
{
    if(state.stdinFileHandleCopy != 0xFF)
    {
        CloseFile(STDIN);
        DuplicateFileHandle(state.stdinFileHandleCopy, null);
        CloseFile(state.stdinFileHandleCopy);
        state.stdinFileHandleCopy = 0xFF;
    }

    if(state.stdoutFileHandleCopy != 0xFF)
    {
        CloseFile(STDOUT);
        DuplicateFileHandle(state.stdoutFileHandleCopy, null);
        CloseFile(state.stdoutFileHandleCopy);
        state.stdoutFileHandleCopy = 0xFF;
    }
}


void ReinitializeCgiEngine(byte errorCodeFromCgi)
{
    CreateTempFilePaths();
    RestoreStandardFileHandles();
    InitializeDataBuffers();
    DisableDiskErrorPrompt();

    if(state.verbosityLevel > VERBOSE_MODE_SILENT)
        printf("CGI script returned error code %xh\r\n", errorCodeFromCgi);

    error_code_from_cgi = errorCodeFromCgi;
    StartSendingCgiResult();
}


void RunCgi()
{
    byte error;
    byte file_handle;
    byte file_handle2;

    ParseFilename(file_fib.filename, data_buffer);
    if(!StringStartsWith(&data_buffer[8], "CGI") && !StringStartsWith(&data_buffer[8], "COM"))
    {
        SendNotFoundError();
        CloseConnectionToClient();
        return;
    }

    SetAuthRelatedEnvItems();

    PrintUnlessSilent("Running CGI script\r\n");

    error = CreateFile(temp_out_filename, &file_handle, FILE_OPEN_NO_READ | FILE_OPEN_INHERITABLE); //File handle will be STDOUT
    if(error)
    {
        if(state.verbosityLevel > VERBOSE_MODE_SILENT)
            printf("*** Error cretating temp out file: %xh\r\n", error);
        
        SendInternalError();
        CloseConnectionToClient();
        return;
    }

    error = DuplicateFileHandle(STDOUT, &state.stdoutFileHandleCopy);
    if(error)
    {
        if(state.verbosityLevel > VERBOSE_MODE_SILENT)
            printf("*** Error duplicating STDOUT: %xh\r\n", error);
        
        SendInternalError();
        CloseConnectionToClient();
        return;
    }

    CloseFile(STDOUT);
    DuplicateFileHandle(file_handle, null);
    CloseFile(file_handle);

    state.requestMethodType = request_is_get ? REQUEST_METHOD_GET : request_is_head ? REQUEST_METHOD_HEAD : REQUEST_METHOD_OTHER;

    //Unfortunately we can't use file_fib here, since the messing with STDOUT above
    //spoils the contents of the buffer returned by _WPATH, and thus the CGI program
    //would receive an incorrect value for the PROGRAM environment item.
    error = proc_fork(filename_buffer, script_command_line, &state);

    //If we get here there was an error while attempting to fork

    RestoreStandardFileHandles();
    if(state.verbosityLevel > VERBOSE_MODE_SILENT)
        printf("*** Error running CGI script: %i\r\n", error);

    SendInternalError();
    CloseConnectionToClient();
}


bool CreateAndRedirectInFile()
{
    byte error;

    if(!input_content_length_received && !request_is_get && !request_is_head)
    {
        PrintUnlessSilent("*** Content-Length header not received\r\n");
        SendBadRequestError();
        CloseConnectionToClient();
        return false;
    }

    error = CreateFile(temp_in_filename, &file_handle, FILE_OPEN_INHERITABLE); //File handle will be STDIN
    if(error)
    {
        if(state.verbosityLevel > VERBOSE_MODE_SILENT)
            printf("*** Error cretating temp in file: %xh\r\n", error);
        
        SendInternalError();
        CloseConnectionToClient();
        return false;
    }

    error = DuplicateFileHandle(STDIN, &state.stdinFileHandleCopy);
    if(error)
    {
        if(state.verbosityLevel > VERBOSE_MODE_SILENT)
            printf("*** Error duplicating STDIN: %xh\r\n", error);
        
        SendInternalError();
        CloseConnectionToClient();
        return false;
    }

    CloseFile(STDIN);
    DuplicateFileHandle(file_handle, null);
    CloseFile(file_handle);

    return true;
}


void ContinueReadingBody()
{
    int count;
    byte error;

    if(!CheckConnectionIsStillOpenByClient())
        return;

    count = TCP_INPUT_DATA_BUFFER_SIZE;
    if(count > input_content_length)
        count = input_content_length;

    EnsureIncomingTcpDataIsAvailable();
    count = GetIncomingTcpData(TCP_INPUT_DATA_BUFFER_START, count);
    if(count == 0)
        return;

    error = WriteToFile(STDIN, TCP_INPUT_DATA_BUFFER_START, count);
    if(error)
    {
        if(state.verbosityLevel > VERBOSE_MODE_SILENT)
            printf("*** Error writing to SDTDIN file: %xh\r\n", error);

        SendInternalError();
        CloseConnectionToClient();
        return;    
    }
    
    input_content_length -= count;
    if(input_content_length == 0)
    {
        RewindFile(STDIN);
        ProcessFileOrDirectoryRequest();
    }
}


void CleanupCgiEngine()
{
    DeleteFile(temp_out_filename);
    DeleteFile(temp_in_filename);

    DeleteEnvironmentItem(env_gateway_interface);
    DeleteEnvironmentItem(env_server_name);
    DeleteEnvironmentItem(env_server_port);
    DeleteEnvironmentItem(env_server_software);

    DeleteEnvironmentItem(env_request_method);
    DeleteEnvironmentItem(env_script_name);
    DeleteEnvironmentItem(env_path_info);
    DeleteEnvironmentItem(env_query_string);
    DeleteEnvironmentItem(env_server_protocol);
    DeleteEnvironmentItem(env_path_translated);
    DeleteEnvironmentItem(env_remote_addr);
    DeleteEnvironmentItem(env_remote_user);
    DeleteEnvironmentItem(env_remote_password);

    DeleteEnvironmentItem(env_base_directory);

    CleanupHeaderBasedEnvItems();
}


void StartSendingCgiResult()
{
    byte error;
    bool line_ok;

    if(error_code_from_cgi != 0)
    {
        switch(error_code_from_cgi)
        {
            case 1:
                SendBadRequestError();
                break;
            case 2:
                SendNotFoundError();
                break;
            case 3:
                SendMethodNotAllowedError(true);
                break;
            case 4:
                SendNotModifiedStatus();
                break;
            default:
                SendInternalError();
        }
        CloseConnectionToClient();
        return;
    }

    SearchFile(temp_out_filename, &file_fib, false);    //To have its size at hand
    error = OpenFile(&file_fib, &file_handle, FILE_OPEN_NO_WRITE);
    if(error)
    {
        if(state.verbosityLevel > VERBOSE_MODE_SILENT)
            printf("*** Error opening CGI script output file: %xh\r\n", error);

        SendInternalError();
        CloseConnectionToClient();
        return;
    }


    output_data_length = OUTPUT_DATA_BUFFER_LENGTH;
    output_data_pointer = OUTPUT_DATA_BUFFER_START;
    error = ReadFromFile(file_handle, OUTPUT_DATA_BUFFER_START, &output_data_length);
    output_data_pointer[output_data_length] = '\0'; //Will be useful in case the script uses X-CGI-Error
    total_output_data_length = output_data_length;
    if(error == ERR_EOF)
    {
        SendResponseStart(204, "No Content");
        SendContentLengthHeader(0);
        SendLineToClient(empty_str);
        CloseConnectionToClient();
        return;
    }
    else if(error)
    {
        if(state.verbosityLevel > VERBOSE_MODE_SILENT)
            printf("*** Error reading CGI script output file: %xh\r\n", error);

        SendInternalError();
        CloseConnectionToClient();
        return;
    }

    must_send_cgi_out_content = !request_is_head;
    must_send_cgi_contents_file = false;

    if(!GetOutputHeaderLine())
        return;

    status_code = 200;
    status_message = ok_str;

    if(!ProcessFirstHeaderOfCgiResult())
        return;

    SendResponseStart(status_code, status_message);
    automaton_state = HTTPA_SENDING_CGI_RESULT_HEADERS;
    
    ContinueSendingCgiResultHeaders();
}


#define PSLH_PROCESSED 0
#define PSLH_NOT_PROCESSED 1
#define PSLH_ERROR 2

static byte ProcessStatusLikeHeader(char* headerName)
{
    if(!StringStartsWith(cgi_header_pointer, headerName))
        return PSLH_NOT_PROCESSED;

    cgi_header_pointer += strlen(headerName);
    while(*++cgi_header_pointer == ' ');
    status_code = atoi(cgi_header_pointer);
    if(status_code < 100 || status_code > 999)
    {
        if(state.verbosityLevel > VERBOSE_MODE_SILENT)
            printf("*** Malformed data received from CGI script: invalid '%s' header\r\n", headerName);
        SendInternalError();
        CloseConnectionToClient();
        return PSLH_ERROR;
    }
    while(*++cgi_header_pointer >= '0' && *cgi_header_pointer <= '9');
    while(*++cgi_header_pointer == ' ');
    status_message = cgi_header_pointer;

    if(!GetOutputHeaderLine())
        return PSLH_ERROR;

    return PSLH_PROCESSED;
}


static bool ProcessFirstHeaderOfCgiResult()
{
    char* tmp_pointer;
    byte error;

    error = ProcessStatusLikeHeader("Status:");
    if(error == PSLH_ERROR)
        return false;
    else if(error == PSLH_PROCESSED)        
        return true;

    error = ProcessStatusLikeHeader("X-CGI-Error:");
    if(error == PSLH_ERROR)
        return false;
    else if(error == PSLH_PROCESSED)
    {  
        //We need to copy data around because SendErrorResponseToClient
        //overwrites output_data_buffer with the generated HTML
        strncpy(data_buffer, status_message, sizeof(data_buffer));
        strncpy(TCP_INPUT_DATA_BUFFER_START, &cgi_header_pointer[2], TCP_INPUT_DATA_BUFFER_SIZE); //+2 to skip the end of headers empty line
        SendErrorResponseToClient(status_code, data_buffer, TCP_INPUT_DATA_BUFFER_START);
        CloseConnectionToClient();
        
        return false;
    }
    else if(StringStartsWith(cgi_header_pointer, "X-CGI-Response-Type: NPH"))
    {
        PrintUnlessSilent("Sending response as NPH\r\n");

        automaton_state = HTTPA_SENDING_FILE_CONTENTS;
        return false;
    }
    else if(StringStartsWith(cgi_header_pointer, "Location:"))
    {
        tmp_pointer = cgi_header_pointer+9; //Length of "Location:"
        while(*tmp_pointer == ' ') tmp_pointer++;

        if(*tmp_pointer == '/')
        {
            //Local redirect

            if(state.verbosityLevel > VERBOSE_MODE_SILENT)
                printf("Local redirection requested: %s\r\n", tmp_pointer);

            cgi_header_pointer += 9;
            strcpy(data_buffer, tmp_pointer);
            if(ProcessRequestedResource(true))
            {
                CloseFile(file_handle);
                file_handle = 0;
                ProcessFileOrDirectoryRequest();
            }

            return false;
        }

        //Client redirect

        if(state.verbosityLevel > VERBOSE_MODE_SILENT)
            printf("Redirection requested: %s\r\n", tmp_pointer);

        status_code = 302;
        status_message = found_str;
        must_send_cgi_out_content = false;
    }
    else if(StringStartsWith(cgi_header_pointer, "X-CGI-Content-File:"))
    {
        //Value is expected to be a file path relative to CGI file location,
        //with MSX-DOS separators ('\'), without initial '\'.
        //Example: X-CGI-Location: data\myscript.dat

        tmp_pointer = cgi_header_pointer+19; //Length of "X-CGI-Content-File:"
        while(*tmp_pointer == ' ') tmp_pointer++;

        if(state.verbosityLevel > VERBOSE_MODE_SILENT)
            printf("Local file contents to send: %s\r\n", tmp_pointer);

        strcpy(cgi_contents_filename, state.baseDirectory);
        strcat(cgi_contents_filename, "CGI-BIN\\");
        strcat(cgi_contents_filename, tmp_pointer);

        error = SearchFile(cgi_contents_filename, &file_fib, false);
        if(error)
        {
            PrintUnlessSilent("*** Error: local file contents not found\r\n");
            SendInternalError();
            CloseConnectionToClient();
            return false;
        }

        ParseFibDateTime(&file_fib, &fib_date);

        if(GetEnvironmentItem("HTTP_IF_MODIFIED_SINCE", data_buffer) && 
           ParseVerboseDateTime(data_buffer, &if_modified_since_date) &&
           CompareDates(&fib_date, &if_modified_since_date) <= 0)
        {
            SendNotModifiedStatus();
            CloseConnectionToClient();
            return false;
        }

        if(!GetOutputHeaderLine())
            return false;

        must_send_cgi_contents_file = true;

    }

    return true;
}


void ContinueSendingCgiResultHeaders()
{
    ulong content_length;
    byte error;

    while(true) //Continue while there are more headers to send AND the TCP connection has room for more output data
    {
        if(GetSimplifiedTcpConnectionState() == TCP_STATE_CLOSED)
        {
            PrintUnlessSilent(connection_lost_str);
            CloseConnectionToClient();
            return;
        }

        if(cgi_header_length == 0)
            break;

        if(!SendLineToClient(cgi_header_pointer))
            return;
        
        if(!GetOutputHeaderLine())
            return;
    }

    if(!must_send_cgi_out_content)
    {
        SendContentLengthHeader(0);
        SendLineToClient(empty_str);
        CloseConnectionToClient();
        return;
    }

    if(must_send_cgi_contents_file)
    {
        CloseFile(file_handle);

        error = OpenFile(&file_fib, &file_handle, FILE_OPEN_NO_WRITE);
        if(error)
        {
            if(state.verbosityLevel > VERBOSE_MODE_SILENT)
                printf("*** Error opening CGI contents file: %xh\r\n", error);

            CloseConnectionToClient();
            return;
        }

        SendContentLengthHeader(file_fib.fileSize);
        SendLastModified();
        SendLineToClient(empty_str);

        output_data_length = 0;
        automaton_state = HTTPA_SENDING_FILE_CONTENTS;
        return;
    }

    //content length = output file size minus headers length
    content_length = file_fib.fileSize - (output_data_pointer-OUTPUT_DATA_BUFFER_START);
    SendContentLengthHeader(content_length);
    SendLineToClient(empty_str);

    if(content_length == 0)
        CloseConnectionToClient();
    else
        automaton_state = HTTPA_SENDING_FILE_CONTENTS;
}


static bool GetOutputHeaderLine()
{
    cgi_header_pointer = output_data_pointer;
    cgi_header_length = 0;

    while(*output_data_pointer != '\n' && output_data_length)
    {
        output_data_pointer++;
        cgi_header_length++;
        output_data_length--;
    }

    //Got to the end of the data (or the first OUTPUT_DATA_BUFFER_LENGTH bytes of the data)
    //without finding a proper end of line
    if(output_data_length == 0)
    {
        RestoreStandardFileHandles();
        PrintUnlessSilent("*** Malformed data received from CGI script: no headers end mark\r\n");
        SendInternalError();
        CloseConnectionToClient();
        return false;
    }

    //The \n is part of the header...
    output_data_pointer++;
    output_data_length--;
    //...but we want the header to be zero-terminated
    cgi_header_length--; //discard the \r
    output_data_pointer[-2] = '\0';

    return true;
}


bool SetupRequestDependantEnvItems()
{
    //GET /cgi-bin/myscript.cgi/foo/bar?fiz=buzz HTTP/1.1
    //--+ +--------------------++-----+ +------+ +------+
    //verb    script_name      path_info query_s protocol

    char* pointer;
    char* previous_pointer;
    char* temp;
    byte slashes_count;
    char ch;

    pointer = raw_request;
    slashes_count = 0;
    
    //Verb

    previous_pointer = pointer;
    while(*++pointer != ' ');
    *pointer = '\0';
    SetEnvironmentItem(env_request_method, previous_pointer);

    //Script name

    while(*pointer++ == ' ');
    previous_pointer = pointer;
    while(slashes_count < 2 && (ch = *++pointer) != ' ' && ch != '?')
    {
        if(ch == '\0')
        {
            SendBadRequestError();
            CloseConnectionToClient();
            return false;
        }
        if(ch == '/') slashes_count++;
    } 

    *pointer = '\0';
    SetEnvironmentItem(env_script_name, previous_pointer);

    //Path info

    if(ch == ' ' || ch == '?')
    {
        DeleteEnvironmentItem(env_path_info);
        DeleteEnvironmentItem(env_path_translated);
    }
    else
    {
        *pointer = '/';
        previous_pointer = pointer;

        while((ch = *++pointer) != '?' && ch != ' ')
        {
            if(ch == '\0')
            {
                SendBadRequestError();
                CloseConnectionToClient();
                return false;
            }
        }

        *pointer = '\0';

        UrlDecode(previous_pointer, &data_buffer[128], false);
        SetEnvironmentItem(env_path_info, &data_buffer[128]);

        //Path translated (<base directory>\<path info>)

        strcpy(data_buffer, state.baseDirectory);
        strcat(data_buffer, &data_buffer[128+1]);
        temp = data_buffer;
        while((ch = *++temp) != '\0')
        {
            if(ch == '/') *temp = '\\';
        }
        SetEnvironmentItem(env_path_translated, data_buffer);
    }

    //Query string

    previous_pointer = pointer+1;
    if(ch == '?')
    {
        while((ch = *++pointer) != ' ')
        {
            if(ch == '\0')
            {
                SendBadRequestError();
                CloseConnectionToClient();
                return false;
            }
        }
        *pointer = '\0';
        SetEnvironmentItem(env_query_string, previous_pointer);
        SetupScriptCommandLine(previous_pointer);
    }
    else
    {
        DeleteEnvironmentItem(env_query_string);
        SetupScriptCommandLine(null);
    }
    
    //Protocol

    while(*++pointer== ' ');
    SetEnvironmentItem(env_server_protocol, pointer);

    //Remote IP address

    GetRemoteIpAddress(data_buffer);
    FormatIpAddress(data_buffer+17, data_buffer);
    SetEnvironmentItem(env_remote_addr, data_buffer+17);

    return true;
}


void SetupScriptCommandLine(char* queryString)
{
    if(queryString == null || strlen(queryString) > MAX_COMMAND_LINE_LENGTH || strchr(queryString, '='))
    {
        *script_command_line = '\0';
        return;
    }

    UrlDecode(queryString, script_command_line, true);
}


void ProcessHeaderForCgi()
{
    char* pointer;
    char* value_pointer;
    char ch;
    bool skip_prefix;
    bool no_body;

    no_body = request_is_get || request_is_head;

    if(ProcessAuthenticationHeader(true))
        return;
    
    if(StringStartsWith(data_buffer, "Connection:") || StringStartsWith(data_buffer, "Upgrade-Insecure-Requests:"))
        return;

    if(StringStartsWith(data_buffer, "Content-Length:"))
    {
        if(no_body)
            return;

        input_content_length_received = true;
        pointer = &data_buffer[15];
        while(*pointer == ' ') pointer++;
        input_content_length = atol(pointer);
        skip_prefix = true;
    }
    else
    {
        skip_prefix = StringStartsWith(data_buffer, "Content-Type:");
        if(skip_prefix && no_body) 
            return;
    }

    pointer = data_buffer;

    while((ch = *pointer) != ':')
    {
        if(ch == '-')
            *pointer = '_';
        else if(ch >= 'a' && ch <= 'z')
            *pointer = ToUpper(ch);

        pointer++;
    }

    *pointer++ = '\0';
    while(*pointer == ' ') pointer++;
    value_pointer = pointer;

    if(skip_prefix)
    {
        pointer = data_buffer;
    }
    else
    {
        strcpy(header_name_buffer+5, data_buffer);
        pointer = header_name_buffer;
    }

    SetEnvironmentItem(pointer, value_pointer);
}


void CleanupHeaderBasedEnvItems()
{
    int index;

    DeleteEnvironmentItem(env_content_type);
    DeleteEnvironmentItem(env_content_length);

    index = 1;
    while(FindEnvironmentItem(index, OUTPUT_DATA_BUFFER_START))
    {
        //Increase index only if no deletion happens because
        //when we delete item N the previous N+1 item becomes the new N.
        if(StringStartsWith(OUTPUT_DATA_BUFFER_START, http_prefix_str))
            DeleteEnvironmentItem(OUTPUT_DATA_BUFFER_START);
        else
            index++;
    }
}
