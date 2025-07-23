#ifndef CWR_STRING_H
#define CWR_STRING_H

#include <stdlib.h>
#include <string.h>

static inline char *cwr_string_duplicate(char *source)
{
    char *copy = malloc(strlen(source) + 1);
    if (copy == NULL)
    {
        return NULL;
    }

    strcpy(copy, source);
    return copy;
}

#endif // CWR_STRING_H