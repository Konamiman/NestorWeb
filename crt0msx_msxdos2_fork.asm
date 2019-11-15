	;--- crt0.asm for MSX-DOS 2 - by Konamiman, 1/2020
	;
    ;    Advanced version with fork:
    ;
    ;    - Defines main as "int main(char** argv, int argc)",
	;      the returned value will be passed to _TERM.
    ;
    ;    - Allows executing a separate program as a subprocess,
    ;      when the suprocess finishes the main program is reloaded,
    ;      see proc.h for details.
    ;
    ;    Assemble with sdasz80 -g -o crt0msx_msxdos2_fork.rel crt0msx_msxdos2_fork.asm
	;
    ;    Compile programs with --code-loc 0x410 --data-loc X
    ;    X=0  -> global vars will be placed immediately after code
    ;    X!=0 -> global vars will be placed at address X
    ;            (make sure that X>0x100+code size)

_TERM0 .equ #0x00
_STROUT .equ #0x09
_FFIRST .equ #0x40
_OPEN  .equ #0x43
_CLOSE .equ #0x45
_READ  .equ #0x48
_WPATH .equ #0x5E
_FORK  .equ #0x60
_JOIN  .equ #0x61
_TERM  .equ #0x62
_GENV  .equ #0x6B
_SENV  .equ #0x6C
_DOSVER .equ #0x6F


;---Possible errors returned by proc_fork

FORK_ERR_SMALL_TPA .equ 1
FORK_ERR_FILE_NOT_FOUND .equ 2
FORK_ERR_FILE_TOO_BIG .equ 3
FORK_ERR_FILE_OPEN .equ 4
FORK_ERR_FORKING .equ 5


	.globl	_main
    .globl  _proc_fork

    .globl  l__INITIALIZER
    .globl  s__INITIALIZED
    .globl  s__INITIALIZER

	.area _HEADER (ABS)


        .org    0x0100  ;MSX-DOS .COM programs start address

        ;Jump table for easier debugging: 0x100 = init, 0x103 = fork, 0x104 = join

        jp init
_proc_fork:
        jp do_proc_fork
join:
        jp do_join


;================================================================
;=====  Starting the program instance from scratch ("main") =====
;================================================================

        ;--- Step 1: Initialize globals

init:   call    gsinit
        call setup_put_px

        ;Save stack pointer to reset it when forking a subprocess

        ld hl,#0
        add hl,sp
        ld (orig_stack),hl

        ;--- Step 2: Build the parameter pointers table on 0x100,
        ;    and terminate each parameter with 0.
        ;    MSX-DOS places the command line length at 0x80 (one byte),
        ;    and the command line itself at 0x81 (up to 127 characters).

        ;* Check if there are any parameters at all

        ld      a,(#0x80)
        or      a
        ld      c,#0
        jr      z,cont
        
        ;* Copy the command line processing code to 0xC000 and
        ;  execute it from there, this way the memory of the original code
        ;  can be recycled for the parameter pointers table.
        ;  (The space from 0x100 up to "cont" can be used,
        ;   this is room for about 40 parameters.
        ;   No real world application will handle so many parameters.)
        
        ld      hl,#parloop
        ld      de,#0xC000
        ld      bc,#parloopend-#parloop
        ldir
        
        ;* Initialize registers and jump to the loop routine
        
        ld      hl,#0x81        ;Command line pointer
        ld      c,#0            ;Number of params found
        ld      ix,#init       ;Params table pointer
        
        ld      de,#cont        ;To continue execution at "cont"
        push    de              ;when the routine RETs
        jp      0xC000
        
        ;>>> Command line processing routine begin
        
        ;* Loop over the command line: skip spaces
        
parloop: ld      a,(hl)
        or      a       ;Command line end found?
        ret     z

        cp      #32
        jr      nz,parfnd
        inc     hl
        jr      parloop

        ;* Parameter found: add its address to params table...

parfnd: ld      (ix),l
        ld      1(ix),h
        inc     ix
        inc     ix
        inc     c
        
        ld      a,c     ;protection against too many parameters
        cp      #40
        ret     nc
        
        ;* ...and skip chars until finding a space or command line end
        
parloop2:       ld      a,(hl)
        or      a       ;Command line end found?
        ret     z
        
        cp      #32
        jr      nz,nospc        ;If space found, set it to 0
                                ;(string terminator)...
        ld      (hl),#0
        inc     hl
        jr      parloop         ;...and return to space skipping loop

nospc:  inc     hl
        jr      parloop2

parloopend:
        
        ;>>> Command line processing routine end
        
        ;* Command line processing done. Here, C=number of parameters.

cont:   ld      hl,#init
        ld      b,#0
        push    bc      ;Pass info as parameters to "main"
        push    hl

        ;--- Step 3: Call the "main" function

	    push de
	    ld de,#_HEAP_start
	    ld (_heap_top),de
	    pop de

	    call    _main

        ;--- Step 4: Program termination.
        ;    Termination code for DOS 2 was returned on L.
                
        ld      c,#0x62   ;DOS 2 function for program termination (_TERM)
        ld      b,l
        call    5      ;On DOS 2 this terminates; on DOS 1 this returns...
        ld      c,#0x0
        jp      5      ;...and then this one terminates
                       ;(DOS 1 function for program termination).


;==================================
;=====  Forking a subprocess  =====
;==================================

;Declarations needed in the program:
;
;extern unsigned char proc_fork(void* subprocess_file_or_fib, char* arguments_for_subprocess, void* state_data);
;unsigned char proc_join(unsigned char error_code_from_subprocess, void* state_data);
;
;See proc.h for details on how forking works.


;--- Contents of the temporary 256 bytes area allocated when forking

;Copy of CP/M serial number and BDOS jump
TMP_CPM_SERIAL_AND_BDOS_JUMP .equ 0

;Original destination of the REBOOT hook
TMP_REBOOT .equ 9

;Process ID returned by _FORK
TMP_PROCID .equ 11

;Original stack pointer, to be restored when rejoining
TMP_SP .equ 12

;Code that reloads the main program file
TMP_RELOAD_CODE .equ 14

;Path of the main program file
TMP_MAIN_PROG_PATH .equ 64

;Space to hold a copy of the state data passed to proc_fork
TMP_STATE_DATA .equ 128


do_proc_fork:

    ;Check TPA size

    ld a,(#7)
    cp #0xC4
    ld l,#FORK_ERR_SMALL_TPA
    ret c

    ;Get and save arguments

    ld iy,#2
    add iy,sp

    ld e,2(iy)
    ld d,3(iy)
    ld (fork_cmdline),de
    ld e,4(iy)
    ld d,5(iy)
    ld (fork_statedata),de
    ld e,(iy)
    ld d,1(iy)
    ld (fork_path),de

    ;If a path is supplied for the subprocess program ile,
    ;execute _FFIRST to get a FIB.
    ;Reuse the code area for "main" to hold the FIB.

    push de
    pop iy
    ld a,(de)
    cp #0xFF
    jr nz,fork_search_do

    ex de,hl
    ld de,#init
    ld bc,#64
    ldir
    jr fork_search_ok

fork_search_do:
    push ix
    ld ix,#init
    ld b,#0
    ld c,#_FFIRST
    call #5
    pop ix
    ld l,#FORK_ERR_FILE_NOT_FOUND
    ret nz

    ;Check subprocess program file size

fork_search_ok:
    ld iy,#init
    ld l,#FORK_ERR_FILE_TOO_BIG
    ld a,24(iy)
    or a
    ret nz
    ld a,23(iy)
    or a
    ret nz
    ld a,22(iy)
    cp #0xC0
    ret nc

    ;Open the subprocess program file

    push iy
    pop de
    ld a,#5  ;Open in inheritable mode (since we'll need to access it after the _FORK) + in read-only mode
    ld c,#_OPEN
    call #5
    ld l,#FORK_ERR_FILE_OPEN
    ret nz
    ld a,b
    ld (NEW_PROC_FH),a
    
    ;Execute _FORK

    ld c,#_FORK
    call #5
    jr z,fork_ok

    ld a,(NEW_PROC_FH)
    ld b,a
    ld c,#_CLOSE
    call #5
    ld l,#FORK_ERR_FORKING
    ret

fork_ok:    
    ld a,b
    ld (NEW_PROC_ID),a

    ;>>> All preconditions are met, we'll not return any error from now

    ;Allocate 256 bytes right below the current TPA end.
    ;The size needs to be 256 because the low byte of the TPA end
    ;is required by MSX-DOS to be fixed.
    ;We'll use this area to store some temporary data and code
    ;(see TMP_* declarations).

    ld bc,(orig_stack)
    ld hl,#0
    or a
    sbc hl,bc
    ld hl,#-256
    add hl,sp
    ld sp,hl

    ;Setup the copy of CP/M serial number and jump to BDOS

    ld de,(#6)
    ld e,#0
    push de
    dec d
    pop hl
    ld bc,#9
    push de
    ldir
    pop iy

    ;Save a copy of the original stack, to be restored when rejoining

    ld hl,(orig_stack)
    ld TMP_SP(iy),l
    ld TMP_SP+1(iy),h

    ;Save a copy of the original reboot destination address.
    ;The reboot hook (at address 0) contains a JP to a location
    ;that in turn contains another JP, we save the target
    ;of that second JP.
    ;We need to do it this way because MSXDOS2.SYS resets the
    ;contents of the hook at address 0 when _TERM is executed.

    ld hl,(#1)
    inc hl
    ld a,(hl)
    inc hl
    ld h,(hl)
    ld l,a
    ld TMP_REBOOT(iy),l
    ld TMP_REBOOT+1(iy),h

    ;Save the subprocess id to be passed to _JOIN

    ld a,(NEW_PROC_ID)
    ld TMP_PROCID(iy),a

    ;Save the main program path so that we can reload it later

    ld c,#_GENV
    ld hl,#PROGRAM_S
    push iy
    pop de
    ld e,#TMP_MAIN_PROG_PATH
    ld b,#64
    call #5

    ;Setup PROGRAM variable for the subprocess

    ld a,(init+#25)
    add #64 ;"A"-1
    ld (init),a
    ld a,#58 ;":"
    ld (init+#1),a
    ld a,#92 ;"\"
    ld (init+#2),a
    ld de,#init+#3
    ld c,#_WPATH
    call #5
    ld de,#init
    ld hl,#PROGRAM_S
    ld c,#_SENV
    call #5

    ;Setup the command line arguments for the new process,
    ;both at address 0x80 and in the PARAMETERS variable

    ld de,(fork_cmdline)
    ld b,#0
    ld hl,#0x81

    ld a,d
    or e
    jr z,CMDLINEOK

    ld a,(de)
    or a
    jr z,CMDLINEOK

    inc b
    ld (hl),#32  ;Prepend a space
    inc hl

CMDLINELOOP:
    ld a,b
    cp #126
    jr z,CMDLINEOK
    ld a,(de)
    or a
    jr z,CMDLINEOK
    ld (hl),a
    inc hl
    inc de
    inc b
    jr CMDLINELOOP    

CMDLINEOK:
    ld (hl),#0
    ld a,b
    ld (0x80),a
    ld c,#_SENV
    ld hl,#PARAMETERS_S
    ld de,#0x81
    call #5

    ;Copy the code that reloads the main process

    ld hl,#RELOAD
    push iy
    pop de
    ld e,#TMP_RELOAD_CODE
    ld bc,#RELOAD_END-#RELOAD
    ldir

    ;Copy the state data if present

    ld hl,(fork_statedata)
    ld a,h
    or l
    jr z,fork_nostate
    push iy
    pop de
    ld e,#TMP_STATE_DATA
    ld bc,#128
    ldir
fork_nostate:

    ;Update the BDOS jump so that it points to the copy
    ;(needed in order to signal the new end of TPA)

    ld a,(#7)
    dec a
    ld (#7),a

    ;Update the reboot hook to point to the code that
    ;reloads the main process

    push iy
    pop hl
    ld bc,#TMP_RELOAD_CODE
    add hl,bc
    ex de,hl
    ld hl,(#1)
    inc hl
    ld (hl),e
    inc hl
    ld (hl),d

    ;Copy the code that loads and runs the subprocess program file to end of stack
    ;(needed because we'll overwrite the entire TPA with the file contents)
    ;and execute it.

    ld hl,#RUN_SCRIPT-#RUN_SCRIPT_END ;Actually -(RUN_SCRIPT_END-RUN_SCRIPT)
    add hl,sp
    ld (#DO_RUN_SCRIPT+1),hl
    ld sp,hl
    ex de,hl
    ld hl,#RUN_SCRIPT
    ld bc,#RUN_SCRIPT_END-#RUN_SCRIPT
    ldir
DO_RUN_SCRIPT:
    jp 0


    ;--- Code that loads the subprocess program file and executes it

RUN_SCRIPT:
    ld a,(NEW_PROC_FH)
    ld b,a

    push bc
    ld c,#_READ
    ld de,#0x100
    ld hl,#0xC000
    call #5 ;TODO: What if this fails?

    pop bc
    ld c,#_CLOSE
    call #5

    ;The -2 is to make the stack point to DIR_RET_POINTER
    ld hl,#RUN_SCRIPT_END-#RUN_SCRIPT-2
    add hl,sp
    ld sp,hl

    jp 0x100

DIR_RET_POINTER:
    .ds 2    ;Zero, so RET with original stack pointer is equivalent to JP 0
RUN_SCRIPT_END:


    ;--- Code that reloads the main program, must be relocatable.
    ;    This code must be as small as possible, so after reloading
    ;    jump straight to "join" and continue the work there.

RELOAD:
    ld a,#3
PUTP0:
    call 0
    dec a
PUTP1:    
    call 0
    dec a
PUTP2:
    call 0

    ;Try to close file handle 62 to make sure that there's at least one handle free.
    ;Maximum file handle number in MSX-DOS 2 is 63, but due to a bug
    ;the _CLOSE function will always return "Invalid file handle" for handle 63.
    ld bc,#_CLOSE + #0x3E00
    call #5

    ld de,(#6) ;DE = TPA temporary area created by us
    ld e,#TMP_MAIN_PROG_PATH
    ld c,#_OPEN
    ld a,#1  ;Open as read-only
    call #5
    push bc

    ld c,#_READ
    ld de,#0x100
    ld hl,#0xC000
    call #5

    jp join
RELOAD_END:


;--- Temporary variables for the forking process

fork_path: .dw 0
fork_cmdline: .dw 0
fork_statedata: .dw 0
NEW_PROC_FH: .db 0
NEW_PROC_ID: .db 0


;=========================================
;=====  Rejoining from a subprocess  =====
;=========================================

do_join:
    
    ;Close main program file handle

    pop bc
    ld c,#_CLOSE
    call #5

    ld ix,(#6)  ;Data area allocated by us
    .db #0xDD, #0x2E, #0    ;"ld ixl,0"

    ;Initialize main program global variables again

    ld sp,#0xC400   ;Temporarily set stack at sensible location (setup_put_px requires it at page 3)

    call gsinit
    call setup_put_px
    ld de,#_HEAP_start
    ld (_heap_top),de

    ;Restore PROGRAM environment variable

    push ix
    pop de
    ld e,#TMP_MAIN_PROG_PATH
    ld hl,#PROGRAM_S
    ld c,#_SENV
    call #5

    ;Execute _JOIN to "officially" end the subprocess

    ld b,TMP_PROCID(ix)
    ld c,#_JOIN
    call 5
    ld d,b  ;Error code returned by subprocess
    push de

    ;Restore original reboot hook contents

    ld hl,(#1)   
    inc hl
    ld a,TMP_REBOOT(ix)
    ld (hl),a
    inc hl
    ld a,TMP_REBOOT+1(ix)
    ld (hl),a

    ;Restore BDOS jump, thus increasing TPA size back by 256 bytes

    ld a,(#7)
    inc a
    ld (#7),a

    ;Copy state data to a survivable location,
    ;we'll recycle the area for the command line arguments

    push ix
    pop hl
    ld l,#TMP_STATE_DATA
    ld de,#0x80
    ld bc,#128
    ldir

    ;Reset stack pointer to the value it had at "main"

    pop bc ;Error code returned by subprocess
    ld l,TMP_SP(ix)
    ld h,TMP_SP+1(ix)
    ld sp,hl
    ld (orig_stack),hl ;Needed to be able to re-fork

    ;Push arguments for proc_join, then execute it

    ld de,#0x80
    push de
    push bc
    inc sp
    call _proc_join   ;This is an external symbol! Assembly with -g is required.
    inc sp
    pop de
    ld b,l
    ld c,#_TERM
    jp 5


    ;--- Auxiliary code, strings and variables

setup_put_px:
    push ix
    xor a
    ld de,#0x0402
    call #0xFFCA
    pop ix
    or a
    ret z
    ld bc,#0x18
    add hl,bc   ;HL = PUT_P1
    ld de,#0xC000
    ld bc,#15
    ldir    ;Copy PUT_P0 to PUT_P2 to known location

    ld hl,(#0xC000+#1)
    ld (#PUTP0+1),hl
    ld hl,(#0xC000+#6+#1)
    ld (#PUTP1+1),hl
    ld hl,(#0xC000+#12+#1)
    ld (#PUTP2+1),hl

    ret


PROGRAM_S: 
        .strz "PROGRAM"

PARAMETERS_S:
        .strz "PARAMETERS"

orig_stack: .dw 0


;=================================================
;=====  Program code and data (global vars)  =====
;=================================================

	;* Place data after program code, and data init code after data

	.area	_CODE
	.area	_DATA
_heap_top::
	.dw 0

        .area   _GSINIT
gsinit::
        ld	bc,#l__INITIALIZER
        ld	a,b
        or	a,c
        jp	z,gsinext
        ld	de,#s__INITIALIZED
        ld	hl,#s__INITIALIZER
        ldir
gsinext:
        .area   _GSFINAL
        ret

	;* These don't seem to be necessary... (?)

        ;.area  _OVERLAY
	;.area	_HOME
        ;.area  _BSS
	.area	_HEAP

_HEAP_start::
