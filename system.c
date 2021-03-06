#include "system.h"
#include "types.h"
#include "asm.h"
#include "msxdos.h"
#include "string.h"
#include "stdio.h"


extern Z80_registers regs;
static fileInfoBlock fib;


bool MsxDos2IsRunning()
{
    DosCall(F_DOSVER, &regs, REGS_MAIN, REGS_MAIN);
    return regs.Bytes.A == 0 && regs.Bytes.B >= 2;
}


void TerminateWithErrorCode(byte errorCode)
{
    regs.Bytes.B = errorCode;
    DosCall(F_TERM, &regs, REGS_MAIN, REGS_NONE);
    //Fallback in case we are in MSX-DOS 1
    DosCall(F_TERM0, &regs, REGS_NONE, REGS_NONE);
}


byte NormalizeDirectory(char* directoryPath, char* normalizedDirectoryPath)
{
    char* pointer;

    // First append "\" to the directory path, unless it already has it
    // or the path has the form "X:" (meaning "current directory of drive X").
    // That way we tell DOS that we expect the path to be a directory.

    strcpy(normalizedDirectoryPath, directoryPath);
    pointer = normalizedDirectoryPath + strlen(normalizedDirectoryPath) - 1;
    if(pointer[0] != '\\' && pointer[0] != ':')
    {
        pointer[1] = '\\';
        pointer[2] = '\0';
    }

    // Then invoke _FFIRST, and if we get no error or .NOFIL Then
    // we're good to go (the later means that the directory is empty).
    // Otherwise we'll typically get "Invalid drive", "Invalid path"
    // or "Directory not found".

    regs.Words.DE = (int)normalizedDirectoryPath;
    regs.Bytes.B = 0;
    regs.Words.IX = (int)&fib;
    DosCall(F_FFIRST, &regs, REGS_ALL, REGS_MAIN);

    if(regs.Bytes.A != 0 && regs.Bytes.A != ERR_NOFIL)
        return regs.Bytes.A;

    // Now _WPATH will give us the normalized directory, we just need to
    // prepend the drive (which we can get from the FIB generated by _FFIRST).

    normalizedDirectoryPath[0] = fib.logicalDrive + 'A' - 1;
    normalizedDirectoryPath[1] = ':';
    normalizedDirectoryPath[2] = '\\';

    regs.Words.DE = (int)normalizedDirectoryPath + 3;
    DosCall(F_WPATH, &regs, REGS_MAIN, REGS_MAIN);

    // Lastly, we need to remove the "????????.???" that _WPATH appends
    // at the end of the normalized path (_WPATH returns HL pointing to it)

    pointer = (char*)regs.Words.HL;
    if(pointer[0] == '?')
    {
        pointer[0] = '\0';
    }

    return 0;
}


bool KeyIsPressed()
{
    regs.Bytes.E = 0xFF;
    DosCall(F_DIRIO, &regs, REGS_MAIN, REGS_MAIN);
    return regs.Bytes.A != 0;
}


byte SearchFile(void* fileNameOrFib, fileInfoBlock* fib, bool include_dirs)
{
    regs.Words.DE = (int)fileNameOrFib;
    if(*((byte*)fileNameOrFib) == 0xFF)
        regs.Words.HL = (int)"*.*";
    regs.Bytes.B = include_dirs ? FILE_ATTR_DIRECTORY : 0;
    regs.Words.IX = (int)fib;
    DosCall(F_FFIRST, &regs, REGS_ALL, REGS_MAIN);
    return regs.Bytes.A;
}


byte SearchNextFile(fileInfoBlock* fib)
{
    regs.Words.IX = (int)fib;
    DosCall(F_FNEXT, &regs, REGS_ALL, REGS_MAIN);
    return regs.Bytes.A;
}


byte OpenOrCreateFile(byte function_call, void* path_or_fib, byte* file_handle, byte flags)
{
    regs.Words.DE = (int)path_or_fib;
    regs.Bytes.A = flags;
    regs.Bytes.B = 0;
    DosCall(function_call, &regs, REGS_MAIN, REGS_MAIN);
    if(regs.Bytes.A == 0 && file_handle != null)
        *file_handle = regs.Bytes.B;
    
    return regs.Bytes.A;
}


byte ReadFromFile(byte file_handle, byte* destination, int* length)
{
    regs.Bytes.B = file_handle;
    regs.Words.DE = (int)destination;
    regs.Words.HL = *length;
    DosCall(F_READ, &regs, REGS_MAIN, REGS_MAIN);
    *length = regs.Words.HL;
    return regs.Bytes.A;
}


byte WriteToFile(byte file_handle, byte* source, int length)
{
    regs.Bytes.B = file_handle;
    regs.Words.DE = (int)source;
    regs.Words.HL = length;
    DosCall(F_WRITE, &regs, REGS_MAIN, REGS_AF);
    return regs.Bytes.A;
}


void CloseFile(byte file_handle)
{
    regs.Bytes.B = file_handle;
    DosCall(F_CLOSE, &regs, REGS_MAIN, REGS_NONE);
}


// This prevents disk errors from triggering an "Abort, Retry, Ignore?" prompt,
// which wouldn't be a nice behavior for a server.
void DisableDiskErrorPrompt() __naked
{
    __asm
    
    push    ix
    ld de,#DSKERR_CODE
    ld c,#F_DEFER
    call    #5
    ld  de,#ABORT_CODE
    ld  c,#F_DEFAB
    call    #5
    pop ix
    ret

    ;--- Disk error handling routine
DSKERR_CODE:
    ld a,#1  ;Always return "Abort"
    ret

    ;--- Program termination handling routine
ABORT_CODE:
    ld c,a
    cp #ERR_ABORT
    jr z,ABORT_CODE2
    cp #ERR_INERR
    jr z,ABORT_CODE2
    cp #ERR_OUTERR
    jr z,ABORT_CODE2
    ld a,c
    ret     ;Not a disk error abort -> the program is willingfully terminating
ABORT_CODE2:

    ;This causes execution to return to the caller of the DOS function call,
    ;instead of continuing to the termination of the program
    pop hl

    ld a,b  ;For ERR_ABORT, B contains the actual disk error code
    ret

    __endasm;
}


bool FunctionKeysAreVisible()
{
    return *((byte*)CNSDFG) == 0;
}

void DisplayFunctionKeys()
{
	BiosCall(DSPFNK, &regs, REGS_NONE);
}


void HideFunctionKeys()
{
	BiosCall(ERAFNK, &regs, REGS_NONE);
}


void InitializeFunctionKeysContents()
{
    BiosCall(INIFNK, &regs, REGS_NONE);
}


void SetFunctionKeyContents(int key, char* contents)
{
    memset(F_KEY_CONTENTS_POINTER(key), (int)' ', F_KEY_CONTENTS_LENGTH);
    if(contents)
	    strcpy(F_KEY_CONTENTS_POINTER(key), contents);
}


bool GetEnvironmentItem(const char* name, char* value)
{
    regs.Words.HL = (int)name;
    regs.Words.DE = (int)value;
    regs.Bytes.B = 255;
    DosCall(F_GENV, &regs, REGS_MAIN, REGS_NONE);
    return *value != '\0';
}


byte SetEnvironmentItem(const char* name, const char* value)
{
    regs.Words.HL = (int)name;
    regs.Words.DE = (int)value;
    DosCall(F_SENV, &regs, REGS_MAIN, REGS_AF);
    return regs.Bytes.A;
}


bool FindEnvironmentItem(uint index, char* name)
{
    regs.UWords.DE = index;
    regs.Words.HL = (int)name;
    regs.Bytes.B = 255;
    DosCall(F_FENV, &regs, REGS_MAIN, REGS_NONE);
    return *name != '\0';
}


char* GetPointerToLastItemOfPathname(const char* pathname)
{
    regs.Bytes.B = 0;
    regs.Words.DE = (int)pathname;
    DosCall(F_PARSE, &regs, REGS_MAIN, REGS_MAIN);
    if(regs.Bytes.A != 0)
        return null;
    
    return (char*)regs.Words.HL;
}


byte DuplicateFileHandle(byte fileHandle, byte* duplicatedFileHandle)
{
    regs.Bytes.B = fileHandle;
    DosCall(F_DUP, &regs, REGS_MAIN, REGS_MAIN);
    if(regs.Bytes.A == 0 && duplicatedFileHandle)
        *duplicatedFileHandle = regs.Bytes.B;
    return regs.Bytes.A;
}


byte DeleteFile(char* path)
{
    regs.Words.DE = (int)path;
    DosCall(F_DELETE, &regs, REGS_MAIN, REGS_AF);
    return regs.Bytes.A;
}


byte ParseFilename(const char* fileName, char* expandedFilename)
{
    regs.Words.DE = (int)fileName;
    regs.Words.HL = (int)expandedFilename;
    DosCall(F_PFILE, &regs, REGS_MAIN, REGS_AF);
    return regs.Bytes.A;
}


void RewindFile(byte fileHandle)
{
    regs.Bytes.B = fileHandle;
    regs.Bytes.A = 0; //Relative to beginning of file_handle
    regs.Words.DE = 0;
    regs.Words.HL = 0;
    DosCall(F_SEEK, &regs, REGS_MAIN, REGS_NONE);
}
