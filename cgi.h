#ifndef __CGI_H
#define __CGI_H

#include "types.h"

void InitializeCgiEngine();
void ReinitializeCgiEngine(byte errorCodeFromCgi);
void RunCgi();
void SendResultAfterCgi();
byte OpenCgiOutFileForRead(byte* file_handle);
void CleanupCgiEngine();
void StartSendingCgiResult();
void ContinueSendingCgiResultHeaders();
bool SetupRequestDependantEnvItems();

#endif