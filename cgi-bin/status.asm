;Example of a NestorHTTP CGI script that sends a custom status code and message.

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
    db "Status: 234 It's all okay",13,10
    db 13,10
    db "Hello, world!!"
HELLO_END:

