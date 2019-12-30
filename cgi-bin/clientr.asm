;Example of NestorWeb CGI script that performs a client redirect.
;The client will receive a "302 Found" status.

CR: equ 13
LF: equ 10

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
    db "Location: http://www.konamiman.com",CR,LF
    db CR,LF
    ;Note: request body will be ignored
REDIR_END: