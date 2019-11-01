#ifndef __SYSTEM_H
#define __SYSTEM_H

#include "types.h"
#include "msxdos.h"

#define SYSTEM_TIMER (*((int*)0xFC9E))

//Well, it's actually 50 or 60 depending on the VDP frequency but :shrug:
#define SYSTEM_TIMER_TICKS_PER_SECOND 50

#define MAX_FILE_PATH_LEN 64

bool MsxDos2IsRunning();
void TerminateWithErrorCode(byte errorCode);
byte NormalizeDirectory(char* directoryPath, char* normalizedDirectoryPath);
bool KeyIsPressed();
byte SearchFile(char* fileName, fileInfoBlock* fib, bool include_dirs);
byte OpenFile(void* path_or_fib, byte* file_handle);
byte ReadFromFile(byte file_handle, byte* destination, int* length);
void CloseFile(byte file_handle);
void DisableDiskErrorPrompt();

#endif
