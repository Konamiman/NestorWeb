#ifndef __SYSTEM_H
#define __SYSTEM_H

#include "types.h"
#include "msxdos.h"

#define SYSTEM_TIMER (*((int*)0xFC9E))

//Well, it's actually 50 or 60 depending on the VDP frequency but :shrug:
#define SYSTEM_TIMER_TICKS_PER_SECOND 50

#define MAX_FILE_PATH_LEN 64

#define ERAFNK 0x00CC
#define DSPFNK 0x00CF
#define CNSDFG 0xF3DE //0=display function keys, other=hide
#define FNKSTR 0xF87F //Function key contents (10 x 16 bytes)
#define F_KEY_CONTENTS_LENGTH 16

#define F_KEY_CONTENTS_POINTER(key) ((char*)FNKSTR + ((key)-1)*F_KEY_CONTENTS_LENGTH)

#define STDIN 0
#define STDOUT 1

#define PARSE_FLAGS_HAS_DIRECTORY (1 << 1)
#define PARSE_FLAGS_HAS_DRIVE (1 << 2)
#define PARSE_FLAGS_HAS_FILENAME (1 << 3)
#define PARSE_FLAGS_IS_AMBIGUOUS (1 << 5)
#define PARSE_FLAGS_IS_DOT (1 << 6)

bool MsxDos2IsRunning();
void TerminateWithErrorCode(byte errorCode);
byte NormalizeDirectory(char* directoryPath, char* normalizedDirectoryPath);
bool KeyIsPressed();
byte SearchFile(void* fileNameOrFib, fileInfoBlock* fib, bool include_dirs);
byte SearchNextFile(fileInfoBlock* fib);
byte OpenFile(void* path_or_fib, byte* file_handle);
byte ReadFromFile(byte file_handle, byte* destination, int* length);
void CloseFile(byte file_handle);
void DisableDiskErrorPrompt();
bool FunctionKeysAreVisible();
void DisplayFunctionKeys();
void HideFunctionKeys();
void SetFunctionKeyContents(int key, char* contents);
bool GetEnvironmentItem(const char* name, char* value);
byte SetEnvironmentItem(const char* name, char* value);
void FindEnvironmentItem(uint index, char* name);
char* GetPointerToLastItemOfPathname(const char* pathname, byte* parse_flags, byte* drive, char** pointer_to_last_item);

#endif
