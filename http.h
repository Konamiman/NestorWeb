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
    HTTPA_SENDING_FILE_CONTENTS,
    HTTPA_SENDING_DIRECTORY_LISTING_HEADER_1,
    HTTPA_SENDING_DIRECTORY_LISTING_HEADER_2,
    HTTPA_SENDING_DIRECTORY_LISTING_ENTRIES,
    HTTPA_SENDING_DIRECTORY_LISTING_FOOTER
};


void InitializeHttpAutomaton(char* base_directory, char* http_error_buffer, byte* ip, uint port, byte verbose_mode, int inactivity_timeout_in_ticks, bool enable_directory_listing, bool enable_cgi);
void CleanupHttpAutomaton();
void DoHttpServerAutomatonStep();
