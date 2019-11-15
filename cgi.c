#include "cgi.h"
#include "types.h"
#include "http.h"
#include "msxdos.h"
#include "proc.h"
#include "tcpip.h"
#include <stdio.h>
#include <string.h>

extern applicationState state;
extern fileInfoBlock file_fib;
extern byte buffer[64];
extern const char* empty_str;
extern char temp_directory[64];

#define PrintUnlessSilent(s) { if(state.verbosityLevel > VERBOSE_MODE_SILENT) printf(s); }

void RunCgi()
{
    byte error;

    PrintUnlessSilent("Running CGI script\r\n");
    error = proc_fork(&file_fib, null, &state);

    if(state.verbosityLevel > VERBOSE_MODE_SILENT)
        printf("*** Error running CGI script: %i\r\n", error);

    SendInternalError();
    CloseConnectionToClient();
}


void SendResultAfterCgi(byte error_code_from_cgi)
{
    SendResponseStart(200, "Ok");
    SendLineToClient("Content-type: text/plain");
    sprintf(buffer, "Error code from CGI script: %i", error_code_from_cgi);
    SendContentLengthHeader(strlen(buffer));
    SendLineToClient(empty_str);
    SendStringToTcpConnection(buffer);
    CloseConnectionToClient();
}
