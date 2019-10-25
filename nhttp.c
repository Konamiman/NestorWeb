#include <stdio.h>

#define VERSION "0.1" 

const char* strTitle = 
    "NestorHTTP " VERSION "\r\n"
    "(c) 2020 by Konamiman\r\n"
    "\r\n";

const char* strHelp =
    "This is the program help!";

int main(char** argv, int argc)
{
    printf(strTitle);
    if(argc == 0)
    {
        printf(strHelp);
        return 0;
    }

    printf("The program itself!");
    return 0;
}
