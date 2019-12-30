;Example of a NestorWeb CGI script that sends a custom status code and message.

CR: equ 13
LF: equ 10

STDOUT: equ 1
_WRITE: equ 49h

    org 100h

    ld de,HELLO
    ld hl,HELLO_END-HELLO
    ld b,STDOUT
    ld c,_WRITE
    call 5

    ret

HELLO:
    db "Status: 234 It's all okay",CR,LF
    db CR,LF
    db "Hello, world!!"
HELLO_END:

