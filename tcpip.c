#include "types.h"
#include "asm.h"
#include "tcpip.h"
#include "stdio.h"


static unapi_code_block tcpip_unapi_code_block;
static const char* tcpip_unapi_identifier = "TCP/IP";
static Z80_registers regs;
static byte connection_id;


bool TcpIpUnapiIsAvailable()
{
    return UnapiGetCount(tcpip_unapi_identifier) > 0;
}


void InitializeTcpIpUnapi()
{
    UnapiBuildCodeBlock(tcpip_unapi_identifier, 1, &tcpip_unapi_code_block);
    connection_id = 0;
}


bool TcpIpSupportsPassiveTcpConnections()
{
    regs.Bytes.B = 1;
    UnapiCall(&tcpip_unapi_code_block, TCPIP_GET_CAPAB, &regs, REGS_MAIN, REGS_MAIN);
    return regs.Bytes.L & TCPIP_CAPAB_OPEN_TCP_PASSIVE_CONN_WITH_NO_REMOTE_SOCKET != 0;
}


void GetLocalIpAddress(byte* buffer)
{
    regs.Bytes.B = 1;
    UnapiCall(&tcpip_unapi_code_block, TCPIP_GET_IPINFO, &regs, REGS_MAIN, REGS_MAIN);
    buffer[0] = regs.Bytes.L;
    buffer[1] = regs.Bytes.H;
    buffer[2] = regs.Bytes.E;
    buffer[3] = regs.Bytes.D;
}


void LetTcpipBreathe()
{
    UnapiCall(&tcpip_unapi_code_block, TCPIP_WAIT, &regs, REGS_NONE, REGS_NONE);
}


void AbortAllTransientTcpConnections()
{
    regs.Bytes.B = 0;
    UnapiCall(&tcpip_unapi_code_block, TCPIP_TCP_ABORT, &regs, REGS_MAIN, REGS_NONE);
    connection_id = 0;
}


static byte _OpenPassiveTcpConnection(byte* connection_id)
{
    tcpConnectionParameters params;

    *((ulong*)params.remoteIP) = 0;
    params.remotePort = 0;
    params.localPort = 80;
    params.timeoutValue = 0;
    params.flags = TCP_OPEN_FLAGS_PASSIVE;

    regs.Words.HL = (int)&params;
    UnapiCall(&tcpip_unapi_code_block, TCPIP_TCP_OPEN, &regs, REGS_MAIN, REGS_MAIN);
    if(regs.Bytes.A == 0)
        *connection_id = regs.Bytes.B;

    return regs.Bytes.A;
}


byte OpenPassiveTcpConnection()
{
    byte error;

    error = _OpenPassiveTcpConnection(&connection_id);
    if(error == ERR_NO_FREE_CONN)
    {
        AbortAllTransientTcpConnections();
        error = _OpenPassiveTcpConnection(&connection_id);
    }

    return error;
}


byte GetSimplifiedTcpConnectionState()
{
    regs.Bytes.B = connection_id;
    regs.Words.HL = 0;
    UnapiCall(&tcpip_unapi_code_block, TCPIP_TCP_STATE, &regs, REGS_MAIN, REGS_MAIN);

    if(regs.Bytes.A != 0)
    {
        connection_id = 0;
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


void CloseTcpConnection()
{
    if(connection_id == 0)
        return;

    regs.Bytes.B = connection_id;
    UnapiCall(&tcpip_unapi_code_block, TCPIP_TCP_CLOSE, &regs, REGS_MAIN, REGS_NONE);
}