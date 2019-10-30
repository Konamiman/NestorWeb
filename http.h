#include "types.h"


#define HTTP_DEFAULT_SERVER_PORT 80

#define VERBOSE_MODE_SILENT 0
#define VERBOSE_MODE_CONNECTIONS 1
#define VERBOSE_MODE_ALL 2


enum HttpAutomatonStates {
    HTTPA_NONE,
    HTTPA_LISTENING,
    HTTPA_READING_REQUEST,
    HTTPA_READING_HEADERS,
    HTTPA_SENDING_RESPONSE
};


void InitializeHttpAutomaton(char* http_error_buffer, uint port, byte verbose_mode);
void DoHttpServerAutomatonStep();
