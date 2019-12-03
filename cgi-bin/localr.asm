;Example of NestorHTTP CGI script that performs a local redirect.
;The server will respond as if the request had been "GET /foo/bar.txt"

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
    db "Location: /foo/bar.txt",13,10
    ;Note: additional headers and request body will be ignored
    db 13,10
REDIR_END: