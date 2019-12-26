#ifndef __CGI_H
#define __CGI_H

#include "types.h"

void InitializeCgiEngine();
void RestoreStandardFileHandles();
void ReinitializeCgiEngine(byte errorCodeFromCgi);
void RunCgi();
void SendResultAfterCgi();
bool CreateAndRedirectInFile();
void ContinueReadingBody();
void CleanupCgiEngine();
void StartSendingCgiResult();
void ContinueSendingCgiResultHeaders();
bool SetupRequestDependantEnvItems();
void ProcessHeaderForCgi();
void CleanupHeaderBasedEnvItems();

#endif