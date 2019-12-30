;Example of NestorWeb CGI script that retuns an error message
;using the system-defined markup.

CR: equ 13
LF: equ 10

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
    db "X-CGI-Error: 434 Bad Foobar",CR,LF
    ;Note: no additional headers allowed here.
    db CR,LF
    db "This <b>foobar</b> isn't <i>fizzbuzzed</i> enough."
ERROR_END