#include "types.h"
#include "utils.h"
#include <stdio.h>

bool strncmpi(const char *s1, const char *s2, int len) {
    char c1, c2;

    while(len--)
    {
        c1 = ToLower(*s1++);
        c2 = ToLower(*s2++);

        if(c1 == 0 && c2 == 0)
            return true;
        
        if(c1 != c2)
            return false;
    }

    return c1 == c2;
}
