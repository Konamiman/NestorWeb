;NestorWeb CGI script that returns all the environment items defined in the server
;and the request body, if any, assuming that it's text.
;This can be useful to understand how CGI related environment variables work.

_TERM0: equ 0
_CONOUT: equ 2
_STROUT: equ 9
_READ: equ 48h
_WRITE: equ 49h
_TERM: equ 62h
_GENV: equ 6Bh
_FENV: equ 6Dh

STDIN: equ 0
STDOUT: equ 1

CR: equ 13
LF: equ 10

    org 100h

    ;--- Environment items

    ld hl,BUFFER
    ld de,1

LOOP:
    push de
    
    ld b,255
    ld c,_FENV
    call 5

    ld a,(hl)   ;Empty result = no more items
    or a
    jr z,END

    push hl     ;Save name
    call ADVANCE_TO_ZERO
    ld (NAME_END),hl    ;We need the name to end with 0 for now, but later we want to replace it with a space
    inc hl
    ld a,'='
    ld (hl),a
    inc hl
    ld a,' '
    ld (hl),a
    inc hl

    ex de,hl    ;DE = value
    pop hl      ;HL = name
    push de
    ld b,255
    ld c,_GENV
    call 5

    ld hl,(NAME_END)
    ld a,' '
    ld (hl),a
    pop hl
    call ADVANCE_TO_ZERO
    ld a,CR
    ld (hl),a
    inc hl
    ld a,LF
    ld (hl),a
    inc hl

    pop de
    inc de

    jr LOOP

END:
    pop de
    ld bc,HEADERS
    or a
    sbc hl,bc
    ld de,HEADERS
    ld b,STDOUT
    ld c,_WRITE
    call 5

    ;--- Request body

    ld b,STDIN
    ld de,8000h
    ld hl,4000h
    ld c,_READ
    call 5
    ld c,_TERM0
    jp nz,5     ;If no request body is present _READ will return EOF error

    push hl ;Request body size

    ld de,BODY_TITLE
    ld hl,BODY_TITLE_END-BODY_TITLE
    ld b,STDOUT
    ld c,_WRITE
    call 5
    
    pop hl
    ld de,8000h
    ld b,STDOUT
    ld c,_WRITE
    call 5

    ld c,_TERM0
    jp 5

    ;--- Subroutines

ADVANCE_TO_ZERO:
    ld a,(hl)
    or a
    ret z
    inc hl
    jr ADVANCE_TO_ZERO

NAME_END: dw 0

ENV_TITLE:
    db "Environment items:",CR,LF,CR,LF
ENV_TITLE_END:

BODY_TITLE:
    db CR,LF,"Request body:",CR,LF,CR,LF
BODY_TITLE_END:

HEADERS:
    db "Content-Type: text/plain",CR,LF
    db CR,LF
    db "Environment items:",CR,LF
    db CR,LF
BUFFER:
