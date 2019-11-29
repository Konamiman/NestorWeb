#include "types.h"
#include "asm.h"


enum TcpipUnapiFunctions {
    TCPIP_GET_CAPAB = 1,
    TCPIP_GET_IPINFO = 2,
    TCPIP_NET_STATE = 3,
    TCPIP_TCP_OPEN = 13,
    TCPIP_TCP_CLOSE = 14,
    TCPIP_TCP_ABORT = 15,
    TCPIP_TCP_STATE = 16,
    TCPIP_TCP_SEND = 17,
    TCPIP_TCP_RCV = 18,
    TCPIP_WAIT = 29
};


enum TcpipUnapiErrorCodes {
    ERR_OK = 0,			    
    ERR_NOT_IMP,		
    ERR_NO_NETWORK,		
    ERR_NO_DATA,		
    ERR_INV_PARAM,		
    ERR_QUERY_EXISTS,	
    ERR_INV_IP,		    
    ERR_NO_DNS,		    
    ERR_DNS,		    
    ERR_NO_FREE_CONN,	
    ERR_CONN_EXISTS,	
    ERR_NO_CONN,		
    ERR_CONN_STATE,		
    ERR_BUFFER,		    
    ERR_LARGE_DGRAM,	
    ERR_INV_OPER
};


typedef struct {
    byte remoteIP[4];
    uint remotePort;
    uint localPort;
    uint timeoutValue;
    byte flags;
} tcpConnectionParameters;

#define TCP_OPEN_FLAGS_PASSIVE 1


#define TCPIP_CAPAB_OPEN_TCP_PASSIVE_CONN_WITH_NO_REMOTE_SOCKET (1 << 5)
#define TCP_SEND_FLAGS_PUSH 1
#define MAX_USABLE_TCP_PORT 0xFFFE
#define MAX_USABLE_TCP_PORT_STR "65534"


#define TCP_STATE_CLOSED 0
#define TCP_STATE_LISTEN 1
#define TCP_STATE_ESTABLISHED 4
#define TCP_STATE_CLOSE_WAIT 7
#define TCP_STATE_CLOSING 8


bool TcpIpUnapiIsAvailable();
void InitializeTcpIpUnapi();
void ReinitializeTcpIpUnapi();
bool TcpIpSupportsPassiveTcpConnections();
void GetLocalIpAddress(byte* buffer);
void LetTcpipBreathe();
void AbortAllTransientTcpConnections();
byte OpenPassiveTcpConnection(uint port);
byte GetSimplifiedTcpConnectionState();
bool CloseTcpConnection();
bool EnsureIncomingTcpDataIsAvailable();
byte GetIncomingTcpByte();
int GetIncomingTcpData(byte* buffer, int count);
bool SendDataToTcpConnection(byte* data, int length);
bool SendStringToTcpConnection(char* string);
byte GetRemoteIpAddress(byte* ipBuffer);



