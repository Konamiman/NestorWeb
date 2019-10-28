#include "types.h"
#include "http.h"
#include "stdio.h"
#include "tcpip.h"


static byte automaton_state;
static char* error_buffer;


static void OpenServerConnection();
static void HandleIncomingConnectionIfAvailable();
static void ContinueReadingRequest();


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

    status = GetSimplifiedTcpConnectionState();
    switch(status)
    {
        case TCP_STATE_CLOSED:
            printf("Connection lost\r\n");
            automaton_state = HTTPA_NONE;
            return;
        
        case TCP_STATE_CLOSE_WAIT:
            printf("Connection closed by client");
            CloseTcpConnection();
            automaton_state = HTTPA_NONE;
            return;
    }


    printf("WIP...\r\n");
    AbortAllTransientTcpConnections();
}