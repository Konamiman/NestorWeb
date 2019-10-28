#ifndef __SYSTEM_H
#define __SYSTEM_H

#include "types.h"

bool MsxDos2IsRunning();
void TerminateWithErrorCode(byte errorCode);
byte NormalizeDirectory(char* directoryPath, char* normalizedDirectoryPath);
bool KeyIsPressed();

#endif
