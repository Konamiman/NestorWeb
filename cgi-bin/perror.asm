;Example of NestorHTTP CGI script that returns a predefined error message.
;This will send "405 Method Not Allowed" to the client.

_TERM: equ 62h

    ld b,3
    ld c,_TERM
    jp 5
