;Example of NestorHTTP CGI script that performs a client redirect.
;The client will receive a "302 Found" status.

STDOUT: equ 1
_WRITE: equ 49h

    org 100h

    ld de,REDIR
    ld hl,REDIR_END-REDIR
    ld b,STDOUT
    ld c,_WRITE
    call 5

    ret

REDIR:
    db "Location: http://www.konamiman.com",13,10
    db 13,10
    ;Note: request body will be ignored
REDIR_END: