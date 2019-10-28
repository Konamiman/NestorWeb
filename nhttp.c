#include <stdio.h>
#include "types.h"
#include "system.h"
#include "tcpip.h"
#include "http.h"

#define VERSION "0.1" 

const char* strTitle = 
    "NestorHTTP " VERSION "\r\n"
    "(c) 2020 by Konamiman\r\n"
    "\r\n";

const char* strHelp =
    "Usage: NHTTP <base directory>";

char base_directory[64];
char http_error_buffer[80];


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

    if(argc == 0)
    {
        printf(strHelp);
        TerminateWithErrorCode(0);
    }

    error = NormalizeDirectory(argv[0] , base_directory);
    if(error != 0)
        TerminateWithErrorCode(error);
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
    printf("Listening on %i.%i.%i.%i:80\r\n", buffer[0], buffer[1], buffer[2], buffer[3]);
    printf("Press any key to exit\r\n\r\n");

    InitializeHttpAutomaton(http_error_buffer);
}


void TerminateWithErrorMessage(char* message)
{
    printf("*** %s\r\n", message);
    TerminateWithErrorCode(1);
}


void Cleanup()
{
    AbortAllTransientTcpConnections();
}