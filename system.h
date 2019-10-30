#ifndef __SYSTEM_H
#define __SYSTEM_H

#include "types.h"

#define SYSTEM_TIMER (*((int*)0xFC9E))

//Well, it's actually 50 or 60 depending on the VDP frequency but :shrug:
#define SYSTEM_TIMER_TICKS_PER_SECOND 50

bool MsxDos2IsRunning();
void TerminateWithErrorCode(byte errorCode);
byte NormalizeDirectory(char* directoryPath, char* normalizedDirectoryPath);
bool KeyIsPressed();

#endif
