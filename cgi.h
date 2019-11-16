#ifndef __CGI_H
#define __CGI_H

#include "types.h"

void InitializeCgiEngine();
#define ReinitializeCgiEngine() InitializeCgiEngine()
void RunCgi();
void SendResultAfterCgi(byte error_code_from_cgi);

#endif