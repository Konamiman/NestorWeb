;Example of NestorHTTP SCGI script that returns a response body from a content file.
;The file for this example is to be located at <base directory>\cgi-bin\contentf\datafile.dat

STDOUT: equ 1
_WRITE: equ 49h

    org 100h

    ld b,STDOUT
    ld de,HEADERS
    ld hl,HEADER_END-HEADERS
    ld c,_WRITE
    call 5
    ret

HEADERS:
    db "X-CGI-Content-File: contentf\\datafile.dat",13,10
    db 13,10
    ;Note: response body is ignored (the file contents is sent instead)
HEADER_END:
