_TERM0: equ 00h
_STROUT: equ 09h
_OPEN: equ 43h
_CREATE: equ 44h
_CLOSE: equ 45h
_DUP: equ 47h
_READ: equ 48h
_FORK: equ 60h
_JOIN: equ 61h
_DEFAB: equ 63h
_GENV: equ 6Bh
_SENV: equ 6Ch

;Contents of the temporary 256 bytes area allocated

TMP_REBOOT: equ 9
TMP_PROCID: equ 11
TMP_ERRCODE: equ 12
TMP_SP: equ 13
TMP_OUT_FH: equ 15
TMP_RELOAD_CODE: equ 16

    org 100h

    ;--- Get arguments, terminate if none

    ld c,_GENV
    ld hl,PARAMETERS_S
    ld de,BUFFER
    ld b,255
    call 5
    ld a,(de)
    or a
    jr nz,OK_PARAMS

    ld de,WELCOME_S
    ld c,_STROUT
    call 5

    ld de,USAGE_S
    ld c,_STROUT
    call 5
    
    ld c,_TERM0
    jp 5
OK_PARAMS:

    ;--- If argument is "*", we are returning from the script

    ld a,(BUFFER)
    cp "*"
    jr nz,NOT_RETURNING

    ld hl,(6)
    ld l,0
    push hl
    pop ix  ;IX = data area created by us
    
    ld a,(ix+TMP_ERRCODE)
    add "0"
    ld (RETRUNED_ERRCODE),a

    ld de,RETURNED_S
    ld c,_STROUT
    call 5

    ld hl,(1)   ;Restore reboot address
    inc hl
    ld a,(ix+TMP_REBOOT)
    ld (hl),a
    inc hl
    ld a,(ix+TMP_REBOOT+1)
    ld (hl),a

    ld a,ixh
    inc a
    ld (7),a

    ld c,_TERM0
    jp 5

NOT_RETURNING:
    ld de,WELCOME_S
    ld c,_STROUT
    call 5

    ld de,RUNNING_S
    ld c,_STROUT
    call 5

    ;--- Allocate 256 byte work area right below the BDOS jump area.
    ;    Contents of this area:
    ;    - Copy of CP/M serial number and jump to BDOS
    ;    - Space for process id returned by _FORK
    ;    - Space for error code returned by _JOIN
    ;    - File handle of the file where the output is redirected
    ;    - yyyy here:
    ;      0000: jp xxxx
    ;      xxxx: jp yyyy
    ;    - Code that reloads this program, to be the new destination of the reboot jump (yyyy above)

    ld c,_GENV
    ld hl,PROGRAM_S
    ld de,PROGRAM_PATH_RELOAD
    ld b,64
    call 5

    exx
    ld hl,0
    add hl,sp
    exx

    ld hl,-256  ;Lower stack by the same amount we allocate
    add hl,sp
    ld sp,hl

    ld de,(6)   ;Setup +0..+8
    ld e,0
    push de
    dec d
    pop hl
    ld bc,9
    push de
    ldir
    pop ix

    exx
    ld (ix+TMP_SP),l
    ld (ix+TMP_SP+1),h
    exx

    ld hl,(1)   ;Setup reboot address
    inc hl
    ld a,(hl)
    inc hl
    ld h,(hl)
    ld l,a
    ld (ix+TMP_REBOOT),l
    ld (ix+TMP_REBOOT+1),h

    ld c,_FORK
    call 5
    ld a,b
    ld (RL_REJOIN+1),a

    ld a,ixh
    ld (RL_PROGRAM_PATH+2),a
    ld (RL_PARAMETERS+2),a
    ld (RL_ASTERISK+2),a

    ld (DIRECT_RET_POINTER+1),a
    ld hl,DIRECT_RET_TARGET
    ld a,l
    ld (DIRECT_RET_POINTER),a

    push ix
    pop hl
    ld bc,TMP_RELOAD_CODE
    add hl,bc
    ex de,hl
    ld hl,RELOAD_CODE
    ld bc,RELOAD_CODE_END-RELOAD_CODE
    ldir

    ;--- Now update the BDOS jump and the reboot jump

    ld a,ixh
    ld (7),a

    push ix
    pop hl
    ld bc,TMP_RELOAD_CODE
    add hl,bc
    ex de,hl
    ld hl,(1)
    inc hl
    ld (hl),e
    inc hl
    ld (hl),d

    ;--- Create the output file, save its FH, duplicate to standard output

    ld c,_CREATE
    ld de,OUTFILE_S
    xor a
    ld b,0
    call 5
    ld a,b
    ld (ix+TMP_OUT_FH),a

    push bc
    ld b,1  ;Standard output
    ld c,_CLOSE
    call 5
    pop bc

    ld c,_DUP
    call 5

    ;--- Open the program, load it and run it

    ld hl,-(RUN_SCRIPT_END-RUN_SCRIPT)
    add hl,sp
    ld (DO_RUN_SCRIPT+1),hl
    ld sp,hl
    ex de,hl
    ld hl,RUN_SCRIPT
    ld bc,RUN_SCRIPT_END-RUN_SCRIPT
    ldir

    ld de,BUFFER
SKIPSP:
    ld a,(de)
    cp " "
    jr nz,DO_RUN_SCRIPT
    inc de
    jr SKIPSP
DO_RUN_SCRIPT:
    jp 0

RUN_SCRIPT:
    ld c,_OPEN
    ld a,1  ;read-only
    call 5

    push bc
    ld c,_READ
    ld de,100h
    ld hl,16384
    call 5

    pop bc
    ld c,_CLOSE
    call 5

    ld hl,RUN_SCRIPT_END-RUN_SCRIPT-2   ;The -2 makes the stack point to a 0, so that the program can just RET
    add hl,sp
    ld sp,hl

    jp 100h

DIRECT_RET_POINTER:
    ds 2    ;This is the 0 the stack will point to
RUN_SCRIPT_END:

    ;--- Code that reloads this program

    ds (($ & 0FF00h)+256)+TMP_RELOAD_CODE-$

RELOAD_CODE:
    ld hl,(6)
    ld l,0
    push hl
    pop ix  ;IX = data area created by us

    ;Note that this causes the stack to point to the end
    ;of the same 256 bytes area where this code is running
    ld l,(ix+TMP_SP)
    ld h,(ix+TMP_SP+1)
    ld sp,hl

    ld b,(ix+TMP_OUT_FH)
    ld c,_CLOSE
    call 5

RL_REJOIN:
    ld b,0
    ld c,_JOIN
    call 5

    ld (ix+TMP_ERRCODE),b

    ld de,0
    ld c,_DEFAB
    call 5

RL_PROGRAM_PATH:
    ld de,PROGRAM_PATH_RELOAD
    ld c,_OPEN
    ld a,1  ;read-only
    call 5

    push bc
    ld c,_READ
    ld de,100h
    ld hl,16384
    call 5

    pop bc
    ld c,_CLOSE
    call 5

RL_PARAMETERS:
    ld hl,PARAMETERS_S_RELOAD
RL_ASTERISK:
    ld de,ASTERISK_S_RELOAD
    ld c,_SENV
    call 5

    jp 100h

DIRECT_RET_TARGET:
    ld c,_TERM0
    jp 5

PARAMETERS_S_RELOAD:
    db "PARAMETERS",0

ASTERISK_S_RELOAD:
    db "*",0

PROGRAM_PATH_RELOAD:
    ds 64

RELOAD_CODE_END:

    ;--- Strings

WELCOME_S:
    db "Executable file runner test",13,10
    db "(c) Konamiman 2019",13,10
    db 13,10
    db "$"

USAGE_S:
    db "Usage: runner <executable file path>",13,10,"$"

RUNNING_S:
    db "Running program...",13,10,"$"

RETURNED_S:
    db "Returned from program! Error code: "
RETRUNED_ERRCODE: 
    db 0
    db 13,10,"$"

PARAMETERS_S:
    db "PARAMETERS",0

PROGRAM_S:
    db "PROGRAM",0

OUTFILE_S:
    db "B:OUT.DAT",0

BUFFER:    
