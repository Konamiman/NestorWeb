#include "types.h"


enum HttpAutomatonStates {
    HTTPA_NONE,
    HTTPA_LISTENING,
    HTTPA_READING_REQUEST,
    HTTPA_READING_HEADERS,
    HTTPA_SENDING_RESPONSE
};


void InitializeHttpAutomaton(char* http_error_buffer);
void DoHttpServerAutomatonStep();
