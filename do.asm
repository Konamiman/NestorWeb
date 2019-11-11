_CONOUT: equ 2
_STROUT: equ 9
_CLOSE: equ 45h
_GENV: equ 6Bh

    org 100h

    ld de,myparams_s
    ld c,_STROUT
    call 5

    ld hl,PARAMETERS_S
    ld de,bufer
    ld b,128
    ld c,_GENV
    call 5

    call PRINT

    ld de,myprog_s
    ld c,_STROUT
    call 5

    ld hl,PROGRAM_S
    ld de,bufer
    ld b,128
    ld c,_GENV
    call 5

    call PRINT

    ld b,123
    ld c,62h
    jp 5

PRINT:
    ld hl,bufer
loop:
    ld a,(hl)
    or a
    jr z,loopend
    ld e,a
    ld c,_CONOUT
    push hl
    call 5
    pop hl
    inc hl
    jr loop
loopend:

    ;ld b,0
    ;ld c,_CLOSE
    ;call 5

    ld de,crlf
    ld c,_STROUT
    jp 5        

PARAMETERS_S: db "PARAMETERS",0
PROGRAM_S: db "PROGRAM",0
myprog_s: db "!My PROGRAM: $",0
myparams_s: db "!My PARAMETERS: $",0
crlf: db 13,10,"$"

bufer: