;Example of NestorWeb CGI script that returns a predefined error message.
;The available predefined error codes are:
;
;1: 400 Bad Request
;2: 404 Not Found
;3: 405 Method Not Allowed
;4: 304 Not Modified
;5: 401 Unauthorized (also sends WWW-Authenticate header)
;6: 403 Forbidden

_TERM: equ 62h

    ld b,3
    ld c,_TERM
    jp 5
