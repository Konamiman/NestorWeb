;Example of NestorHTTP CGI script that returns a NPH (Non-Parsed Headers) response.
;Anything past the first line will be sent verbatim to the client,
;bypassing any additional processing.
;This means that the script must provide the status code+message
;and all headers, including Content-Length.

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
    db "X-CGI-Response-Type: NPH",13,10

    db "HTTP/1.1 234 Ok, NPH style",13,10
    db "Server: NestorHTTP, but NPH-ed",13,10
    db "Content-Type: text/plain",13,10
    db "Content-Length: 3",13,10
    db 13,10
    db "MSX"
NPH_END:
