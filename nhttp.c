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

void CheckPrerequisites();
void CheckTcpIpSupportsPassiveConnections();
void TerminateWithErrorMessage(char* message);

int main(char** argv, int argc)
{
    printf(strTitle);
    if(argc == 0)
    {
        printf(strHelp);
        TerminateWithErrorCode(0);
    }

    CheckPrerequisites();
    InitializeTcpIpUnapi();
    CheckTcpIpSupportsPassiveConnections();

    printf("The program itself!");
    TerminateWithErrorCode(0);

    return 0;
}

void CheckPrerequisites()
{
    if(!MsxDos2IsRunning())
        TerminateWithErrorMessage("MSX-DOS 2 required");

    if(!TcpIpUnapiIsAvailable())
        TerminateWithErrorMessage("No TCP/IP UNAPI implementations found");
}

void CheckTcpIpSupportsPassiveConnections()
{
    if(!TcpIpSupportsPassiveTcpConnections())
        TerminateWithErrorMessage("The TCP/IP UNAPI implementation doesn't support passive TCP connections");;
}

void TerminateWithErrorMessage(char* message)
{
    printf("*** %s\r\n", message);
    TerminateWithErrorCode(1);
}
