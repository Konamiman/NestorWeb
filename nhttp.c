#include <stdio.h>
#include <stdlib.h>
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
    "\r\n"
    "p: Server port number, 1-" MAX_USABLE_TCP_PORT_STR ", default is 80\r\n"
    "v: Verbosity mode:\r\n"
    "   0: silent, 1: show connections and errors (default), 2: show all headers\r\n"
    "t: Inactivity timeout for client connections in seconds, default is " DEFAULT_INACTIVITY_TIMEOUT_SECS_STR "\r\n"
    "\r\n"
    "A request for '/' or for a directory will serve INDEX.HTM file if it exists.\r\n"
    "\r\n";

char base_directory[64];
char http_error_buffer[80];
uint port;
int verbose_mode;
int inactivity_timeout;


void ProcessArguments(char** argv, int argc);
void Initialize();
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
        else if(c == 't')
        {
            inactivity_timeout = (uint)atoi(&argv[i][2]);
            if(inactivity_timeout == 0)
                TerminateWithErrorMessage("Inactivity timeout must be at least 1");
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
    byte buffer[4];

    CHECK(MsxDos2IsRunning, "MSX-DOS 2 required");

    CHECK(TcpIpUnapiIsAvailable, "No TCP/IP UNAPI implementations found");
    InitializeTcpIpUnapi();
    CHECK(TcpIpSupportsPassiveTcpConnections, "The TCP/IP UNAPI implementation doesn't support passive TCP connections");

    AbortAllTransientTcpConnections();

    printf("Base directory: %s\r\n", base_directory);

    GetLocalIpAddress(buffer);
    printf("Listening on %i.%i.%i.%i:%u\r\n", buffer[0], buffer[1], buffer[2], buffer[3], port);
    printf("Press any key to exit\r\n\r\n");

    InitializeHttpAutomaton(base_directory, http_error_buffer, port, verbose_mode, inactivity_timeout * SYSTEM_TIMER_TICKS_PER_SECOND);
}


void TerminateWithErrorMessage(char* message)
{
    printf("*** %s\r\n", message);
    TerminateWithErrorCode(1);
}


void Cleanup()
{
    CleanupHttpAutomaton();
    AbortAllTransientTcpConnections();
}