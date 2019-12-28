;Example of NestorWeb CGI script that performs a local redirect.
;The server will respond as if the request had been "GET /foo/bar.txt"

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
    db "Location: /foo/bar.txt",CR,LF
    ;Note: additional headers and request body will be ignored
    db CR,LF
REDIR_END: