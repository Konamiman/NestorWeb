#include "types.h"
#include "http.h"
#include "stdio.h"
#include "tcpip.h"
#include <string.h>


static byte automaton_state;
static char* error_buffer;
static byte data_buffer[256+1];
static byte* data_buffer_pointer;
static int data_buffer_length;
static bool skipping_data;

#define InitializeDataBuffer() { data_buffer_pointer = data_buffer; data_buffer_length = 0; skipping_data = 0;}


static void OpenServerConnection();
static void HandleIncomingConnectionIfAvailable();
static void ContinueReadingRequest();
static bool ContinueReadingLine();
static void ContinueReadingHeaders();


void InitializeHttpAutomaton(char* http_error_buffer)
{
    automaton_state = HTTPA_NONE;
    error_buffer = http_error_buffer;
    error_buffer[0] = '\0';
}


void DoHttpServerAutomatonStep()
{
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

    error = OpenPassiveTcpConnection();
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
            CloseTcpConnection();
            automaton_state = HTTPA_NONE;
            break;
    }
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

    //TODO: Process request
    InitializeDataBuffer();
    automaton_state = HTTPA_READING_HEADERS;
}


bool ContinueReadingLine()
{
    byte datum;

    while(EnsureIncomingTcpDataIsAvailable())
    {
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

    return false;
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
        //WIP
        AbortAllTransientTcpConnections();
    }
}