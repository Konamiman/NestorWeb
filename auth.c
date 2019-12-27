#include "types.h"
#include "auth.h"
#include "system.h"
#include <string.h>
#include <stdio.h>
#include "http.h"
#include "buffers.h"
#include "utils.h"
#include "base64.h"

byte auth_user[255+1];
byte auth_password[255+1];
byte auth_realm[255+1];
byte auth_header[256+1];
char current_user[255+1];
char current_password[255+1];

extern applicationState state;
extern const char* empty_str;
extern byte data_buffer[];
extern const char* env_remote_user;
extern const char* env_remote_password;

const char* auth_mode_backup_env_item = "_NHTTP_AUTH_MODE";

char* InitializeAuthentication()
{
    if(state.authenticationMode > AUTH_MODE_STATIC_AND_CGI)
        return "Unknown authentication mode";

    current_user[0] = state.authenticationMode + '0';
    current_user[1] = '\0';
    SetEnvironmentItem(auth_mode_backup_env_item, current_user);

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


void CleanupAuthentication()
{
    DeleteEnvironmentItem(auth_mode_backup_env_item);
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


void InitializeAuthenticationBuffers()
{
    *current_user = '\0';
    *current_password = '\0';
    DeleteEnvironmentItem(env_remote_user);
    DeleteEnvironmentItem(env_remote_password);
}


void SendUnauthorizedError()
{
    SendResponseStart(401, "Unauthorized");
    SendContentLengthHeader(0);
    sprintf(OUTPUT_DATA_BUFFER_START, "WWW-Authenticate: Basic realm=\"%s\"\r\n", auth_realm);
    SendLineToClient(OUTPUT_DATA_BUFFER_START);
    SendLineToClient(empty_str);
}

bool ProcessAuthenticationHeader(bool forCgi)
{
    char* pointer;
    char* pointer_temp;
    int length;
    byte error;
    char ch;

    if(!forCgi && state.authenticationMode == AUTH_MODE_NONE)
        return false;

    if(!StringStartsWith(data_buffer, "Authorization:"))
        return false;

    pointer = &data_buffer[14];
    while(*pointer == ' ') pointer++;

    if(!StringStartsWith(pointer, "Basic "))
        return false;

    pointer+=5;
    while(*pointer == ' ') pointer++;

    Base64Init(0);
    length = Base64DecodeChunk(pointer, auth_header, strlen(pointer), 1, &error);
    if(error)
    {
        SendBadRequestError();
        CloseConnectionToClient();
        return true;
    }

    pointer = auth_header;
    pointer[length] = '\0';

    pointer_temp = current_user;
    while((ch = *pointer) != ':')
    {
        if(ch  == '\0')
        {
            SendBadRequestError();
            CloseConnectionToClient();
            return true;
        }
        *pointer_temp++ = ch;
        *pointer++;
    }
    *pointer_temp = '\0';

    pointer++;  //Skip ':'
    pointer_temp = current_password;
    while((ch = *pointer) != '\0')
    {
        *pointer_temp++ = ch;
        *pointer++;
    }
    *pointer_temp = '\0';

    return true;
}


bool ProcessAuthentication(bool forCgi)
{
    if(state.authenticationMode == AUTH_MODE_NONE)
        return true;

    if(forCgi && state.authenticationMode == AUTH_MODE_STATIC)
        return true;

    if(*current_user == '\0')
    {
        SendUnauthorizedError();
        CloseConnectionToClient();
        return false;
    }

    if((strcmp(current_user, auth_user) == 0) && (strcmp(current_password, auth_password) == 0))
        return true;
    
    SendForbiddenError();
    CloseConnectionToClient();
    return false;
}


void SendForbiddenError()
{
    SendErrorResponseToClient(403, "Forbidden", "Sorry, the provided credentials aren't valid.");
}


void SetAuthRelatedEnvItems()
{
    SetEnvironmentItem(env_remote_user, current_user);

    if(state.authenticationMode != AUTH_MODE_STATIC_AND_CGI)
        SetEnvironmentItem(env_remote_password, current_password);
}
