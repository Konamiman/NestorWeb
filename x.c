#include <stdio.h>
#include "asm.h"
#include "proc.h"

#define _GENV 0x6B
#define _FFIRST 0x40

const char* theString = "Hello";
int theNum = 1234;
Z80_registers regs;

int main(char** argv, int argc)
{
    byte bufer[128];
    byte error;
    byte fib[64];
    char* filename;

    regs.Bytes.B = sizeof(bufer);
    regs.Words.HL = (int)"PARAMETERS";
    regs.Words.DE = (int)bufer;
    DosCall(_GENV, &regs, REGS_MAIN, REGS_NONE);
    printf("My PARAMETERS: %s\r\n", bufer);
    printf("My command line at 80h: %s\r\n", (char*)0x81);
    printf("Will fork. Globals: %i, %s\r\n", theNum, theString);
    
    filename = argc == 0 ? "DO.COM" : argv[0];
    regs.Words.DE = (int)filename;
    regs.Words.IX = (int)fib;
    regs.Bytes.B = 0;
    DosCall(_FFIRST, &regs, REGS_ALL, REGS_MAIN);
    if(regs.Bytes.A != 0) return regs.Bytes.A;

    error = proc_fork(fib, "some parameters al usez", "This is some damn good state data if you ask me!!! Mira, yo no obtengo beneficio, pero es por el sistema. Anda que nooo!!! 34.");
    printf("*** Error forking: %i\r\n", error);
    return 89;
}

byte proc_join(byte error_code, void* state_data)
{
    byte bufer[128];

    printf("Returned: %i\r\n", error_code);
    printf("Globals: %i, %s\r\n", theNum, theString);
    printf("State: %x, %s\r\n", (int)state_data, (char*)state_data);

    regs.Bytes.B = sizeof(bufer);
    regs.Words.HL = (int)"PARAMETERS";
    regs.Words.DE = (int)bufer;
    DosCall(_GENV, &regs, REGS_MAIN, REGS_NONE);
    printf("My PARAMETERS after join: %s\r\n", bufer);

    regs.Bytes.B = sizeof(bufer);
    regs.Words.HL = (int)"PROGRAM";
    regs.Words.DE = (int)bufer;
    DosCall(_GENV, &regs, REGS_MAIN, REGS_NONE);
    printf("My PROGRAM after join: %s\r\n", bufer);

    return 0x50;
}
