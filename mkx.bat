sdasz80 -g -o crt0fork.rel crt0msx_msxdos_fork.asm
if errorlevel 1 goto :end

sjasm do.asm do.com
if errorlevel 1 goto :end

sdcc --code-loc 0x430 --data-loc 0 -mz80 --disable-warning 196 --disable-warning 85 --no-std-crt0 putchar_msxdos.rel asm.lib printf_simple.rel crt0fork.rel x.c
if errorlevel 1 goto :end

hex2bin -e com x.ihx
copy x.com f:
copy do.com f:

:end