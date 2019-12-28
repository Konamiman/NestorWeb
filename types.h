#ifndef __TYPES_H
#define __TYPES_H

typedef unsigned int uint;
typedef unsigned char byte;
typedef unsigned long ulong;
typedef unsigned char bool;
#define false (0)
#define true (!(false))
#define null ((void*)0)

#include "asm.h"

#define REQUEST_METHOD_GET 0
#define REQUEST_METHOD_HEAD 1
#define REQUEST_METHOD_OTHER 2

typedef struct {
    char baseDirectory[64];
    unapi_code_block unapiCodeBlock;
    uint tcpPort;
    uint inactivityTimeout;
    bool directoryListingEnabled;
    byte verbosityLevel;
    bool cgiEnabled;
    byte localIp[4];
    byte tcpConnectionNumber;
    byte stdinFileHandleCopy;
    byte stdoutFileHandleCopy;
    byte requestMethodType;
    byte authenticationMode;
} applicationState;


#endif   //__TYPES_H
