#ifndef CWR_STRING_BUFFER_H
#define CWR_STRING_BUFFER_H

#include <stdbool.h>

typedef struct cwr_string_buffer cwr_string_buffer;

cwr_string_buffer *cwr_string_buffer_create();

bool cwr_string_buffer_append(cwr_string_buffer *string_buffer, char value);

char *cwr_string_buffer_copy(cwr_string_buffer *string_buffer);

char *cwr_string_buffer_copy_and_clear(cwr_string_buffer *string_buffer);

bool cwr_string_buffer_is_empty(cwr_string_buffer *string_buffer);

void cwr_string_buffer_clear(cwr_string_buffer *string_buffer);

void cwr_string_buffer_destroy(cwr_string_buffer *string_buffer);

#endif // CWR_STRING_BUFFER_H