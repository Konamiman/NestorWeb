;Example of NestorWeb CGI script that returns a NPH (Non-Parsed Headers) response.
;Anything past the first line will be sent verbatim to the client,
;bypassing any additional processing.
;This means that the script must provide the status code+message
;and all headers, including Content-Length.

CR: equ 13
LF: equ 10

STDOUT: equ 1
_WRITE: equ 49h

    org 100h

    ld de,NPH
    ld hl,NPH_END-NPH
    ld b,STDOUT
    ld c,_WRITE
    call 5

    ret

NPH:
    db "X-CGI-Response-Type: NPH",CR,LF

    db "HTTP/1.1 234 Ok, NPH style",CR,LF
    db "Server: NestorWeb, but NPH-ed",CR,LF
    db "Content-Type: text/plain",CR,LF
    db "Content-Length: 3",CR,LF
    db CR,LF
    db "MSX"
NPH_END:
