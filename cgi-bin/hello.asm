;Hello world - NestorWeb CGI version.
;Includes a custom response header.

CR: equ 13
LF: equ 10
STDOUT: equ 1
_WRITE: equ 49h
_TERM:  equ 62h

    org 100h

    ld de,HELLO
    ld hl,HELLO_END-HELLO
    ld b,STDOUT
    ld c,_WRITE
    call 0005h
    ld b,a  ;If can't write to STDOUT, terminate with error
    ld c,_TERM
    jp 0005h

HELLO:
    db "Content-Type: text/plain",CR,LF
    db "X-Custom-Header: Konamiman rocks",CR,LF
    db CR,LF
    db "Hello, world!"
HELLO_END:
