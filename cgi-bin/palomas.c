// This is just a very old and weird joke.
// Please, don't try to understand it.

typedef unsigned char byte;
typedef unsigned int uint;
typedef unsigned char bool;

#include <stdio.h>
#include <string.h>
#include "asm.h"

#define SYSTEM_TIMER (*((byte*)0xFC9E))

#define false 0
#define true (!false)

#define STDOUT 1

#define _WRITE 0x49
#define _TERM 0x62

#define NWEB_ERR_BAD_REQUEST 1
#define NWEB_ERR_NOT_FOUND 2
#define NWEB_ERR_BAD_METHOD 3

#define buffer ((char*)0x4000)
Z80_registers regs;

byte random(byte max);
byte WriteToFile(byte file_handle, const void* source, int length);
#define WriteStringToStdout(string) WriteToFile(STDOUT, string, strlen(string))
void Terminate(byte error_code);

const byte random_sequence[] = {155,193,224,137,132,252,65,45,2,101,131,231,184,170,19,182,118,223,60,14,50,225,89,13,117,204,21,237,175,76,97,66,4,167,226,198,37,211,148,151,11,127,188,124,130,94,20,96,47,215,70,85,33,87,111,250,9,122,163,62,22,160,172,247,196,248,39,25,23,147,54,123,27,243,90,219,229,83,108,0,7,134,72,63,230,239,74,30,46,51,227,80,150,8,43,173,75,177,234,121,140,67,99,106,34,159,24,165,109,158,169,186,10,202,86,32,116,220,64,135,200,119,192,68,205,245,143,206,95,16,168,191,42,209,152,197,210,41,129,113,128,114,3,235,71,189,187,238,254,240,115,82,26,246,253,242,78,105,100,183,59,154,195,207,241,93,81,57,55,110,73,199,218,28,179,255,61,103,162,244,236,142,48,201,44,228,194,91,69,49,5,166,56,84,181,1,164,217,171,203,112,15,35,29,139,125,107,6,161,38,249,153,180,98,222,92,31,157,104,213,221,58,208,216,144,88,12,79,18,176,126,214,174,149,138,233,77,40,120,133,141,52,53,251,190,185,146,212,102,36,156,232,145,136,178,17};

const char* suhetos[] = {"JUANJO","SAVER","DORRA","TOTRENTI","PAYO","ANTON","YAYO","PANPOYA","XISK","TOLO","PAYOFON","MARCELAI","RECUERDA QUE","DIME AL OIDO LO DE","ME PONGO CACHONDA CUANDO ME DICES QUE","NO NECESARIAMENTE","MEJOR","DESDE PEQUENYO","SOLO","CON LAS YAYAS","A ESTAS ALTURAS Y","CON EL CHERTER","DE HECHO","DIOS NO EXISTE PORQUE"};
const char* verbos[] = {"TE VI","FOLLO","CAGAS","ANDUVE","DECAIGO","BENDIGO","ME HAGO PAJAS","APRENDO JAPONATA","QUIERO UNA JAPONESA","ME FALTA TENER","NO TE DAS CUEN","ME CAGO","ME CORRO","SE CAGA LA PERRA","DAME UN BESO","TENGO UN BUCADILL","HE SUSPENDIO","ESTOY","ME CONFORMO","ME FAMILIARIZO","ME REBERBERO","SE JUSTIFICA","ME HAGO CARGO","DE TODA LA VIDA"," ME MALTRATAN"};
const char* cc[] = {"JUGANDO CON LAS PALOMAS","CAGANDO LECHES","LLENDO A BASURALONA","CON EL EME ESE MESEXE","EN LA SECTA","PROGRAMANDO INFINITAMENTE","CON LA PUNTA DEL CAPUYO","CON EL TELETEXTO","EN EL FIN DEL MUNDO","COMO SI FUERA LA PRIMERA VEZ","CADA VEZ MAS","CON MUCHA FRECUENCIA","MIENTRAS ME DAN POR POYA"," SOLO POR JODER"," EN UN PLIS PLAS","COMO A MI ME GUSTA","DE PUTA MADRE","COMO LAS ZORRAS","PORQUE SE ME HABRE LA PUERTA"," CON GANAS DE BOMITAR","COMO SI LO VIVIERA"," MAS QUE NUNCA"," DE AKA PARA YA"};
const char* otras[] = {"CUANDO","SIEMPRE","DE NO SER POR QUE","ULTIMAMENTE","DESDE QUE","YA QUE","LE DJE QUE","CON ESTE PROGRAMA"};
const char* otrasmas[] = {"DEBERIAS","ES QUE EN REALIDAD","TE DIGO QUE","PUES","ME ACUERDO QUE","COMO CUANDO","TIENDO","SOLO PORQUE","YA QUE"};

#define count(a) (sizeof(a)/2)

void main()
{
    byte i;
    byte error;

    *buffer = '\0';

    if(random(9) < 4)
    {
        i = random(count(otras)-1);
        strcat(buffer, otras[i]);
        strcat(buffer, " ");

        i = random(count(verbos)-1);
        strcat(buffer, verbos[i]);
        strcat(buffer, " ");

        i = random(count(cc)-1);
        strcat(buffer, cc[i]);
        strcat(buffer, " ");

        i = random(count(otrasmas)-1);
        strcat(buffer, otrasmas[i]);
        strcat(buffer, " ");

        strcat(buffer, " ");
        i = random(count(verbos)-1);
        strcat(buffer, verbos[i]);
        strcat(buffer, " ");

        i = random(count(cc)-1);
        strcat(buffer, cc[i]);
        strcat(buffer, ".");
    }
    else
    {
        i = random(count(suhetos)-1);
        strcat(buffer, suhetos[i]);
        strcat(buffer, " ");

        i = random(count(verbos)-1);
        strcat(buffer, verbos[i]);
        strcat(buffer, " ");

        i = random(count(cc)-1);
        strcat(buffer, cc[i]);
        strcat(buffer, ".");
    }

    error = WriteStringToStdout("Content-Type: text/plain\r\nCache-Control: no-cache, no-store\r\n\r\n");
    if(error) Terminate(error);

    error = WriteStringToStdout(buffer);
    Terminate(error);
}

byte refresh_register() __naked
{
    __asm
    ld a,r
    ld l,a
    ret
    __endasm;
}

byte random_byte()
{
    return random_sequence[(int)(SYSTEM_TIMER ^ refresh_register())];
}

byte random(byte max)
{
    byte value;

    while(true)
    {
        if((value = random_byte()) <= max) return value;
    }
}

byte WriteToFile(byte file_handle, const void* source, int length)
{
    regs.Bytes.B = file_handle;
    regs.Words.DE = (int)source;
    regs.Words.HL = length;
    DosCall(_WRITE, &regs, REGS_MAIN, REGS_AF);
    return regs.Bytes.A;
}

void Terminate(byte error_code)
{
    regs.Bytes.B = error_code;
    DosCall(_TERM, &regs, REGS_MAIN, REGS_NONE);
}
