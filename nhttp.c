#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "system.h"
#include "tcpip.h"
#include "http.h"
#include "utils.h"
#include "version.h"

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

char base_directory[64];
char http_error_buffer[80];
uint port;
int verbose_mode;
int inactivity_timeout;
bool directory_listing_enabled;
bool function_keys_were_visible;
bool cgi_enabled;
char function_keys_backup[5 * F_KEY_CONTENTS_LENGTH];

void ProcessArguments(char** argv, int argc);
void Initialize();
void InitializeInfoArea(char* ip, uint port);
void TerminateWithErrorMessage(char* message);
void Cleanup();


#define ExitRequested() KeyIsPressed()
#define HttpAutomatonHasReportedAFatalError() (http_error_buffer[0] != '\0')

int main(char** argv, int argc)
{
    printf(strTitle);

    ProcessArguments(argv, argc);
    Initialize();

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

    error = NormalizeDirectory(argv[0] , base_directory);
    if(error != 0)
        TerminateWithErrorCode(error);

    port = HTTP_DEFAULT_SERVER_PORT;
    verbose_mode = VERBOSE_MODE_CONNECTIONS;
    inactivity_timeout = DEFAULT_INACTIVITY_TIMEOUT_SECS;
    directory_listing_enabled = false;
    cgi_enabled = true;

    for(i = 1; i<argc; i++)
    {
        c = ToLower(argv[i][0]);
        if(c == 'p')
        {
            port = (uint)atoi(&argv[i][2]);
            if(port == 0 || port > MAX_USABLE_TCP_PORT)
                TerminateWithErrorMessage("Port number must be between 1 and " MAX_USABLE_TCP_PORT_STR);
        }
        else if(c == 'v')
        {
            verbose_mode = ((byte)argv[i][2]-'0');
        }
        else if(c == 'd')
        {
            directory_listing_enabled = ((byte)argv[i][2]-'0') != 0;
        }
        else if(c == 't')
        {
            inactivity_timeout = (uint)atoi(&argv[i][2]);
            if(inactivity_timeout == 0)
                TerminateWithErrorMessage("Inactivity timeout must be at least 1");
        }
        else if(c == 'c')
        {
            cgi_enabled = ((byte)argv[i][2]-'0') != 0;
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
    byte ip[4];

    CHECK(MsxDos2IsRunning, "MSX-DOS 2 required");

    CHECK(TcpIpUnapiIsAvailable, "No TCP/IP UNAPI implementations found");
    InitializeTcpIpUnapi();
    CHECK(TcpIpSupportsPassiveTcpConnections, "The TCP/IP UNAPI implementation doesn't support passive TCP connections");

    DisableDiskErrorPrompt();
    AbortAllTransientTcpConnections();

    GetLocalIpAddress(ip);
    if(*(long*)ip == 0)
        TerminateWithErrorMessage("Local IP address is not configured");

    printf("Base directory: %s\r\n", base_directory);
    printf("Directory listing is %s\r\n", directory_listing_enabled ? "ON" : "OFF");
    printf("CGI support is %s\r\n", cgi_enabled ? "ON" : "OFF");
    printf("Listening on %i.%i.%i.%i:%u\r\n", ip[0], ip[1], ip[2], ip[3], port);
    printf("Press any key to exit\r\n\r\n");

    InitializeInfoArea(ip, port);

    InitializeHttpAutomaton(base_directory, http_error_buffer, ip, port, verbose_mode, inactivity_timeout * SYSTEM_TIMER_TICKS_PER_SECOND, directory_listing_enabled, cgi_enabled);
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
}