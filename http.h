#include "types.h"


#define HTTP_DEFAULT_SERVER_PORT 80

#define VERBOSE_MODE_SILENT 0
#define VERBOSE_MODE_CONNECTIONS 1
#define VERBOSE_MODE_ALL 2

#define DEFAULT_INACTIVITY_TIMEOUT_SECS 5
#define DEFAULT_INACTIVITY_TIMEOUT_SECS_STR "5"

#define QUERY_STRING_SEPARATOR '?'

enum HttpAutomatonStates {
    HTTPA_NONE,
    HTTPA_LISTENING,
    HTTPA_READING_REQUEST,
    HTTPA_READING_HEADERS,
    HTTPA_SENDING_RESPONSE
};


void InitializeHttpAutomaton(char* base_directory, char* http_error_buffer, uint port, byte verbose_mode, int inactivity_timeout_in_ticks, bool enable_directory_listing);
void CleanupHttpAutomaton();
void DoHttpServerAutomatonStep();
