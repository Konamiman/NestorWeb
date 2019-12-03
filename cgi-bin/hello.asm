;Hello world - NestorHTTP CGI version.
;Includes a custom response header.

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
    db "Content-Type: text/plain",CR,LF
    db "X-Custom-Header: Foobar",CR,LF
    db CR,LF
    db "Hello, world!!"
HELLO_END:
