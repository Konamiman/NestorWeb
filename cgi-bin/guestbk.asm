;--------------------------------------------------------
; File Created by SDCC : free open source ANSI-C Compiler
; Version 3.6.0 #9615 (MINGW64)
;--------------------------------------------------------
	.module guestbk
	.optsdcc -mz80
	
;--------------------------------------------------------
; Public variables in this module
;--------------------------------------------------------
	.globl _isxdigit
	.globl _main
	.globl _DosCall
	.globl _strlen
	.globl _strcat
	.globl _sprintf
	.globl _regs
	.globl _extracted_value_pointer
	.globl _extracted_key_pointer
	.globl _next_extraction_pointer
	.globl _buffer2
	.globl _buffer
	.globl _SendForm
	.globl _ProcessFormData
	.globl _TerminateWithMissingDataError
	.globl _ExtractNextKeyAndValue
	.globl _SendThanks
	.globl _strcmpi
	.globl _DoDosCall
	.globl _Terminate
	.globl _GetEnvironmentItem
	.globl _OpenFile
	.globl _CreateFile
	.globl _WriteToFile
	.globl _ReadFromFile
	.globl _GetDate
	.globl _UrlDecode
	.globl _Debug
;--------------------------------------------------------
; special function registers
;--------------------------------------------------------
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _DATA
_buffer::
	.ds 256
_buffer2::
	.ds 256
_next_extraction_pointer::
	.ds 2
_extracted_key_pointer::
	.ds 2
_extracted_value_pointer::
	.ds 2
_regs::
	.ds 12
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _INITIALIZED
;--------------------------------------------------------
; absolute external ram data
;--------------------------------------------------------
	.area _DABS (ABS)
;--------------------------------------------------------
; global & static initialisations
;--------------------------------------------------------
	.area _HOME
	.area _GSINIT
	.area _GSFINAL
	.area _GSINIT
;--------------------------------------------------------
; Home
;--------------------------------------------------------
	.area _HOME
	.area _HOME
;--------------------------------------------------------
; code
;--------------------------------------------------------
	.area _CODE
;guestbk.c:100: void main()
;	---------------------------------
; Function main
; ---------------------------------
_main::
;guestbk.c:102: GetEnvironmentItem("PATH_INFO", buffer);
	ld	hl,#_buffer
	push	hl
	ld	hl,#___str_0
	push	hl
	call	_GetEnvironmentItem
	pop	af
;guestbk.c:104: if(strcmpi(buffer, ""))
	ld	hl, #___str_1
	ex	(sp),hl
	ld	hl,#_buffer
	push	hl
	call	_strcmpi
	pop	af
	pop	af
	ld	a,l
	or	a, a
	jr	Z,00114$
;guestbk.c:106: GetEnvironmentItem("REQUEST_METHOD", buffer);
	ld	hl,#_buffer
	push	hl
	ld	hl,#___str_2
	push	hl
	call	_GetEnvironmentItem
	pop	af
;guestbk.c:107: if(strcmpi(buffer, "GET"))
	ld	hl, #___str_3
	ex	(sp),hl
	ld	hl,#_buffer
	push	hl
	call	_strcmpi
	pop	af
	pop	af
	ld	a,l
	or	a, a
	jr	Z,00105$
;guestbk.c:108: SendForm();
	call	_SendForm
	jr	00115$
00105$:
;guestbk.c:109: else if(strcmpi(buffer, "POST"))
	ld	hl,#___str_4
	push	hl
	ld	hl,#_buffer
	push	hl
	call	_strcmpi
	pop	af
	pop	af
	ld	a,l
	or	a, a
	jr	Z,00102$
;guestbk.c:110: ProcessFormData();
	call	_ProcessFormData
	jr	00115$
00102$:
;guestbk.c:112: Terminate(NWEB_ERR_BAD_METHOD);
	ld	a,#0x03
	push	af
	inc	sp
	call	_Terminate
	inc	sp
	jr	00115$
00114$:
;guestbk.c:115: else if(strcmpi(buffer, "/thanks"))
	ld	hl,#___str_5
	push	hl
	ld	hl,#_buffer
	push	hl
	call	_strcmpi
	pop	af
	pop	af
	ld	a,l
	or	a, a
	jr	Z,00111$
;guestbk.c:117: GetEnvironmentItem("REQUEST_METHOD", buffer);
	ld	hl,#_buffer
	push	hl
	ld	hl,#___str_2
	push	hl
	call	_GetEnvironmentItem
	pop	af
;guestbk.c:118: if(strcmpi(buffer, "GET"))
	ld	hl, #___str_3
	ex	(sp),hl
	ld	hl,#_buffer
	push	hl
	call	_strcmpi
	pop	af
	pop	af
	ld	a,l
	or	a, a
	jr	Z,00108$
;guestbk.c:119: SendThanks();
	call	_SendThanks
	jr	00115$
00108$:
;guestbk.c:121: Terminate(NWEB_ERR_BAD_METHOD);
	ld	a,#0x03
	push	af
	inc	sp
	call	_Terminate
	inc	sp
	jr	00115$
00111$:
;guestbk.c:125: Terminate(NWEB_ERR_NOT_FOUND);
	ld	a,#0x02
	push	af
	inc	sp
	call	_Terminate
	inc	sp
00115$:
;guestbk.c:128: Terminate(0);
	xor	a, a
	push	af
	inc	sp
	call	_Terminate
	inc	sp
	ret
___str_0:
	.ascii "PATH_INFO"
	.db 0x00
___str_1:
	.db 0x00
___str_2:
	.ascii "REQUEST_METHOD"
	.db 0x00
___str_3:
	.ascii "GET"
	.db 0x00
___str_4:
	.ascii "POST"
	.db 0x00
___str_5:
	.ascii "/thanks"
	.db 0x00
;guestbk.c:132: void SendForm()
;	---------------------------------
; Function SendForm
; ---------------------------------
_SendForm::
;guestbk.c:134: WriteStringToStdout(
	ld	de,#___str_6+0
	ld	c, e
	ld	b, d
	push	bc
	call	_strlen
	ex	(sp),hl
	push	de
	ld	a,#0x01
	push	af
	inc	sp
	call	_WriteToFile
	pop	af
	pop	af
	inc	sp
	ret
___str_6:
	.ascii "X-CGI-Content-File: guestbk"
	.db 0x5c
	.ascii "form.htm"
	.db 0x0d
	.db 0x0a
	.ascii "Content-Type: text/htm"
	.ascii "l"
	.db 0x0d
	.db 0x0a
	.db 0x0d
	.db 0x0a
	.db 0x00
;guestbk.c:142: void ProcessFormData()
;	---------------------------------
; Function ProcessFormData
; ---------------------------------
_ProcessFormData::
	push	ix
	ld	ix,#0
	add	ix,sp
	ld	hl,#-6
	add	hl,sp
	ld	sp,hl
;guestbk.c:149: GetDate(DATE_BUFFER);
	ld	hl,#0x4810
	push	hl
	call	_GetDate
;guestbk.c:153: GetEnvironmentItem("CONTENT_TYPE", buffer);
	ld	hl, #_buffer
	ex	(sp),hl
	ld	hl,#___str_7
	push	hl
	call	_GetEnvironmentItem
	pop	af
;guestbk.c:154: if(!strcmpi(buffer, "application/x-www-form-urlencoded"))
	ld	hl, #___str_8
	ex	(sp),hl
	ld	hl,#_buffer
	push	hl
	call	_strcmpi
	pop	af
	pop	af
	ld	a,l
	or	a, a
	jr	NZ,00102$
;guestbk.c:155: Terminate(NWEB_ERR_BAD_REQUEST);
	ld	a,#0x01
	push	af
	inc	sp
	call	_Terminate
	inc	sp
00102$:
;guestbk.c:159: name = text = "";
	ld	-4 (ix),#<(___str_9)
	ld	-3 (ix),#>(___str_9)
	ld	de,#___str_9
;guestbk.c:161: length = ReadFromFile(STDIN, BIG_BUFFER, 4096);
	push	de
	ld	hl,#0x1000
	push	hl
	ld	h, #0x40
	push	hl
	xor	a, a
	push	af
	inc	sp
	call	_ReadFromFile
	pop	af
	pop	af
	inc	sp
	pop	de
;guestbk.c:162: BIG_BUFFER[length] = 0;
	ld	bc,#0x4000
	add	hl,bc
	ld	(hl),#0x00
;guestbk.c:164: next_extraction_pointer = BIG_BUFFER;
	ld	hl,#0x4000
	ld	(_next_extraction_pointer),hl
;guestbk.c:166: while(ExtractNextKeyAndValue())
00109$:
	push	de
	call	_ExtractNextKeyAndValue
	pop	de
	ld	a,l
	or	a, a
	jr	Z,00111$
;guestbk.c:168: if(strcmpi(extracted_key_pointer, "name"))
	push	de
	ld	hl,#___str_10
	push	hl
	ld	hl,(_extracted_key_pointer)
	push	hl
	call	_strcmpi
	pop	af
	pop	af
	pop	de
	ld	a,l
	or	a, a
	jr	Z,00107$
;guestbk.c:169: name = extracted_value_pointer;
	ld	de,(_extracted_value_pointer)
	jr	00109$
00107$:
;guestbk.c:170: else if(strcmpi(extracted_key_pointer, "text"))
	push	de
	ld	hl,#___str_11
	push	hl
	ld	hl,(_extracted_key_pointer)
	push	hl
	call	_strcmpi
	pop	af
	pop	af
	pop	de
	ld	a,l
	or	a, a
	jr	Z,00104$
;guestbk.c:171: text = extracted_value_pointer;
	ld	hl,(_extracted_value_pointer)
	ld	-4 (ix),l
	ld	-3 (ix),h
	jr	00109$
00104$:
;guestbk.c:173: Terminate(NWEB_ERR_BAD_REQUEST);
	push	de
	ld	a,#0x01
	push	af
	inc	sp
	call	_Terminate
	inc	sp
	pop	de
	jr	00109$
00111$:
;guestbk.c:176: if(*name == '\0' || *text == '\0')
	ld	a,(de)
	or	a, a
	jr	Z,00112$
	ld	l,-4 (ix)
	ld	h,-3 (ix)
	ld	a,(hl)
	or	a, a
	jr	NZ,00113$
00112$:
;guestbk.c:177: TerminateWithMissingDataError();
	push	de
	call	_TerminateWithMissingDataError
	pop	de
00113$:
;guestbk.c:179: UrlDecode(name, NAME_BUFFER);
	ld	hl,#0x4824
	push	hl
	push	de
	call	_UrlDecode
	pop	af
;guestbk.c:180: UrlDecode(text, TEXT_BUFFER);
	ld	hl, #0x489c
	ex	(sp),hl
	ld	l,-4 (ix)
	ld	h,-3 (ix)
	push	hl
	call	_UrlDecode
	pop	af
	pop	af
;guestbk.c:186: while(*name == ' ') name++;
	ld	bc,#0x4824
00115$:
	ld	a,(bc)
	sub	a, #0x20
	jr	NZ,00143$
	inc	bc
	jr	00115$
;guestbk.c:187: while(*text == ' ') text++;
00143$:
	ld	-4 (ix),c
	ld	-3 (ix),b
	ld	hl,#0x489c
00118$:
	ld	c,(hl)
	ld	a,c
	sub	a, #0x20
	jr	NZ,00144$
	inc	hl
	jr	00118$
00144$:
	ld	-2 (ix),l
	ld	-1 (ix),h
;guestbk.c:188: if(*name == '\0' || *text == '\0')
	ld	l,-4 (ix)
	ld	h,-3 (ix)
	ld	a,(hl)
	or	a, a
	jr	Z,00121$
	ld	a,c
	or	a, a
	jr	NZ,00122$
00121$:
;guestbk.c:189: TerminateWithMissingDataError();
	call	_TerminateWithMissingDataError
00122$:
;guestbk.c:193: GetEnvironmentItem("REMOTE_ADDR", IP_BUFFER);
	ld	hl,#0x4800
	push	hl
	ld	hl,#___str_12
	push	hl
	call	_GetEnvironmentItem
	pop	af
;guestbk.c:194: GetEnvironmentItem("GUESTBK_DIR", buffer);
	ld	hl, #_buffer
	ex	(sp),hl
	ld	hl,#___str_13
	push	hl
	call	_GetEnvironmentItem
	pop	af
	pop	af
;guestbk.c:195: if(*buffer == '\0') GetEnvironmentItem("_NWEB_TEMP", buffer);
	ld	a,(#_buffer + 0)
	ld	-5 (ix), a
	or	a, a
	jr	NZ,00125$
	ld	hl,#_buffer
	push	hl
	ld	hl,#___str_14
	push	hl
	call	_GetEnvironmentItem
	pop	af
	pop	af
00125$:
;guestbk.c:196: strcat(buffer, "GUESTBK.TXT");
	ld	hl,#___str_15
	push	hl
	ld	hl,#_buffer
	push	hl
	call	_strcat
	pop	af
;guestbk.c:197: file_handle = OpenFile(buffer);
	ld	hl, #_buffer
	ex	(sp),hl
	call	_OpenFile
	pop	af
	ld	-6 (ix), l
	ld	-5 (ix), l
;guestbk.c:198: if(!file_handle) file_handle = CreateFile(buffer);
	ld	a,-6 (ix)
	or	a, a
	jr	NZ,00127$
	ld	hl,#_buffer
	push	hl
	call	_CreateFile
	pop	af
	ld	-6 (ix), l
	ld	-5 (ix), l
00127$:
;guestbk.c:199: sprintf(FILE_CONTENTS_BUFFER, "\r\n----------\r\nMessage from %s (%s) at %s\r\n\r\n%s\r\n", name, IP_BUFFER, DATE_BUFFER, text);
	ld	l,-2 (ix)
	ld	h,-1 (ix)
	push	hl
	ld	hl,#0x4810
	push	hl
	ld	l, #0x00
	push	hl
	ld	l,-4 (ix)
	ld	h,-3 (ix)
	push	hl
	ld	hl,#___str_16
	push	hl
	ld	hl,#0x549c
	push	hl
	call	_sprintf
	ld	hl,#12
	add	hl,sp
	ld	sp,hl
;guestbk.c:200: WriteToFile(file_handle, FILE_CONTENTS_BUFFER, strlen(FILE_CONTENTS_BUFFER));
	ld	hl,#0x549c
	push	hl
	call	_strlen
	ex	(sp),hl
	ld	hl,#0x549c
	push	hl
	ld	a,-5 (ix)
	push	af
	inc	sp
	call	_WriteToFile
	pop	af
;guestbk.c:204: GetEnvironmentItem("SERVER_NAME", buffer);
	inc	sp
	ld	hl,#_buffer
	ex	(sp),hl
	ld	hl,#___str_17
	push	hl
	call	_GetEnvironmentItem
	pop	af
;guestbk.c:205: GetEnvironmentItem("SERVER_PORT", &buffer[16]);
	ld	hl, #(_buffer + 0x0010)
	ex	(sp),hl
	ld	hl,#___str_18
	push	hl
	call	_GetEnvironmentItem
	pop	af
;guestbk.c:206: GetEnvironmentItem("SCRIPT_NAME", &buffer[22]);
	ld	hl, #(_buffer + 0x0016)
	ex	(sp),hl
	ld	hl,#___str_19
	push	hl
	call	_GetEnvironmentItem
	pop	af
;guestbk.c:207: sprintf(buffer2, "Location: http://%s:%s%s/thanks\r\nContent-Type: text/plain\r\n\r\n", buffer, &buffer[16], &buffer[22]);
	ld	hl, #(_buffer + 0x0016)
	ex	(sp),hl
	ld	hl,#(_buffer + 0x0010)
	push	hl
	ld	hl,#_buffer
	push	hl
	ld	hl,#___str_20
	push	hl
	ld	hl,#_buffer2
	push	hl
	call	_sprintf
	ld	hl,#10
	add	hl,sp
	ld	sp,hl
;guestbk.c:208: WriteStringToStdout(buffer2);
	ld	hl,#_buffer2
	push	hl
	call	_strlen
	ex	(sp),hl
	ld	hl,#_buffer2
	push	hl
	ld	a,#0x01
	push	af
	inc	sp
	call	_WriteToFile
	ld	sp,ix
	pop	ix
	ret
___str_7:
	.ascii "CONTENT_TYPE"
	.db 0x00
___str_8:
	.ascii "application/x-www-form-urlencoded"
	.db 0x00
___str_9:
	.db 0x00
___str_10:
	.ascii "name"
	.db 0x00
___str_11:
	.ascii "text"
	.db 0x00
___str_12:
	.ascii "REMOTE_ADDR"
	.db 0x00
___str_13:
	.ascii "GUESTBK_DIR"
	.db 0x00
___str_14:
	.ascii "_NWEB_TEMP"
	.db 0x00
___str_15:
	.ascii "GUESTBK.TXT"
	.db 0x00
___str_16:
	.db 0x0d
	.db 0x0a
	.ascii "----------"
	.db 0x0d
	.db 0x0a
	.ascii "Message from %s (%s) at %s"
	.db 0x0d
	.db 0x0a
	.db 0x0d
	.db 0x0a
	.ascii "%s"
	.db 0x0d
	.db 0x0a
	.db 0x00
___str_17:
	.ascii "SERVER_NAME"
	.db 0x00
___str_18:
	.ascii "SERVER_PORT"
	.db 0x00
___str_19:
	.ascii "SCRIPT_NAME"
	.db 0x00
___str_20:
	.ascii "Location: http://%s:%s%s/thanks"
	.db 0x0d
	.db 0x0a
	.ascii "Content-Type: text/plain"
	.db 0x0d
	.db 0x0a
	.db 0x0d
	.db 0x0a
	.db 0x00
;guestbk.c:212: void TerminateWithMissingDataError()
;	---------------------------------
; Function TerminateWithMissingDataError
; ---------------------------------
_TerminateWithMissingDataError::
;guestbk.c:214: WriteStringToStdout("X-CGI-Error: 400 Bad Request\r\n\r\nPlease provide both your name and a message.");
	ld	de,#___str_21+0
	ld	c, e
	ld	b, d
	push	bc
	call	_strlen
	ex	(sp),hl
	push	de
	ld	a,#0x01
	push	af
	inc	sp
	call	_WriteToFile
	pop	af
	pop	af
	inc	sp
;guestbk.c:215: Terminate(0);
	xor	a, a
	push	af
	inc	sp
	call	_Terminate
	inc	sp
	ret
___str_21:
	.ascii "X-CGI-Error: 400 Bad Request"
	.db 0x0d
	.db 0x0a
	.db 0x0d
	.db 0x0a
	.ascii "Please provide both your nam"
	.ascii "e and a message."
	.db 0x00
;guestbk.c:219: bool ExtractNextKeyAndValue()
;	---------------------------------
; Function ExtractNextKeyAndValue
; ---------------------------------
_ExtractNextKeyAndValue::
;guestbk.c:223: if((ch = *next_extraction_pointer) == '\0')
	ld	hl,(_next_extraction_pointer)
	ld	a,(hl)
;guestbk.c:224: return false;
	or	a,a
	jr	NZ,00102$
	ld	l,a
	ret
00102$:
;guestbk.c:226: extracted_key_pointer = next_extraction_pointer;
	ld	(_extracted_key_pointer),hl
;guestbk.c:228: do
00104$:
;guestbk.c:230: ch = *next_extraction_pointer++;
	ld	hl,(_next_extraction_pointer)
	ld	c,(hl)
	ld	iy,#_next_extraction_pointer
	inc	0 (iy)
	jr	NZ,00139$
	inc	1 (iy)
00139$:
;guestbk.c:232: while(ch != '=' && ch != '\0');
	ld	a,c
	cp	a,#0x3d
	jr	Z,00106$
	or	a, a
	jr	NZ,00104$
00106$:
;guestbk.c:234: if(ch == '\0')
	ld	a,c
;guestbk.c:235: return false;
	or	a,a
	jr	NZ,00108$
	ld	l,a
	ret
00108$:
;guestbk.c:237: next_extraction_pointer[-1] = '\0';
	ld	iy,#0xffff
	ld	de,(_next_extraction_pointer)
	add	iy, de
	ld	0 (iy), #0x00
;guestbk.c:238: extracted_value_pointer = next_extraction_pointer;
	ld	hl,(_next_extraction_pointer)
	ld	(_extracted_value_pointer),hl
;guestbk.c:240: while((ch = *next_extraction_pointer++) != '&' && ch != '\0');
00110$:
	ld	hl,(_next_extraction_pointer)
	ld	b,(hl)
	ld	iy,#_next_extraction_pointer
	inc	0 (iy)
	jr	NZ,00141$
	inc	1 (iy)
00141$:
	ld	a,b
	ld	c,a
	sub	a, #0x26
	jr	Z,00112$
	ld	a,c
	or	a, a
	jr	NZ,00110$
00112$:
;guestbk.c:241: next_extraction_pointer[-1] = '\0';
	ld	iy,#0xffff
	ld	de,(_next_extraction_pointer)
	add	iy, de
	ld	0 (iy), #0x00
;guestbk.c:243: return true;
	ld	l,#0x01
	ret
;guestbk.c:247: void SendThanks()
;	---------------------------------
; Function SendThanks
; ---------------------------------
_SendThanks::
;guestbk.c:249: GetEnvironmentItem("SCRIPT_NAME", buffer);
	ld	hl,#_buffer
	push	hl
	ld	hl,#___str_22
	push	hl
	call	_GetEnvironmentItem
	pop	af
;guestbk.c:262: buffer
;guestbk.c:261: "</html>",
;guestbk.c:251: sprintf(BIG_BUFFER,
	ld	hl, #_buffer
	ex	(sp),hl
	ld	hl,#___str_23
	push	hl
	ld	hl,#0x4000
	push	hl
	call	_sprintf
	ld	hl,#6
	add	hl,sp
	ld	sp,hl
;guestbk.c:265: WriteStringToStdout("Content-Type: text/html\r\n\r\n");
	ld	de,#___str_24+0
	ld	c, e
	ld	b, d
	push	bc
	call	_strlen
	ex	(sp),hl
	push	de
	ld	a,#0x01
	push	af
	inc	sp
	call	_WriteToFile
	pop	af
;guestbk.c:266: WriteStringToStdout(BIG_BUFFER);
	inc	sp
	ld	hl,#0x4000
	ex	(sp),hl
	call	_strlen
	ex	(sp),hl
	ld	hl,#0x4000
	push	hl
	ld	a,#0x01
	push	af
	inc	sp
	call	_WriteToFile
	pop	af
	pop	af
	inc	sp
	ret
___str_22:
	.ascii "SCRIPT_NAME"
	.db 0x00
___str_23:
	.ascii "<html><head><title>NestorWeb guestbook example</title></head"
	.ascii "><body><h1>Thank you!</h1><p>Your message has been saved, an"
	.ascii "d some day you might even see it published!</p><a href="
	.db 0x22
	.ascii "%s"
	.db 0x22
	.ascii ">"
	.ascii "Post another</a></body></html>"
	.db 0x00
___str_24:
	.ascii "Content-Type: text/html"
	.db 0x0d
	.db 0x0a
	.db 0x0d
	.db 0x0a
	.db 0x00
;guestbk.c:270: bool strcmpi(char* s1, char* s2)
;	---------------------------------
; Function strcmpi
; ---------------------------------
_strcmpi::
	push	ix
	ld	ix,#0
	add	ix,sp
	push	af
;guestbk.c:274: while((ch1 = ToUpper(*s1++)) == (ch2 = ToUpper(*s2++)) && ch1 != '\0' && ch2 != '\0');
	ld	c,4 (ix)
	ld	b,5 (ix)
	ld	e,6 (ix)
	ld	d,7 (ix)
	push	de
	pop	iy
00103$:
	ld	a,(bc)
	inc	bc
	ld	d,a
	res	5, d
	ld	-2 (ix),d
	ld	e, 0 (iy)
	inc	iy
	res	5, e
	ld	-1 (ix),e
	ld	a,d
	sub	a, e
	jr	NZ,00105$
	ld	a,-2 (ix)
	or	a, a
	jr	Z,00105$
	ld	a,-1 (ix)
	or	a, a
	jr	NZ,00103$
00105$:
;guestbk.c:275: return ch1 == '\0' && ch2 == '\0';
	ld	a,-2 (ix)
	or	a, a
	jr	NZ,00108$
	ld	a,-1 (ix)
	or	a, a
	jr	Z,00109$
00108$:
	ld	l,#0x00
	jr	00110$
00109$:
	ld	l,#0x01
00110$:
	ld	sp, ix
	pop	ix
	ret
;guestbk.c:279: void DoDosCall(byte function)
;	---------------------------------
; Function DoDosCall
; ---------------------------------
_DoDosCall::
;guestbk.c:281: DosCall(function, &regs, REGS_MAIN, REGS_MAIN);
	ld	hl,#0x0202
	push	hl
	ld	hl,#_regs
	push	hl
	ld	hl, #6+0
	add	hl, sp
	ld	a, (hl)
	push	af
	inc	sp
	call	_DosCall
	pop	af
	pop	af
	inc	sp
;guestbk.c:282: if(function >= MIN_MSXDOS2_FUNCTION && regs.Bytes.A != 0 && regs.Bytes.A != ERR_EOF && regs.Bytes.A != ERR_NOFIL)
	ld	hl, #2+0
	add	hl, sp
	ld	a, (hl)
	sub	a, #0x40
	ret	C
	ld	hl,#_regs + 1
	ld	a,(hl)
	or	a, a
	ret	Z
	ld	a,(hl)
	sub	a, #0xc7
	ret	Z
	ld	a,(hl)
	sub	a, #0xd7
	ret	Z
;guestbk.c:284: Terminate(regs.Bytes.A);
	ld	b,(hl)
	push	bc
	inc	sp
	call	_Terminate
	inc	sp
	ret
;guestbk.c:289: void Terminate(byte error_code)
;	---------------------------------
; Function Terminate
; ---------------------------------
_Terminate::
;guestbk.c:291: regs.Bytes.B = error_code;
	ld	hl,#(_regs + 0x0003)
	ld	iy,#2
	add	iy,sp
	ld	a,0 (iy)
	ld	(hl),a
;guestbk.c:292: DosCall(_TERM, &regs, REGS_MAIN, REGS_NONE);
	ld	hl,#0x0002
	push	hl
	ld	hl,#_regs
	push	hl
	ld	a,#0x62
	push	af
	inc	sp
	call	_DosCall
	pop	af
	pop	af
	inc	sp
	ret
;guestbk.c:296: char* GetEnvironmentItem(const char* name, char* value)
;	---------------------------------
; Function GetEnvironmentItem
; ---------------------------------
_GetEnvironmentItem::
	push	ix
	ld	ix,#0
	add	ix,sp
;guestbk.c:298: regs.Words.HL = (int)name;
	ld	c,4 (ix)
	ld	b,5 (ix)
	ld	((_regs + 0x0006)), bc
;guestbk.c:299: regs.Words.DE = (int)value;
	ld	c,6 (ix)
	ld	b,7 (ix)
	ld	((_regs + 0x0004)), bc
;guestbk.c:300: regs.Bytes.B = 255;
	ld	hl,#(_regs + 0x0003)
	ld	(hl),#0xff
;guestbk.c:301: DoDosCall(_GENV);
	ld	a,#0x6b
	push	af
	inc	sp
	call	_DoDosCall
	inc	sp
;guestbk.c:302: return value;
	ld	l,6 (ix)
	ld	h,7 (ix)
	pop	ix
	ret
;guestbk.c:306: byte OpenFile(void* path)
;	---------------------------------
; Function OpenFile
; ---------------------------------
_OpenFile::
;guestbk.c:310: regs.Words.DE = (int)path;
	pop	de
	pop	bc
	push	bc
	push	de
	ld	((_regs + 0x0004)), bc
;guestbk.c:311: regs.Bytes.A = 0;
	ld	bc,#_regs + 1
	xor	a, a
	ld	(bc),a
;guestbk.c:312: DoDosCall(_OPEN);
	push	bc
	ld	a,#0x43
	push	af
	inc	sp
	call	_DoDosCall
	inc	sp
	pop	bc
;guestbk.c:313: if(regs.Bytes.A != 0)
	ld	a,(bc)
	or	a, a
	jr	Z,00102$
;guestbk.c:314: return 0;
	ld	l,#0x00
	ret
00102$:
;guestbk.c:316: file_handle = regs.Bytes.B;
	ld	hl, #_regs + 3
	ld	c,(hl)
;guestbk.c:318: regs.Words.DE = 0;
	ld	hl,#0x0000
	ld	((_regs + 0x0004)), hl
;guestbk.c:319: regs.Words.HL = 0;
	ld	l, #0x00
	ld	((_regs + 0x0006)), hl
;guestbk.c:320: regs.Bytes.A = 2;   //Seek relative to end of file
	ld	hl,#(_regs + 0x0001)
	ld	(hl),#0x02
;guestbk.c:321: DoDosCall(_SEEK);
	push	bc
	ld	a,#0x4a
	push	af
	inc	sp
	call	_DoDosCall
	inc	sp
	pop	bc
;guestbk.c:323: return file_handle;
	ld	l,c
	ret
;guestbk.c:327: byte CreateFile(void* path)
;	---------------------------------
; Function CreateFile
; ---------------------------------
_CreateFile::
;guestbk.c:329: regs.Words.DE = (int)path;
	pop	de
	pop	bc
	push	bc
	push	de
	ld	((_regs + 0x0004)), bc
;guestbk.c:330: regs.Bytes.A = 0;
	ld	hl,#(_regs + 0x0001)
	ld	(hl),#0x00
;guestbk.c:331: regs.Bytes.B = 0;
	ld	hl,#_regs + 3
	ld	(hl),#0x00
;guestbk.c:332: DoDosCall(_CREATE);
	push	hl
	ld	a,#0x44
	push	af
	inc	sp
	call	_DoDosCall
	inc	sp
	pop	hl
;guestbk.c:333: return regs.Bytes.B;
	ld	l,(hl)
	ret
;guestbk.c:337: void WriteToFile(byte file_handle, const void* source, int length)
;	---------------------------------
; Function WriteToFile
; ---------------------------------
_WriteToFile::
	push	ix
	ld	ix,#0
	add	ix,sp
;guestbk.c:339: regs.Bytes.B = file_handle;
	ld	hl,#(_regs + 0x0003)
	ld	a,4 (ix)
	ld	(hl),a
;guestbk.c:340: regs.Words.DE = (int)source;
	ld	c,5 (ix)
	ld	b,6 (ix)
	ld	((_regs + 0x0004)), bc
;guestbk.c:341: regs.Words.HL = length;
	ld	hl,#(_regs + 0x0006)
	ld	a,7 (ix)
	ld	(hl),a
	inc	hl
	ld	a,8 (ix)
	ld	(hl),a
;guestbk.c:342: DoDosCall(_WRITE);
	ld	a,#0x49
	push	af
	inc	sp
	call	_DoDosCall
	inc	sp
	pop	ix
	ret
;guestbk.c:346: int ReadFromFile(byte file_handle, const void* destination, int length)
;	---------------------------------
; Function ReadFromFile
; ---------------------------------
_ReadFromFile::
	push	ix
	ld	ix,#0
	add	ix,sp
;guestbk.c:348: regs.Bytes.B = file_handle;
	ld	hl,#(_regs + 0x0003)
	ld	a,4 (ix)
	ld	(hl),a
;guestbk.c:349: regs.Words.DE = (int)destination;
	ld	c,5 (ix)
	ld	b,6 (ix)
	ld	((_regs + 0x0004)), bc
;guestbk.c:350: regs.Words.HL = length;
	ld	hl,#(_regs + 0x0006)
	ld	a,7 (ix)
	ld	(hl),a
	inc	hl
	ld	a,8 (ix)
	ld	(hl),a
;guestbk.c:351: DoDosCall(_READ);
	ld	a,#0x48
	push	af
	inc	sp
	call	_DoDosCall
	inc	sp
;guestbk.c:352: return regs.Words.HL;
	ld	hl, (#(_regs + 0x0006) + 0)
	pop	ix
	ret
;guestbk.c:356: void GetDate(char* dest)
;	---------------------------------
; Function GetDate
; ---------------------------------
_GetDate::
	push	ix
	ld	ix,#0
	add	ix,sp
	ld	hl,#-8
	add	hl,sp
	ld	sp,hl
;guestbk.c:362: DoDosCall(_GTIME);
	ld	a,#0x2c
	push	af
	inc	sp
	call	_DoDosCall
	inc	sp
;guestbk.c:363: hour = regs.Bytes.H;
	ld	hl, #_regs + 7
	ld	c,(hl)
;guestbk.c:364: min = regs.Bytes.L;
	ld	hl, #_regs + 6
	ld	b,(hl)
;guestbk.c:365: sec = regs.Bytes.E;
	ld	hl,#_regs + 4
	ld	e,(hl)
;guestbk.c:367: DoDosCall(_GDATE);
	push	hl
	push	bc
	push	de
	ld	a,#0x2a
	push	af
	inc	sp
	call	_DoDosCall
	inc	sp
	pop	de
	pop	bc
	pop	hl
;guestbk.c:369: sprintf(dest, "%i-%i-%i %i:%i:%i", regs.Words.HL, regs.Bytes.D, regs.Bytes.E, hour, min, sec);
	ld	d,#0x00
	ld	-4 (ix),b
	ld	-3 (ix),#0x00
	ld	-6 (ix),c
	ld	-5 (ix),#0x00
	ld	c,(hl)
	ld	-2 (ix),c
	ld	-1 (ix),#0x00
	ld	a, (#_regs + 5)
	ld	-8 (ix),a
	ld	-7 (ix),#0x00
	ld	hl, (#(_regs + 0x0006) + 0)
	ld	bc,#___str_25+0
	push	de
	ld	e,-4 (ix)
	ld	d,-3 (ix)
	push	de
	ld	e,-6 (ix)
	ld	d,-5 (ix)
	push	de
	ld	e,-2 (ix)
	ld	d,-1 (ix)
	push	de
	ld	e,-8 (ix)
	ld	d,-7 (ix)
	push	de
	push	hl
	push	bc
	ld	l,4 (ix)
	ld	h,5 (ix)
	push	hl
	call	_sprintf
	ld	hl,#16
	add	hl,sp
	ld	sp,hl
	ld	sp, ix
	pop	ix
	ret
___str_25:
	.ascii "%i-%i-%i %i:%i:%i"
	.db 0x00
;guestbk.c:373: char isxdigit(unsigned char c)
;	---------------------------------
; Function isxdigit
; ---------------------------------
_isxdigit::
	push	ix
	ld	ix,#0
	add	ix,sp
	dec	sp
;guestbk.c:375: return ( c >= '0' && c <= '9') ||
	ld	a,4 (ix)
	sub	a, #0x30
	jr	C,00111$
	ld	a,#0x39
	sub	a, 4 (ix)
	jr	NC,00104$
00111$:
;guestbk.c:376: ( c >= 'a' && c <= 'f') ||
	ld	a,4 (ix)
	sub	a, #0x61
	jr	C,00108$
	ld	a,#0x66
	sub	a, 4 (ix)
	jr	NC,00104$
00108$:
;guestbk.c:377: ( c >= 'A' && c <= 'F');
	ld	a,4 (ix)
	sub	a, #0x41
	jr	C,00103$
	ld	a,#0x46
	sub	a, 4 (ix)
	jr	NC,00104$
00103$:
	ld	l,#0x00
	jr	00105$
00104$:
	ld	l,#0x01
00105$:
	inc	sp
	pop	ix
	ret
;guestbk.c:382: void UrlDecode(char *sSource, char *sDest) 
;	---------------------------------
; Function UrlDecode
; ---------------------------------
_UrlDecode::
	push	ix
	ld	ix,#0
	add	ix,sp
	ld	hl,#-14
	add	hl,sp
	ld	sp,hl
;guestbk.c:387: for (nLength = 0; *sSource; nLength++) {
	ld	hl,#0x0000
	ex	(sp), hl
00112$:
	ld	a,4 (ix)
	ld	-12 (ix),a
	ld	a,5 (ix)
	ld	-11 (ix),a
	ld	l,-12 (ix)
	ld	h,-11 (ix)
	ld	e,(hl)
;guestbk.c:390: sDest[nLength] = ' ';
	ld	a,6 (ix)
	add	a, -14 (ix)
	ld	-7 (ix),a
	ld	a,7 (ix)
	adc	a, -13 (ix)
	ld	-6 (ix),a
;guestbk.c:387: for (nLength = 0; *sSource; nLength++) {
	ld	a,e
	or	a, a
	jp	Z,00110$
;guestbk.c:388: if ((ch = *sSource) == '+')
	ld	-3 (ix),e
;guestbk.c:391: sSource++;
	ld	a,-12 (ix)
	add	a, #0x01
	ld	-10 (ix),a
	ld	a,-11 (ix)
	adc	a, #0x00
	ld	-9 (ix),a
;guestbk.c:388: if ((ch = *sSource) == '+')
	ld	a,e
	sub	a, #0x2b
	jr	NZ,00102$
;guestbk.c:390: sDest[nLength] = ' ';
	ld	l,-7 (ix)
	ld	h,-6 (ix)
	ld	(hl),#0x20
;guestbk.c:391: sSource++;
	ld	a,-10 (ix)
	ld	4 (ix),a
	ld	a,-9 (ix)
	ld	5 (ix),a
;guestbk.c:392: continue;
	jp	00109$
00102$:
;guestbk.c:397: sDest[nLength] = 16 * sSource[1] + sSource[2];
	ld	a,4 (ix)
	add	a, #0x01
	ld	-5 (ix),a
	ld	a,5 (ix)
	adc	a, #0x00
	ld	-4 (ix),a
;guestbk.c:394: if (ch == '%' && sSource[1] && sSource[2] && isxdigit(sSource[1]) && isxdigit(sSource[2])) {
	ld	a,-3 (ix)
	sub	a, #0x25
	jp	NZ,00104$
	ld	l,-10 (ix)
	ld	h,-9 (ix)
	ld	b,(hl)
	ld	a,b
	or	a, a
	jp	Z,00104$
	ld	l,-12 (ix)
	ld	h,-11 (ix)
	inc	hl
	inc	hl
	ld	a,(hl)
	or	a, a
	jp	Z,00104$
	push	hl
	push	bc
	inc	sp
	call	_isxdigit
	inc	sp
	ld	a,l
	pop	hl
	or	a, a
	jp	Z,00104$
	ld	b,(hl)
	push	bc
	inc	sp
	call	_isxdigit
	inc	sp
	ld	a,l
	or	a, a
	jp	Z,00104$
;guestbk.c:395: sSource[1] -= sSource[1] <= '9' ? '0' : (sSource[1] <= 'F' ? 'A' : 'a')-10;
	ld	a,-10 (ix)
	ld	-12 (ix),a
	ld	a,-9 (ix)
	ld	-11 (ix),a
	ld	l,-10 (ix)
	ld	h,-9 (ix)
	ld	a,(hl)
	ld	-10 (ix),a
	ld	a,#0x39
	sub	a, -10 (ix)
	jr	C,00115$
	ld	bc,#0x0030
	jr	00116$
00115$:
	ld	a,#0x46
	sub	a, -10 (ix)
	jr	C,00117$
	ld	-2 (ix),#0x41
	ld	-1 (ix),#0x00
	jr	00118$
00117$:
	ld	-2 (ix),#0x61
	ld	-1 (ix),#0x00
00118$:
	ld	a,-2 (ix)
	add	a,#0xf6
	ld	c,a
	rla
	sbc	a, a
00116$:
	ld	a, -10 (ix)
	sub	a, c
	ld	l,-12 (ix)
	ld	h,-11 (ix)
	ld	(hl),a
;guestbk.c:396: sSource[2] -= sSource[2] <= '9' ? '0' : (sSource[2] <= 'F' ? 'A' : 'a')-10;
	ld	c,4 (ix)
	ld	b,5 (ix)
	inc	bc
	inc	bc
	ld	a,(bc)
	ld	e,a
	ld	a,#0x39
	sub	a, e
	jr	C,00119$
	ld	hl,#0x0030
	jr	00120$
00119$:
	ld	a,#0x46
	sub	a, e
	jr	C,00121$
	ld	hl,#0x0041
	jr	00122$
00121$:
	ld	hl,#0x0061
00122$:
	ld	a,l
	add	a,#0xf6
	ld	l,a
	rla
	sbc	a, a
00120$:
	ld	a,e
	sub	a, l
	ld	(bc),a
;guestbk.c:397: sDest[nLength] = 16 * sSource[1] + sSource[2];
	ld	l,-5 (ix)
	ld	h,-4 (ix)
	ld	a,(hl)
	rlca
	rlca
	rlca
	rlca
	and	a,#0xf0
	ld	e,a
	ld	a,(bc)
	ld	c, a
	add	a,e
	ld	l,-7 (ix)
	ld	h,-6 (ix)
	ld	(hl),a
;guestbk.c:398: sSource += 3;
	ld	a,4 (ix)
	add	a, #0x03
	ld	4 (ix),a
	ld	a,5 (ix)
	adc	a, #0x00
	ld	5 (ix),a
;guestbk.c:399: continue;
	jr	00109$
00104$:
;guestbk.c:401: sDest[nLength] = ch;
	ld	l,-7 (ix)
	ld	h,-6 (ix)
	ld	a,-3 (ix)
	ld	(hl),a
;guestbk.c:402: sSource++;
	ld	a,-5 (ix)
	ld	4 (ix),a
	ld	a,-4 (ix)
	ld	5 (ix),a
00109$:
;guestbk.c:387: for (nLength = 0; *sSource; nLength++) {
	inc	-14 (ix)
	jp	NZ,00112$
	inc	-13 (ix)
	jp	00112$
00110$:
;guestbk.c:404: sDest[nLength] = '\0';
	ld	l,-7 (ix)
	ld	h,-6 (ix)
	ld	(hl),#0x00
	ld	sp, ix
	pop	ix
	ret
;guestbk.c:409: void Debug(char* string)
;	---------------------------------
; Function Debug
; ---------------------------------
_Debug::
;guestbk.c:411: WriteToFile(STDERR, "--- GUESTBK: ", 13);
	ld	hl,#0x000d
	push	hl
	ld	hl,#___str_26
	push	hl
	ld	a,#0x02
	push	af
	inc	sp
	call	_WriteToFile
	pop	af
	pop	af
	inc	sp
;guestbk.c:412: WriteToFile(STDERR, string, strlen(string));
	pop	bc
	pop	hl
	push	hl
	push	bc
	push	hl
	call	_strlen
	pop	af
	pop	de
	pop	bc
	push	bc
	push	de
	push	hl
	push	bc
	ld	a,#0x02
	push	af
	inc	sp
	call	_WriteToFile
	pop	af
;guestbk.c:413: WriteToFile(STDERR, "\r\n", 2);
	inc	sp
	ld	hl,#0x0002
	ex	(sp),hl
	ld	hl,#___str_27
	push	hl
	ld	a,#0x02
	push	af
	inc	sp
	call	_WriteToFile
	pop	af
	pop	af
	inc	sp
	ret
___str_26:
	.ascii "--- GUESTBK: "
	.db 0x00
___str_27:
	.db 0x0d
	.db 0x0a
	.db 0x00
	.area _CODE
	.area _INITIALIZER
	.area _CABS (ABS)
