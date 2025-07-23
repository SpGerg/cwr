#ifndef CWR_PREPROCESSOR_INCLUDER_H
#define CWR_PREPROCESSOR_INCLUDER_H

#include <string.h>

#define CWR_STDIO "stdio"
#define CWR_STDIO_SOURCE          \
    "void printf(char *content);" \
    "void printf(char content);"  \
    "void printf(float content);"

static char *cwr_preprocessor_includer_get_from_std(char *name)
{
    if (strcmp(CWR_STDIO, name) == 0)
    {
        return CWR_STDIO_SOURCE;
    }

    return NULL;
}

#endif // CWR_PREPROCESSOR_INCLUDER_H