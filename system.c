#include "system.h"
#include "types.h"
#include "asm.h"
#include "msxdos.h"

static Z80_registers regs;

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
