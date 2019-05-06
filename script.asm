    org 100h
    ld c,63h    ;DEFAB
    ld de,abort
    call 5
    ld c,9
    ld de,hello
    call 5
    ;ret
    ld sp,8000h
    ld c,62h    ;_TERM
    ld b,7
    jp 5
abort:
    push af
    push bc
    ld e,"*"
    ld c,2
    call 5
    pop bc
    pop af
    ret
hello: db "Hello!!",13,10,"$"