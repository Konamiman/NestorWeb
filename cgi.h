#ifndef __CGI_H
#define __CGI_H

#include "types.h"

void RunCgi();
void SendResultAfterCgi(byte error_code_from_cgi);

#endif