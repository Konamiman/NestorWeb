;NestorHTTP CGI script to return the current date on the server.
;This one uses _CONOUT and _STROUT to write the request body to STDOUT,
;but using _WRITE instead is generally recommended.

_CONOUT: equ 2
_STROUT: equ 9
_GDATE: equ 2Ah
_GTIME: equ 2Ch

    org 100h

    ld de,HEADERS
    ld c,_STROUT
    call 5

    ;Date

    ld c,_GDATE     ;HL = year, D = month, E = day
    call 5
    
    call HL2ASC     ;Year
    call PRINTBUF

    call PRINTDASH

    ld a,d
    call BYTE2ASC   ;Month
    call PRINTBUF

    call PRINTDASH

    ld a,e
    call BYTE2ASC   ;Day
    call PRINTBUF

    ld e,' '
    ld c,_CONOUT
    call 5

    ;Time

    ld c,_GTIME      ;H = hour, D = minute, E = seconds
    call 5

    push hl
    ld a,h
    call BYTE2ASC   ;Hour
    call PRINTBUF

    call PRINTCOLON

    pop hl
    ld a,l
    call BYTE2ASC   ;Minute
    call PRINTBUF

    call PRINTCOLON

    ld a,d
    call BYTE2ASC   ;Second
    call PRINTBUF

    ret


PRINTDASH:
    push hl
    push de
    ld e,'-'
    ld c,_CONOUT
    call 5
    pop de
    pop hl
    ret

PRINTCOLON:
    push hl
    push de
    ld e,':'
    ld c,_CONOUT
    call 5
    pop de
    pop hl
    ret

PRINTBUF:
    push hl
    push de
    ld a,(buf+1)
    cp "0"
    jr z,PRINTBUF2
    ld e,a
    ld c,_CONOUT
    call 5
    ld a,(buf+2)
    ld e,a
    ld c,_CONOUT
    call 5
PRINTBUF2:    
    ld a,(buf+3)
    ld e,a
    ld c,_CONOUT
    call 5
    ld a,(buf+4)
    ld e,a
    ld c,_CONOUT
    call 5
    pop de
    pop hl
    ret

buf: ds 5

    ;http://wikiti.brandonw.net/index.php?title=Z80_Routines:Other:DispHL
BYTE2ASC:
    ld h,0
    ld l,a
HL2ASC:    
    ld ix,buf    
	ld	bc,-10000
	call	Num1
	ld	bc,-1000
	call	Num1
	ld	bc,-100
	call	Num1
	ld	c,-10
	call	Num1
	ld	c,-1
Num1:	ld	a,'0'-1
Num2:	inc	a
	add	hl,bc
	jr	c,Num2
	sbc	hl,bc
	ld (ix),a
    inc ix

	ret 

HEADERS:
    db "Content-Type: text/plain",13,10
    db 13,10
    db "$"