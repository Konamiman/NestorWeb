/*
NestorWeb CGI script that implements a guestbook

- "GET /cgi-bin/guestbk.cgi" will display the form to enter name and message.
- "POST /cgi-bin/guestbk.cgi" will append the message to a file named GUESTBK.TXT.
- "GET /cgi-bin/guestbk.cgi/thanks" will display the thank you message.
- Name and message are mandatory, if any is missing then a "Bad Request" error is returned.

The GUESTBK.TXT file will be created/searched for in the directory specified by the GUESTBK_DIR environment item
(it must end with "\"), or if that item doesn't exist, in the NestorWeb temporary directory.

TODO:
- Sanitize input (at least, verify lengths against maximums allowed)
- Do something after urldecoding the form data (character sets and stuff?)

Compile with:

sdcc -mz80 --disable-warning 196 --disable-warning 85 -o nweb.ihx --code-loc 0x110 --data-loc 0 --no-std-crt0
     crt0msx_msxdos.rel printf_simple.rel putchar_msxdos.rel asm.lib guestbk.c
hex2bin -e com guestbk.c

crt0msx_msxdos.rel, printf_simple.rel, putchar_msxdos.rel, asm.lib/.h
are available at konamiman.com
*/

typedef unsigned char byte;
typedef unsigned int uint;
typedef unsigned char bool;

#include <stdio.h>
#include <string.h>
#include "asm.h"

#define MAX_NAME_LENGTH 40
#define MAX_TEXT_LENGTH 1024

#define BIG_BUFFER ((byte*)0x4000)
#define IP_BUFFER (BIG_BUFFER+2048)
#define DATE_BUFFER (IP_BUFFER+16)
#define NAME_BUFFER (DATE_BUFFER+20)
#define TEXT_BUFFER (NAME_BUFFER+MAX_NAME_LENGTH*3) //3 to account for urlencoidng
#define FILE_CONTENTS_BUFFER (TEXT_BUFFER+MAX_TEXT_LENGTH*3)

#define ToLower(c) ((c) | 32)
#define ToUpper(c) ((c) & ~32)

#define false 0
#define true (!false)

#define STDIN 0
#define STDOUT 1
#define STDERR 2

#define ERR_EOF 0xC7
#define ERR_NOFIL 0xD7

#define MIN_MSXDOS2_FUNCTION 0x40

#define _GDATE 0x2A
#define _GTIME 0x2C
#define _OPEN 0x43
#define _CREATE 0x44
#define _READ 0x48
#define _WRITE 0x49
#define _SEEK 0x4A
#define _TERM 0x62
#define _GENV 0x6B

#define NWEB_ERR_BAD_REQUEST 1
#define NWEB_ERR_NOT_FOUND 2
#define NWEB_ERR_BAD_METHOD 3

char buffer[256];
char buffer2[256];
char* next_extraction_pointer;
char* extracted_key_pointer;
char* extracted_value_pointer;
Z80_registers regs;

void SendForm();
void ProcessFormData();
void TerminateWithMissingDataError();
bool ExtractNextKeyAndValue();
void SendThanks();
bool strcmpi(char* s1, char* s2);
void DoDosCall(byte function);
void Terminate(byte error_code);
char* GetEnvironmentItem(const char* name, char* value);
byte OpenFile(void* path);
byte CreateFile(void* path);
void WriteToFile(byte file_handle, const void* source, int length);
int ReadFromFile(byte file_handle, const void* destination, int length);
void GetDate(char* dest);
void UrlDecode(char *sSource, char *sDest);
void Debug(char* string);

#define WriteStringToStdout(string) { WriteToFile(STDOUT, string, strlen(string)); }


void main()
{
    GetEnvironmentItem("PATH_INFO", buffer);

    if(strcmpi(buffer, ""))
    {
        GetEnvironmentItem("REQUEST_METHOD", buffer);
        if(strcmpi(buffer, "GET"))
            SendForm();
        else if(strcmpi(buffer, "POST"))
            ProcessFormData();
        else
            Terminate(NWEB_ERR_BAD_METHOD);

    }
    else if(strcmpi(buffer, "/thanks"))
    {
        GetEnvironmentItem("REQUEST_METHOD", buffer);
        if(strcmpi(buffer, "GET"))
            SendThanks();
        else
            Terminate(NWEB_ERR_BAD_METHOD);
    }
    else
    {
        Terminate(NWEB_ERR_NOT_FOUND);
    }

    Terminate(0);
}


void SendForm()
{
    WriteStringToStdout(
        "X-CGI-Content-File: guestbk\\form.htm\r\n"
        "Content-Type: text/html\r\n"
        "\r\n"
    );
}


void ProcessFormData()
{
    int length;
    char* name;
    char* text;
    byte file_handle;

    GetDate(DATE_BUFFER);

    //* Verify headers

    GetEnvironmentItem("CONTENT_TYPE", buffer);
    if(!strcmpi(buffer, "application/x-www-form-urlencoded"))
        Terminate(NWEB_ERR_BAD_REQUEST);

    //* Extract form data

    name = text = "";

    length = ReadFromFile(STDIN, BIG_BUFFER, 4096);
    BIG_BUFFER[length] = 0;

    next_extraction_pointer = BIG_BUFFER;
    
    while(ExtractNextKeyAndValue())
    {
        if(strcmpi(extracted_key_pointer, "name"))
            name = extracted_value_pointer;
        else if(strcmpi(extracted_key_pointer, "text"))
            text = extracted_value_pointer;
        else
            Terminate(NWEB_ERR_BAD_REQUEST);
    }

    if(*name == '\0' || *text == '\0')
        TerminateWithMissingDataError();

    UrlDecode(name, NAME_BUFFER);
    UrlDecode(text, TEXT_BUFFER);

    //* Remove leading spaces from data

    name = NAME_BUFFER;
    text = TEXT_BUFFER;
    while(*name == ' ') name++;
    while(*text == ' ') text++;
    if(*name == '\0' || *text == '\0')
        TerminateWithMissingDataError();

    //* Write form data to data file

    GetEnvironmentItem("REMOTE_ADDR", IP_BUFFER);
    GetEnvironmentItem("GUESTBK_DIR", buffer);
    if(*buffer == '\0') GetEnvironmentItem("_NWEB_TEMP", buffer);
    strcat(buffer, "GUESTBK.TXT");
    file_handle = OpenFile(buffer);
    if(!file_handle) file_handle = CreateFile(buffer);
    sprintf(FILE_CONTENTS_BUFFER, "\r\n----------\r\nMessage from %s (%s) at %s\r\n\r\n%s\r\n", name, IP_BUFFER, DATE_BUFFER, text);
    WriteToFile(file_handle, FILE_CONTENTS_BUFFER, strlen(FILE_CONTENTS_BUFFER));

    //* Redirect to "thanks" page

    GetEnvironmentItem("SERVER_NAME", buffer);
    GetEnvironmentItem("SERVER_PORT", &buffer[16]);
    GetEnvironmentItem("SCRIPT_NAME", &buffer[22]);
    sprintf(buffer2, "Location: http://%s:%s%s/thanks\r\nContent-Type: text/plain\r\n\r\n", buffer, &buffer[16], &buffer[22]);
    WriteStringToStdout(buffer2);
}


void TerminateWithMissingDataError()
{
    WriteStringToStdout("X-CGI-Error: 400 Bad Request\r\n\r\nPlease provide both your name and a message.");
    Terminate(0);
}


bool ExtractNextKeyAndValue()
{
    char ch;

    if((ch = *next_extraction_pointer) == '\0')
        return false;

    extracted_key_pointer = next_extraction_pointer;

    do
    {
        ch = *next_extraction_pointer++;
    }
    while(ch != '=' && ch != '\0');

    if(ch == '\0')
        return false;

    next_extraction_pointer[-1] = '\0';
    extracted_value_pointer = next_extraction_pointer;

    while((ch = *next_extraction_pointer++) != '&' && ch != '\0');
    next_extraction_pointer[-1] = '\0';

    return true;
}


void SendThanks()
{
    GetEnvironmentItem("SCRIPT_NAME", buffer);

    sprintf(BIG_BUFFER,
        "<html>"
        "<head>"
        "<title>NestorWeb guestbook example</title>"
        "</head>"
        "<body>"
        "<h1>Thank you!</h1>"
        "<p>Your message has been saved, and some day you might even see it published!</p>"
        "<a href=\"%s\">Post another</a>"
        "</body>"
        "</html>",
        buffer
        );

    WriteStringToStdout("Content-Type: text/html\r\n\r\n");
    WriteStringToStdout(BIG_BUFFER);
}


bool strcmpi(char* s1, char* s2)
{
    char ch1;
    char ch2;
    while((ch1 = ToUpper(*s1++)) == (ch2 = ToUpper(*s2++)) && ch1 != '\0' && ch2 != '\0');
    return ch1 == '\0' && ch2 == '\0';
}


void DoDosCall(byte function)
{
    DosCall(function, &regs, REGS_MAIN, REGS_MAIN);
    if(function >= MIN_MSXDOS2_FUNCTION && regs.Bytes.A != 0 && regs.Bytes.A != ERR_EOF && regs.Bytes.A != ERR_NOFIL)
    {
        Terminate(regs.Bytes.A);
    }
}


void Terminate(byte error_code)
{
    regs.Bytes.B = error_code;
    DosCall(_TERM, &regs, REGS_MAIN, REGS_NONE);
}


char* GetEnvironmentItem(const char* name, char* value)
{
    regs.Words.HL = (int)name;
    regs.Words.DE = (int)value;
    regs.Bytes.B = 255;
    DoDosCall(_GENV);
    return value;
}


byte OpenFile(void* path)
{
    byte file_handle;

    regs.Words.DE = (int)path;
    regs.Bytes.A = 0;
    DoDosCall(_OPEN);
    if(regs.Bytes.A != 0)
        return 0;

    file_handle = regs.Bytes.B;

    regs.Words.DE = 0;
    regs.Words.HL = 0;
    regs.Bytes.A = 2;   //Seek relative to end of file
    DoDosCall(_SEEK);

    return file_handle;
}


byte CreateFile(void* path)
{
    regs.Words.DE = (int)path;
    regs.Bytes.A = 0;
    regs.Bytes.B = 0;
    DoDosCall(_CREATE);
    return regs.Bytes.B;
}


void WriteToFile(byte file_handle, const void* source, int length)
{
    regs.Bytes.B = file_handle;
    regs.Words.DE = (int)source;
    regs.Words.HL = length;
    DoDosCall(_WRITE);
}


int ReadFromFile(byte file_handle, const void* destination, int length)
{
    regs.Bytes.B = file_handle;
    regs.Words.DE = (int)destination;
    regs.Words.HL = length;
    DoDosCall(_READ);
    return regs.Words.HL;
}


void GetDate(char* dest)
{
    byte hour;
    byte min;
    byte sec;

    DoDosCall(_GTIME);
    hour = regs.Bytes.H;
    min = regs.Bytes.L;
    sec = regs.Bytes.E;

    DoDosCall(_GDATE);

    sprintf(dest, "%i-%i-%i %i:%i:%i", regs.Words.HL, regs.Bytes.D, regs.Bytes.E, hour, min, sec);
}


char isxdigit(unsigned char c)
{
    return ( c >= '0' && c <= '9') ||
         ( c >= 'a' && c <= 'f') ||
         ( c >= 'A' && c <= 'F');
}


//https://stackoverflow.com/a/20437049/4574
void UrlDecode(char *sSource, char *sDest) 
{
    int nLength;
    char ch;

    for (nLength = 0; *sSource; nLength++) {
        if ((ch = *sSource) == '+')
        {
            sDest[nLength] = ' ';
            sSource++;
            continue;
        }
        if (ch == '%' && sSource[1] && sSource[2] && isxdigit(sSource[1]) && isxdigit(sSource[2])) {
            sSource[1] -= sSource[1] <= '9' ? '0' : (sSource[1] <= 'F' ? 'A' : 'a')-10;
            sSource[2] -= sSource[2] <= '9' ? '0' : (sSource[2] <= 'F' ? 'A' : 'a')-10;
            sDest[nLength] = 16 * sSource[1] + sSource[2];
            sSource += 3;
            continue;
        }
        sDest[nLength] = ch;
        sSource++;
    }
    sDest[nLength] = '\0';
}


//This prints to the console, use with care!
void Debug(char* string)
{
    WriteToFile(STDERR, "--- GUESTBK: ", 13);
    WriteToFile(STDERR, string, strlen(string));
    WriteToFile(STDERR, "\r\n", 2);
}
