#ifndef __UTILS_H
#define __UTILS_H

#include "types.h"
#include "msxdos.h"

#define ToLower(c) ((c) | 32)
#define ToUpper(c) ((c) & ~32)

typedef struct {
    int year;
    byte month;
    byte day;
    byte hour;
    byte minute;
    byte second;
} dateTime;

bool StringStartsWith(const char *string, const char *prefix);
void ToVerboseDate(char* dst, dateTime* date_time);
void ToIsoDate(char* dst, dateTime* date_time);
void ParseFibDateTime(fileInfoBlock* fib, dateTime* date_time);
bool ParseVerboseDateTime(char* string, dateTime* date_time);
int CompareDates(dateTime* dt1, dateTime* dt2);
void UrlDecode(char *sSource, char *sDest, bool plusIsSpace);
void FormatIpAddress(char* dst, byte* address_bytes);

#endif
