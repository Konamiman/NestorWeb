// proc.h file for programs using crt0msx_msxdos2_fork.rel

#ifndef __PROC_H
#define __PROC_H


//--- Possible errors returned by proc_fork

//TPA is too small (ends below 0xC400)
#define FORK_ERR_SMALL_TPA 1

//Subprocess file not found
#define FORK_ERR_FILE_NOT_FOUND 2

//Subprocess file is too big (maximum size is 48k)
#define FORK_ERR_FILE_TOO_BIG 3

//Error opening subprocess file
#define FORK_ERR_FILE_OPEN 4

//Error executing _FORK
#define FORK_ERR_FORKING 5


/*
--- Program entry point

argv is an array of strings containing the command line arguments,
argc is the count of items in argv.
The return value will be pased to the _TERM function.
*/
int main(char** argv, int argc);


/*
--- Execute another program as a subprocess
    (external function, supplied by crt0msx_msxdos2_fork.rel)

This function loads and executes the specified subprocess file at 0x100
with the specified arguments, and when it terminates, 
the main process program is reloaded and its proc_join function is executed.

The subprocess file must be a standard MSX-DOS .COM executable program
and will receive the PROGRAM and PARAMETERS environment variables
set as expected, as well as the command line arguments at address 0x80;
but will NOT receive the first two arguments converted to FCBs
at 0x005C and 0x006C.

The subprocess file will be executed inside a _FORK/_JOIN cycle,
so it can open files and allocate memory segments and everything
will be cleared up before returning to the main program.

If everything goes well, this function of course doesn't return.
Otherwise it returns an error (one of FORK_ERR_*).

If a FIB is passed in subprocess_file_or_fib, it must be the most recent FIB obtained
with a call to _FFIRST or _FNEXT, since the _WPATH function is used to setup the PROGRAM
variable for the subprocess.

arguments_for_subprocess can be 0 if you don't want to pass arguments
to the subprocess program.

The data supplied in state_data will be passed back to proc_join 
(but not in the same memory location), the maximum length for such data is 128 bytes.
If no state data is required to be saved, 0 can be passed instead.
*/
extern unsigned char proc_fork(void* subprocess_file_or_fib, char* arguments_for_subprocess, void* state_data);


/*
--- Return from a subprocess

error_code_from_subprocess is the error code supplied by the subprocess program
when executing _TERM (0 if it executes _TERM0 instead, or if it does RET or JP 0).

Note that when proc_join runs, the PROGRAM environment item will be correctly set
as the path of the main program; but the command line arguments
(both at address 0x80 and in the PARAMETERS variable) will be the ones that
were passed to the subprocess. If you need to keep information originally supplied
as arguments to the main program, pass it to proc_fork as state data.

This function acts as "main": the program terminates when the function returns,
and the returnd value will be passed to the _TERM function.
*/
unsigned char proc_join(unsigned char error_code_from_subprocess, void* state_data);

#endif
