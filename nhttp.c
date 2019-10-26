#include <stdio.h>
#include "types.h"
#include "system.h"
#include "tcpip.h"

#define VERSION "0.1" 

const char* strTitle = 
    "NestorHTTP " VERSION "\r\n"
    "(c) 2020 by Konamiman\r\n"
    "\r\n";

const char* strHelp =
    "This is the program help!";

void Initialize();
void TerminateWithErrorMessage(char* message);

int main(char** argv, int argc)
{
    printf(strTitle);
    if(argc == 0)
    {
        printf(strHelp);
        TerminateWithErrorCode(0);
    }

    Initialize();

    printf("The program itself!");
    TerminateWithErrorCode(0);

    return 0;
}


#define CHECK(function, message) if(!function()) { TerminateWithErrorMessage(message); }

void Initialize()
{
    CHECK(MsxDos2IsRunning, "MSX-DOS 2 required");

    CHECK(TcpIpUnapiIsAvailable, "No TCP/IP UNAPI implementations found");
    InitializeTcpIpUnapi();
    CHECK(TcpIpSupportsPassiveTcpConnections, "The TCP/IP UNAPI implementation doesn't support passive TCP connections");
}

void TerminateWithErrorMessage(char* message)
{
    printf("*** %s\r\n", message);
    TerminateWithErrorCode(1);
}
