;Example of NestorHTTP CGI script that returns a non-zero error message.
;This will send "500 Internal Server Error" to the client.

_TERM: equ 62h

    ld b,34h
    ld c,_TERM
    jp 5
