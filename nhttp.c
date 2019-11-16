#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "system.h"
#include "tcpip.h"
#include "http.h"
#include "utils.h"
#include "version.h"
#include "proc.h"
#include "cgi.h"

const char* empty_str = "";

const char* strTitle = 
    "NestorHTTP " VERSION " - the HTTP server for MSX\r\n"
    "(c) 2020 by Konamiman\r\n"
    "\r\n";

const char* strHelp =
    "Usage: NHTTP <base directory> [p=<port>] [v=0|1|2] [t=<timeout>]\r\n"
    "             [d=0|1] [c=0|1]\r\n"
    "\r\n"
    "p: Server port number, 1-" MAX_USABLE_TCP_PORT_STR ", default is 80\r\n"
    "v: Verbosity mode:\r\n"
    "   0: silent, 1: show connections and errors (default), 2: show all headers\r\n"
    "t: Inactivity timeout for client connections in seconds, default is " DEFAULT_INACTIVITY_TIMEOUT_SECS_STR "\r\n"
    "d: enable directory listing when 1 (default: disabled)\r\n"
    "d: enable CGI scripts when 1 (default: enabled)\r\n"
    "\r\n"
    "When directory listing is disabled, a request for \"/\" or for a directory will\r\n"
    "serve INDEX.HTM file if it exists, or return a Not Found status if it doesn't.\r\n"
    "\r\n"
    "Files are sent as attachments if \"?a=1\" is added to the request.\r\n"
    "\r\n";

const char* temp_directory_backup_env_item = "_NHTTP_TEMP";

applicationState state;
Z80_registers regs;
char http_error_buffer[80];
char temp_directory[64];
fileInfoBlock file_fib;
byte buffer[64];

static bool function_keys_were_visible;
static char function_keys_backup[5 * F_KEY_CONTENTS_LENGTH];

void ProcessArguments(char** argv, int argc);
void Initialize();
int ServerMainLoop();
void InitializeInfoArea(char* ip, uint port);
void TerminateWithErrorMessage(char* message);
void Cleanup();
bool GetTempDirectory();


#define ExitRequested() KeyIsPressed()
#define HttpAutomatonHasReportedAFatalError() (http_error_buffer[0] != '\0')

int main(char** argv, int argc)
{
    printf(strTitle);

    ProcessArguments(argv, argc);
    Initialize();

    return ServerMainLoop();
}


int ServerMainLoop()
{
    while(!ExitRequested())
    {
        DoHttpServerAutomatonStep();
        if(HttpAutomatonHasReportedAFatalError())
        {
            Cleanup();
            TerminateWithErrorMessage(http_error_buffer);
        }

        LetTcpipBreathe();
    }

    printf("Exiting...");
    Cleanup();
    return 0;
}


void ProcessArguments(char** argv, int argc)
{
    byte error;
    int i;
    int c;

    if(argc == 0)
    {
        printf(strHelp);
        TerminateWithErrorCode(0);
    }

    error = NormalizeDirectory(argv[0] , state.baseDirectory);
    if(error != 0)
        TerminateWithErrorCode(error);

    state.tcpPort = HTTP_DEFAULT_SERVER_PORT;
    state.verbosityLevel = VERBOSE_MODE_CONNECTIONS;
    state.inactivityTimeout = DEFAULT_INACTIVITY_TIMEOUT_SECS;
    state.directoryListingEnabled = false;
    state.cgiEnabled = true;

    for(i = 1; i<argc; i++)
    {
        c = ToLower(argv[i][0]);
        if(c == 'p')
        {
            state.tcpPort = (uint)atoi(&argv[i][2]);
            if(state.tcpPort == 0 || state.tcpPort > MAX_USABLE_TCP_PORT)
                TerminateWithErrorMessage("Port number must be between 1 and " MAX_USABLE_TCP_PORT_STR);
        }
        else if(c == 'v')
        {
            state.verbosityLevel = ((byte)argv[i][2]-'0');
        }
        else if(c == 'd')
        {
            state.directoryListingEnabled = ((byte)argv[i][2]-'0') != 0;
        }
        else if(c == 't')
        {
            state.inactivityTimeout = (uint)atoi(&argv[i][2]) * SYSTEM_TIMER_TICKS_PER_SECOND;
            if(state.inactivityTimeout == 0)
                TerminateWithErrorMessage("Inactivity timeout must be at least 1");
        }
        else if(c == 'c')
        {
            state.cgiEnabled = ((byte)argv[i][2]-'0') != 0;
        }
        else
        {
            TerminateWithErrorMessage("Unknown parameter");
        }
    }
}


#define CHECK(function, message) if(!function()) { TerminateWithErrorMessage(message); }

void Initialize()
{
    CHECK(MsxDos2IsRunning, "MSX-DOS 2 required");

    CHECK(TcpIpUnapiIsAvailable, "No TCP/IP UNAPI implementations found");
    InitializeTcpIpUnapi();
    CHECK(TcpIpSupportsPassiveTcpConnections, "The TCP/IP UNAPI implementation doesn't support passive TCP connections");

    if(state.cgiEnabled)
    {
        if(!GetTempDirectory())
            TerminateWithErrorMessage("Invalid temporary directory (in NHTTP_TEMP or TEMP environment item)");
        
        if(strlen(temp_directory) > 64-11)
            TerminateWithErrorMessage("Invalid temporary directory (in NHTTP_TEMP or TEMP environment item): too long");

        InitializeCgiEngine();
    }        

    DisableDiskErrorPrompt();
    AbortAllTransientTcpConnections();

    GetLocalIpAddress(state.localIp);
    if(*(long*)state.localIp == 0)
        TerminateWithErrorMessage("Local IP address is not configured");

    printf("Base directory: %s\r\n", state.baseDirectory);
    printf("Directory listing is %s\r\n", state.directoryListingEnabled ? "ON" : "OFF");
    printf("CGI support is %s\r\n", state.cgiEnabled ? "ON" : "OFF");
    if(state.cgiEnabled)
        printf("Temporary directory for CGI: %s\r\n", temp_directory);
    printf("Listening on %i.%i.%i.%i:%u\r\n", state.localIp[0], state.localIp[1], state.localIp[2], state.localIp[3], state.tcpPort);
    printf("Press any key to exit\r\n\r\n");

    InitializeInfoArea(state.localIp, state.tcpPort);

    InitializeHttpAutomaton();
}


void InitializeInfoArea(char* ip, uint port)
{
    byte buffer[16];

    function_keys_were_visible = FunctionKeysAreVisible();
    memcpy(function_keys_backup, F_KEY_CONTENTS_POINTER(1), sizeof(function_keys_backup));
    SetFunctionKeyContents(1, "Server address:");
    sprintf(buffer, "%i.%i.%i.%i", ip[0], ip[1], ip[2], ip[3]);
    SetFunctionKeyContents(2, buffer);
    sprintf(buffer, "Port: %u", port);
    SetFunctionKeyContents(3, buffer);
    SetFunctionKeyContents(4, null);
    SetFunctionKeyContents(5, null);
    DisplayFunctionKeys();
}


void TerminateWithErrorMessage(char* message)
{
    printf("*** %s\r\n", message);
    TerminateWithErrorCode(1);
}


void Cleanup()
{
    CleanupHttpAutomaton();

    HideFunctionKeys();
    memcpy(F_KEY_CONTENTS_POINTER(1), function_keys_backup, sizeof(function_keys_backup));

    DeleteEnvironmentItem(temp_directory_backup_env_item);
}


byte proc_join(byte error_code_from_subprocess, void* state_data)
{
    memcpy(&state, state_data, sizeof(applicationState));
    ReinitializeTcpIpUnapi();
    ReinitializeHttpAutomaton(error_code_from_subprocess);
    if(state.cgiEnabled)
    {
        GetEnvironmentItem(temp_directory_backup_env_item, temp_directory);
        ReinitializeCgiEngine();
    }
    return ServerMainLoop();
}


bool GetTempDirectory()
{
    char* pointer_to_last_item;
    byte error;
    byte parse_flags;
    byte len;
    byte drive;
    char* pointer_to_terminator;

    if(!GetEnvironmentItem("NHTTP_TEMP", buffer))
    {
        if(!GetEnvironmentItem("TEMP", buffer))
        {
            GetEnvironmentItem("PROGRAM", buffer);
            pointer_to_last_item = GetPointerToLastItemOfPathname(temp_directory);
            *pointer_to_last_item = '\0';
            return true;
        }
    }

    error = NormalizeDirectory(buffer, temp_directory);
    if(error)
        return false;

    SetEnvironmentItem(temp_directory_backup_env_item, temp_directory);
    return true;
}