#include <stdlib.h>
#include <string.h>
#include <cwr_string_buffer.h>
#include <cwr_string.h>

typedef struct cwr_string_buffer {
    char* buffer;
    size_t capacity;
} cwr_string_buffer;

cwr_string_buffer* cwr_string_buffer_create() {
    cwr_string_buffer* string_buffer = malloc(sizeof(cwr_string_buffer));

    if (string_buffer == NULL) {
        return NULL;
    }

    char* buffer = malloc(sizeof(char));

    if (buffer == NULL) {
        free(string_buffer);
        return NULL;
    }

    string_buffer->buffer = buffer;
    string_buffer->capacity = 0;
    return string_buffer;
}

bool cwr_string_buffer_append(cwr_string_buffer* string_buffer, char value) {
    char* buffer = realloc(string_buffer->buffer, (string_buffer->capacity + 2) * sizeof(char));

    if (buffer == NULL) {
        return false;
    }

    string_buffer->buffer = buffer;
    string_buffer->buffer[string_buffer->capacity++] = value;
    string_buffer->buffer[string_buffer->capacity] = '\0';
    return true;
}

char* cwr_string_buffer_copy(cwr_string_buffer* string_buffer) {
    return cwr_string_duplicate(string_buffer->buffer);
}

char* cwr_string_buffer_copy_and_clear(cwr_string_buffer* string_buffer) {
    char* result = cwr_string_buffer_copy(string_buffer);

    cwr_string_buffer_clear(string_buffer);
    return result;
}

bool cwr_string_buffer_is_empty(cwr_string_buffer* string_buffer) {
    if (string_buffer->capacity == 0) {
        return true;
    }

    for (size_t i = 0;i < string_buffer->capacity;i++) {
        if (string_buffer->buffer[i] == ' ') {
            continue;
        }

        return false;
    }

    return true;
}

void cwr_string_buffer_clear(cwr_string_buffer* string_buffer) {
    string_buffer->capacity = 0;
    string_buffer->buffer = realloc(string_buffer->buffer, sizeof(char));
    string_buffer->buffer[0] = '\0';
}

void cwr_string_buffer_destroy(cwr_string_buffer* string_buffer) {
    free(string_buffer->buffer);
    free(string_buffer);
}