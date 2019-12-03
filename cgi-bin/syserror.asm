;Example of NestorHTTP CGI script that retuns an error message
;using the system-defined markup.

STDOUT: equ 1
_WRITE: equ 49h

    org 100h

    ld de,ERROR
    ld hl,ERROR_END-ERROR
    ld b,STDOUT
    ld c,_WRITE
    call 5

    ret

ERROR:
    db "X-CGI-Error: 434 Bad Foobar",13,10
    ;Note: no additional headers allowed here.
    db 13,10
    db "This <b>foobar</b> isn't <i>fizzbuzzed</i> enough."
ERROR_END