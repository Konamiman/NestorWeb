#include "types.h"
#include "http.h"
#include "stdio.h"


static byte automaton_state;
static char* error_buffer;


void InitializeHttpAutomaton(char* http_error_buffer)
{
    automaton_state = HTTPA_NONE;
    error_buffer = http_error_buffer;
    error_buffer[0] = '\0';
}


void DoHttpServerAutomatonStep()
{

}
