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
} applicationState;


#endif   //__TYPES_H
