#include "types.h"
#include "system.h"
#include "http.h"
#include "stdio.h"
#include "tcpip.h"
#include "version.h"
#include "utils.h"
#include <string.h>


#define MAX_SECONDS_WITHOUT_DATA 5


static byte automaton_state;
static char* error_buffer;
static byte data_buffer[256+1];
static byte* data_buffer_pointer;
static int data_buffer_length;
static bool skipping_data;
static int last_system_timer_value;
static int ticks_without_data;
static byte output_data_buffer[512];
static int server_port;


static void InitializeDataBuffer();
static void OpenServerConnection();
static void HandleIncomingConnectionIfAvailable();
static void CloseConnectionToClient();
static void ContinueReadingRequest();
static bool ContinueReadingLine();
static void ResetInactivityCounter();
static void UpdateInactivityCounter();
static void ContinueReadingHeaders();
static void SendLineToClient(char* line);
static void SendHtmlResponseToClient(int statusCode, char* statusMessage, char* content);
static void SendErrorResponseToClient(int statusCode, char* statusMessage, char* detailedMessage);


static void InitializeDataBuffer()
{
    data_buffer_pointer = data_buffer; 
    data_buffer_length = 0;
    skipping_data = 0;
    ResetInactivityCounter();
}


void InitializeHttpAutomaton(char* http_error_buffer, uint port)
{
    server_port = port;
    error_buffer = http_error_buffer;
    error_buffer[0] = '\0';

    automaton_state = HTTPA_NONE;
}


void DoHttpServerAutomatonStep()
{
    if(ticks_without_data >= MAX_SECONDS_WITHOUT_DATA * 50)
    {
        printf("Closing connection for client inactivity\r\n");
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
            automaton_state = HTTPA_NONE;
            break;
        
        case TCP_STATE_ESTABLISHED:
            printf("Connection received!\r\n");
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
    CloseTcpConnection();
    ResetInactivityCounter();
    automaton_state = HTTPA_NONE;
    printf("I have closed the connection with the client\r\n");
}


static void ContinueReadingRequest()
{
    byte status;
    bool lineComplete;

    status = GetSimplifiedTcpConnectionState();
    switch(status)
    {
        case TCP_STATE_CLOSED:
            printf("Connection lost\r\n");
            automaton_state = HTTPA_NONE;
            return;
        
        case TCP_STATE_CLOSE_WAIT:
            printf("Connection closed by client\r\n");
            CloseTcpConnection();
            automaton_state = HTTPA_NONE;
            return;
    }


    lineComplete = ContinueReadingLine();
    if(!lineComplete)
        return;

    printf("<-- %s\r\n", data_buffer);
    if(strlen(data_buffer) == sizeof(data_buffer)-1)
    {
        printf("ERROR: Request line too long, connection refused\r\n");
        CloseTcpConnection();
        automaton_state = HTTPA_NONE;
        return;
    }

    if(strncmpi(data_buffer, "GET", 3))
    {
        InitializeDataBuffer();
        automaton_state = HTTPA_READING_HEADERS;
    }
    else
    {
        SendErrorResponseToClient(405, "Method Not Allowed", "Sorry, this is a GET only server for now");
        CloseConnectionToClient();
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
                    printf("* WARNING: Line too long, truncating\r\n");
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
            printf("Connection lost\r\n");
            automaton_state = HTTPA_NONE;
            return;
        
        case TCP_STATE_CLOSE_WAIT:
            printf("Connection closed by client\r\n");
            CloseTcpConnection();
            automaton_state = HTTPA_NONE;
            return;
    }

    lineComplete = ContinueReadingLine();
    if(!lineComplete)
        return;

    if(data_buffer_length > 0)
    {
        printf("<-- %s\r\n", data_buffer);
        InitializeDataBuffer();
    }
    else
    {
        SendErrorResponseToClient(404, "Not Found", "Please be patient, we're still setting up stuff here!");
        CloseConnectionToClient();
    }
}


static void SendLineToClient(char* line)
{
    sprintf(data_buffer, "%s\r\n", line);
    printf("--> %s", data_buffer);
    SendStringToTcpConnection(data_buffer);
}


static void SendHtmlResponseToClient(int statusCode, char* statusMessage, char* content)
{
    char buffer[64];

    sprintf(buffer, "HTTP/1.1 %i %s", statusCode, statusMessage);
    SendLineToClient(buffer);
    SendLineToClient("X-Powered-By: NestorHTTP/" VERSION "; MSX-DOS");
    sprintf(buffer, "Content-Length: %i", content ? strlen(content) : 0);
    SendLineToClient(buffer);
    if(content)
    {
        SendLineToClient("Content-Type: text/html");
        SendLineToClient("");
        printf("--> (response)\r\n");
        SendStringToTcpConnection(content);
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
