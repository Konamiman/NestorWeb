#include "types.h"
#include "asm.h"
#include "tcpip.h"
#include "stdio.h"
#include <string.h>


extern applicationState state;
extern Z80_registers regs;

static const char* tcpip_unapi_identifier = "TCP/IP";
static byte data_buffer[512];
static byte* data_buffer_pointer;
static int data_buffer_length;


bool TcpIpUnapiIsAvailable()
{
    return UnapiGetCount(tcpip_unapi_identifier) > 0;
}


void InitializeTcpIpUnapi()
{
    UnapiBuildCodeBlock(tcpip_unapi_identifier, 1, &state.unapiCodeBlock);
    state.tcpConnectionNumber = 0;
    data_buffer_length = 0;
}


void ReinitializeTcpIpUnapi()
{
    data_buffer_length = 0;
}


bool TcpIpSupportsPassiveTcpConnections()
{
    regs.Bytes.B = 1;
    UnapiCall(&state.unapiCodeBlock, TCPIP_GET_CAPAB, &regs, REGS_MAIN, REGS_MAIN);
    return regs.Bytes.L & TCPIP_CAPAB_OPEN_TCP_PASSIVE_CONN_WITH_NO_REMOTE_SOCKET != 0;
}


void GetLocalIpAddress(byte* buffer)
{
    regs.Bytes.B = 1;
    UnapiCall(&state.unapiCodeBlock, TCPIP_GET_IPINFO, &regs, REGS_MAIN, REGS_MAIN);
    buffer[0] = regs.Bytes.L;
    buffer[1] = regs.Bytes.H;
    buffer[2] = regs.Bytes.E;
    buffer[3] = regs.Bytes.D;
}


void LetTcpipBreathe()
{
    UnapiCall(&state.unapiCodeBlock, TCPIP_WAIT, &regs, REGS_NONE, REGS_NONE);
}


void AbortAllTransientTcpConnections()
{
    regs.Bytes.B = 0;
    UnapiCall(&state.unapiCodeBlock, TCPIP_TCP_ABORT, &regs, REGS_MAIN, REGS_NONE);
    state.tcpConnectionNumber = 0;
}


static byte _OpenPassiveTcpConnection(byte* tcpConnectionNumber, uint port)
{
    tcpConnectionParameters params;

    *((ulong*)params.remoteIP) = 0;
    params.remotePort = 0;
    params.localPort = port;
    params.timeoutValue = 0;
    params.flags = TCP_OPEN_FLAGS_PASSIVE;

    regs.Words.HL = (int)&params;
    UnapiCall(&state.unapiCodeBlock, TCPIP_TCP_OPEN, &regs, REGS_MAIN, REGS_MAIN);
    if(regs.Bytes.A == 0)
    {
        data_buffer_length = 0;
        *tcpConnectionNumber = regs.Bytes.B;
    }
        
    return regs.Bytes.A;
}


byte OpenPassiveTcpConnection(uint port)
{
    byte error;

    error = _OpenPassiveTcpConnection(&state.tcpConnectionNumber, port);
    if(error == ERR_NO_FREE_CONN)
    {
        AbortAllTransientTcpConnections();
        error = _OpenPassiveTcpConnection(&state.tcpConnectionNumber, port);
    }

    return error;
}


byte GetSimplifiedTcpConnectionState()
{
    regs.Bytes.B = state.tcpConnectionNumber;
    regs.Words.HL = 0;
    UnapiCall(&state.unapiCodeBlock, TCPIP_TCP_STATE, &regs, REGS_MAIN, REGS_MAIN);

    if(regs.Bytes.A != 0)
    {
        state.tcpConnectionNumber = 0;
        return TCP_STATE_CLOSED;
    }
    else if(regs.Bytes.B < TCP_STATE_ESTABLISHED)
        return TCP_STATE_LISTEN;
    else if(regs.Bytes.B < TCP_STATE_CLOSE_WAIT)
        return TCP_STATE_ESTABLISHED;
    else if(regs.Bytes.B < TCP_STATE_CLOSING)        
        return TCP_STATE_CLOSE_WAIT;
    else
        return TCP_STATE_CLOSING;
}


bool CloseTcpConnection()
{
    if(state.tcpConnectionNumber == 0)
        return false;

    regs.Bytes.B = state.tcpConnectionNumber;
    UnapiCall(&state.unapiCodeBlock, TCPIP_TCP_CLOSE, &regs, REGS_MAIN, REGS_NONE);
    state.tcpConnectionNumber = 0;
    return true;
}


bool EnsureIncomingTcpDataIsAvailable()
{
    if(data_buffer_length != 0)
        return true;

    regs.Bytes.B = state.tcpConnectionNumber;
    regs.Words.DE = (int)data_buffer;
    regs.Words.HL = sizeof(data_buffer);
    UnapiCall(&state.unapiCodeBlock, TCPIP_TCP_RCV, &regs, REGS_MAIN, REGS_MAIN);
    if(regs.Bytes.A != 0)
        return false;
    
    data_buffer_pointer = data_buffer;
    data_buffer_length = regs.Words.BC;

    return data_buffer_length > 0;
}


byte GetIncomingTcpByte()
{
    //This should never happen, EnsureIncomingTcpDataIsAvailable should always
    //be called before this method.
    if(data_buffer_length == 0)
        return 0;
    
    data_buffer_length--;
    return *data_buffer_pointer++;
}


bool SendDataToTcpConnection(byte* data, int length)
{
    regs.Bytes.B = state.tcpConnectionNumber;
    regs.Words.DE = (int)data;
    regs.Words.HL = length;
    regs.Bytes.C = 0; //Not sure if using TCP_SEND_FLAGS_PUSH would be appropriate here
    UnapiCall(&state.unapiCodeBlock, TCPIP_TCP_SEND, &regs, REGS_MAIN, REGS_MAIN);
    
    //Any other error means the connection is in a wrong config and will be handled by the HTTP automaton
    return regs.Bytes.A != ERR_BUFFER;
}


bool SendStringToTcpConnection(char* string)
{
    return SendDataToTcpConnection(string, strlen(string));
}