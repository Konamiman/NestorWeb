;Example of NestorWeb CGI script that returns a predefined error response.
;The available predefined error codes are:
;
;1: 400 Bad Request
;2: 404 Not Found
;3: 405 Method Not Allowed
;4: 304 Not Modified
;5: 401 Unauthorized (also sends WWW-Authenticate header)
;6: 403 Forbidden

_TERM: equ 62h

    ld b,3 ;The client will receive "405 Method Not Allowed"
    ld c,_TERM
    jp 5
