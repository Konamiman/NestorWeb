#include "types.h"
#include "asm.h"
#include "tcpip.h"

static unapi_code_block tcpip_unapi_code_block;
static const char* tcpip_unapi_identifier = "TCP/IP";
static Z80_registers regs;

bool TcpIpUnapiIsAvailable()
{
    return UnapiGetCount(tcpip_unapi_identifier) > 0;
}

void InitializeTcpIpUnapi()
{
    UnapiBuildCodeBlock(tcpip_unapi_identifier, 1, &tcpip_unapi_code_block);
}

bool TcpIpSupportsPassiveTcpConnections()
{
    regs.Bytes.B = 1;
    UnapiCall(&tcpip_unapi_code_block, TCPIP_GET_CAPAB, &regs, REGS_MAIN, REGS_MAIN);
    return regs.Bytes.L & TCPIP_CAPAB_OPEN_TCP_PASSIVE_CONN_WITH_NO_REMOTE_SOCKET != 0;
}