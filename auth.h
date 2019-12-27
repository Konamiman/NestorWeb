#ifndef __AUTH_H
#define __AUTH_H

#define AUTH_MODE_NONE 0
#define AUTH_MODE_STATIC 1
#define AUTH_MODE_STATIC_AND_CGI 2

char* InitializeAuthentication();
char* AuthModeAsString();

#endif