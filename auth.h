#ifndef __AUTH_H
#define __AUTH_H

#define AUTH_MODE_NONE 0
#define AUTH_MODE_STATIC 1
#define AUTH_MODE_STATIC_AND_CGI 2

char* InitializeAuthentication();
void CleanupAuthentication();
char* AuthModeAsString();
void SendUnauthorizedError();
void InitializeAuthenticationBuffers();
bool ProcessAuthenticationHeader(bool forCgi);
bool Authenticate(bool forCgi);
void SendForbiddenError();
void SetAuthRelatedEnvItems();

#endif