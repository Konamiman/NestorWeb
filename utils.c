#include "types.h"
#include "utils.h"
#include "msxdos.h"
#include <stdio.h>
#include <stdlib.h>

static const char* week_days[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static const char* months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

bool StringStartsWith(const char *string, const char *prefix) {
    char c1;
    char c2;
    
    while(c2 = *prefix++)
    {
        if(ToLower(c2) != ToLower(*string++))
            return false;
    }

    return true;
}


static char* WeekDayForDate(int y, byte m, byte d)
{
    //https://stackoverflow.com/a/21235587/4574
    return week_days[(d += m < 3 ? y-- : y - 2, 23*m/9 + d + 4 + y/4- y/100 + y/400)%7];
}


static char* PaddingFor(byte value)
{
    return value < 10 ? "0" : "";
}


void ToVerboseDate(char* dst, dateTime* date_time)
{
    //<week-day>, <day> <month> <year> <hour>:<minute>:<second> GMT
    sprintf(
        dst,
         "%s, %s%i %s %i %s%i:%s%i:%s%i GMT",
         WeekDayForDate(date_time->year, date_time->month, date_time->day),
         PaddingFor(date_time->day), date_time->day,
         months[date_time->month-1],
         date_time->year,
         PaddingFor(date_time->hour), date_time->hour,
         PaddingFor(date_time->minute), date_time->minute,
         PaddingFor(date_time->second), date_time->second);
}


void ToIsoDate(char* dst, dateTime* date_time)
{
    //<year>-<month>-<day> <hour>:<minute>:<second>
    sprintf(
        dst,
         "%i-%s%i-%s%i %s%i:%s%i:%s%i",
         date_time->year,
         PaddingFor(date_time->month), date_time->month,
         PaddingFor(date_time->day), date_time->day,
         PaddingFor(date_time->hour), date_time->hour,
         PaddingFor(date_time->minute), date_time->minute,
         PaddingFor(date_time->second), date_time->second);
}


void ParseFibDateTime(fileInfoBlock* fib, dateTime* date_time)
{
    /*
     Bits 15...9 - YEAR (0..99 corresponding to 1980..2079)
     Bits  8...5 - MONTH (1..12 corresponding to Jan..Dec)
     Bits  4...0 - DAY (1..31)

     Bits 15..11 - HOURS (0..23)
     Bits 10...5 - MINUTES (0..59)
     Bits  4...0 - SECONDS/2 (0..29)
    */

    if(fib->dateOfModification == 0)
    {
        date_time->year = 0;
        return;
    }

    date_time->year = ((fib->dateOfModification >> 9) & 0x7F) + 1980;
    date_time->month = (fib->dateOfModification >> 5) & 0xF;
    date_time->day = fib->dateOfModification & 0x1F;
    date_time->hour = (fib->timeOfModification >> 11) & 0x1F;
    date_time->minute = (fib->timeOfModification >> 5) & 0x3F;
    date_time->second = (fib->timeOfModification & 0x1F) * 2;
}


bool ParseVerboseDateTime(char* string, dateTime* date_time)
{
    //Wed, 21 Oct 2015 07:28:00 GMT 
    //012345678901234567890123
    //          11111111112222

    int temp;

    while(*string == ' ') string++;

    temp = atoi(&string[5]);
    if(temp == 0 || temp > 31) return false;
    date_time->day = (byte)temp;

    temp = 0;
    while(true)
    {
        if(StringStartsWith(&string[8], months[temp]))
        {
            date_time->month = temp+1;
            break;
        }
        else if(temp == 12)
            return false;
        
        temp++;
    }

    temp = atoi(&string[12]);
    if(temp < 1980) return false;
    date_time->year = temp;

    temp = atoi(&string[17]);
    if(temp > 23) return false;
    date_time->hour = (byte)temp;

    temp = atoi(&string[20]);
    if(temp > 59) return false;
    date_time->minute = (byte)temp;

    temp = atoi(&string[23]);
    if(temp > 59) return false;
    date_time->second = (byte)temp;

    return true;
}


// <0 if dt1<dt2, 0 if dt1==dt2, >0 if dt1>dt2
int CompareDates(dateTime* dt1, dateTime* dt2)
{
    if(dt1->year != dt2->year)
        return dt1->year - dt2->year;
    
    if(dt1->month != dt2->month)
        return dt1->month - dt2->month;

    if(dt1->day != dt2->day)
        return dt1->day - dt2->day;

    if(dt1->hour != dt2->hour)
        return dt1->hour - dt2->hour;
    
    if(dt1->minute != dt2->minute)
        return dt1->minute - dt2->minute;

    return dt1->second - dt2->second;
}


static char isxdigit(unsigned char c)
{
    return ( c >= '0' && c <= '9') ||
         ( c >= 'a' && c <= 'f') ||
         ( c >= 'A' && c <= 'F');
}


//https://stackoverflow.com/a/20437049/4574
void UrlDecode(char *sSource, char *sDest, bool plusIsSpace) 
{
    int nLength;
    char ch;

    for (nLength = 0; *sSource; nLength++) {
        if ((ch = *sSource) == '+' && plusIsSpace)
        {
            sDest[nLength] = ' ';
            sSource++;
            continue;
        }
        if (ch == '%' && sSource[1] && sSource[2] && isxdigit(sSource[1]) && isxdigit(sSource[2])) {
            sSource[1] -= sSource[1] <= '9' ? '0' : (sSource[1] <= 'F' ? 'A' : 'a')-10;
            sSource[2] -= sSource[2] <= '9' ? '0' : (sSource[2] <= 'F' ? 'A' : 'a')-10;
            sDest[nLength] = 16 * sSource[1] + sSource[2];
            sSource += 3;
            continue;
        }
        sDest[nLength] = ch;
        sSource++;
    }
    sDest[nLength] = '\0';
}


void FormatIpAddress(char* dst, byte* address_bytes)
{
    sprintf(dst, "%i.%i.%i.%i", address_bytes[0], address_bytes[1], address_bytes[2], address_bytes[3]);
}
