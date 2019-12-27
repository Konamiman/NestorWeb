#include "types.h"
#include "auth.h"
#include "system.h"
#include "string.h"

byte auth_user[128+1];
byte auth_password[128+1];
byte auth_realm[128+1];

extern applicationState state;
extern const char* empty_str;

char* InitializeAuthentication()
{
    if(state.authenticationMode > AUTH_MODE_STATIC_AND_CGI)
        return "Unknown authentication mode";

    if(state.authenticationMode == AUTH_MODE_NONE)
        return empty_str;
    
    GetEnvironmentItem("NHTTP_USER", auth_user);
    GetEnvironmentItem("NHTTP_PASSWORD", auth_password);
    if(*auth_user == '\0' || *auth_password == '\0')
        return "NHTTP_USER and NHTTP_PASSWORD environment items are required when using authentication.";

    GetEnvironmentItem("NHTTP_REALM", auth_realm);
    if(*auth_realm == '\0')
        strcpy(auth_realm, "NestorHTTP");

    return empty_str;
}


char* AuthModeAsString()
{
    switch(state.authenticationMode)
    {
        case AUTH_MODE_NONE:
            return "None";
            break;
        case AUTH_MODE_STATIC:
            return "Static content";
            break;
        default:
            return "Static content and CGI";
    }
}