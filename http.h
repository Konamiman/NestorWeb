#include "types.h"
#include "system.h"


#define HTTP_DEFAULT_SERVER_PORT 80

#define VERBOSE_MODE_SILENT 0
#define VERBOSE_MODE_CONNECTIONS 1
#define VERBOSE_MODE_ALL 2

#define DEFAULT_INACTIVITY_TIMEOUT_SECS 5
#define DEFAULT_INACTIVITY_TIMEOUT_SECS_STR "5"

#define QUERY_STRING_SEPARATOR '?'

#define TCP_UNAPI_OUTPUT_BUFFER_SIZE 512

enum HttpAutomatonStates {
    HTTPA_NONE,
    HTTPA_LISTENING,
    HTTPA_READING_REQUEST,
    HTTPA_READING_HEADERS,
    HTTPA_SENDING_FILE_CONTENTS,
    HTTPA_SENDING_DIRECTORY_LISTING_HEADER_1,
    HTTPA_SENDING_DIRECTORY_LISTING_HEADER_2,
    HTTPA_SENDING_DIRECTORY_LISTING_ENTRIES,
    HTTPA_SENDING_DIRECTORY_LISTING_FOOTER,
    HTTPA_SENDING_CGI_RESULT_HEADERS
};


void InitializeHttpAutomaton();
void ReinitializeHttpAutomaton();
void CleanupHttpAutomaton();

void DoHttpServerAutomatonStep();
void SendBadRequestError();
void SendNotFoundError();
void SendMethodNotAllowedError(bool fromCgi);
void SendInternalError();
void ProcessFileOrDirectoryRequest();
void CloseConnectionToClient();
void SendResponseStart(int statusCode, char* statusMessage);
void SendErrorResponseToClient(int statusCode, char* statusMessage, char* detailedMessage);
bool SendLineToClient(char* line);
bool SendCrlfTerminatedLineToClient(char* line);
void SendContentLengthHeader(ulong length);
bool ProcessRequestedResource(bool is_local_redirect);
void ProcessFileOrDirectoryRequest();
