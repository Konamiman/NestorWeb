;Hello world - NestorHTTP CGI version.
;Includes a custom response header.

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
    db "Content-Type: text/plain",13,10
    db "X-Custom-Header: Foobar",13,10
    db 13,10
    db "Hello, world!!"
HELLO_END:
