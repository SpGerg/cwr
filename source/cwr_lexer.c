#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <cwr_lexer.h>
#include <cwr_string_buffer.h>

typedef struct cwr_lexer {
    char* source;
    size_t length;
    char* executor;
    cwr_token* tokens;
    size_t capacity;
    cwr_string_buffer* buffer;
    cwr_lexer_configuration configuration;
    size_t position;
    bool add_new_line;
} cwr_lexer;

static cwr_location cwr_lexer_create_location(cwr_lexer* lexer) {
    return cwr_location_create(lexer->executor, lexer->position);
}

cwr_lexer* cwr_lexer_create(char* executor, char* source, cwr_lexer_configuration configuration) {
    cwr_lexer* lexer = malloc(sizeof(cwr_lexer));
    if (lexer == NULL) {
        return NULL;
    }

    lexer->buffer = cwr_string_buffer_create();
    if (lexer->buffer == NULL) {
        free(lexer);

        return NULL;
    }

    lexer->source = source;
    lexer->length = strlen(source);
    lexer->executor = executor;
    lexer->configuration = configuration;
    lexer->position = 0;

    return lexer;
}

char cwr_lexer_current(cwr_lexer* lexer) {
    return lexer->source[lexer->position];
}

void cwr_lexer_skip(cwr_lexer* lexer) {
    lexer->position++;
}

bool cwr_lexer_not_ended(cwr_lexer* lexer) {
    return lexer->position < lexer->length;
}

void cwr_lexer_add_token_by_str(cwr_lexer* lexer, char* value, bool check_token) {
    if (strcmp(value, "") == 0) {
        free(value);

        return;
    }

    cwr_token_type type = cwr_token_word_type;
    if (check_token) {
        cwr_lexer_configuration_try_get_token(lexer->configuration, value, &type);
    }

    cwr_lexer_add_token_string(lexer, type, value, true);
}

void cwr_lexer_add_buffer(cwr_lexer* lexer, bool check_token) {
    char* value = cwr_string_buffer_copy_and_clear(lexer->buffer);
    cwr_lexer_add_token_by_str(lexer, value, check_token);
}

bool cwr_lexer_add_token(cwr_lexer* lexer, cwr_token token) {
    cwr_token* buffer = realloc(lexer->tokens, (lexer->capacity + 1) * sizeof(cwr_token));
    if (buffer == NULL) {
        return false;
    }

    if (token.type == cwr_token_define_type) {
        lexer->add_new_line = true;
    }

    lexer->tokens = buffer;
    lexer->tokens[lexer->capacity++] = token;
    return true;
}

bool cwr_lexer_add_token_string(cwr_lexer* lexer, cwr_token_type type, char* value, bool is_free_name) {
    cwr_token token = cwr_token_create(type, value, cwr_lexer_create_location(lexer), is_free_name);
    return cwr_lexer_add_token(lexer, token);
}

bool cwr_lexer_add_token_char(cwr_lexer* lexer, cwr_token_type type, char value) {
    char* buffer = malloc(sizeof(char) * 2);
    if (buffer == NULL) {
        return false;
    }

    buffer[0] = value;
    buffer[1] = '\0';
    cwr_token token = cwr_token_create(type, buffer, cwr_lexer_create_location(lexer), true);

    if (!cwr_lexer_add_token(lexer, token)) {
        free(buffer);
        return false;
    }

    return true;
}

cwr_tokens_list cwr_lexer_tokenize(cwr_lexer* lexer) {
    lexer->tokens = NULL;
    lexer->capacity = 0;
    lexer->position = 0;
    lexer->add_new_line = false;

    while (cwr_lexer_not_ended(lexer)) {
        char current = cwr_lexer_current(lexer);

        if (iscntrl(current)) {
            if (current == '\n' && lexer->add_new_line) {
                cwr_lexer_add_token_char(lexer, cwr_token_new_line_type, '\n');
                lexer->add_new_line = false;
            }

            cwr_lexer_add_buffer(lexer, true);
            cwr_lexer_skip(lexer);

            continue;
        }

        if (cwr_string_buffer_is_empty(lexer->buffer) && isdigit(current)) {
            while (isdigit(current)) {
                cwr_string_buffer_append(lexer->buffer, current);
                cwr_lexer_skip(lexer);
                current = cwr_lexer_current(lexer);
            }

            cwr_lexer_add_token_string(lexer, cwr_token_number_type, cwr_string_buffer_copy_and_clear(lexer->buffer), true);
            continue;
        }

        if (current == '"') {
            cwr_lexer_skip(lexer);
            current = cwr_lexer_current(lexer);

            while (current != '"') {
                if (!cwr_lexer_not_ended(lexer)) {
                    break;
                }

                cwr_string_buffer_append(lexer->buffer, current);
                cwr_lexer_skip(lexer);
                current = cwr_lexer_current(lexer);
            }

            cwr_lexer_skip(lexer);
            cwr_lexer_add_token_string(lexer, cwr_token_string_type, cwr_string_buffer_copy_and_clear(lexer->buffer), true);
            continue;
        }
        else if (current == '\'') {
            cwr_lexer_skip(lexer);

            char value = cwr_lexer_current(lexer);
            cwr_lexer_skip(lexer);
            cwr_lexer_skip(lexer);

            cwr_string_buffer_clear(lexer->buffer);
            cwr_lexer_add_token_char(lexer, cwr_token_char_type, value);
            continue;
        }
        else if (current == '/' && cwr_lexer_not_ended(lexer) && lexer->source[lexer->position + 1] == '/') {
            while (cwr_lexer_not_ended(lexer) && cwr_lexer_current(lexer) != '\n') {
                cwr_lexer_skip(lexer);
            }

            continue;
        }

        cwr_string_buffer_append(lexer->buffer, current);
        cwr_lexer_skip(lexer);

        if (!cwr_lexer_not_ended(lexer)) {
            // Getting last symbol
            current = lexer->source[strlen(lexer->source) - 1];
        }

        if (isspace(current)) {
            char* buffer = cwr_string_buffer_copy_and_clear(lexer->buffer);
            buffer[strlen(buffer) - 1] = '\0';

            cwr_lexer_add_token_by_str(lexer, buffer, true);
            continue;
        }

        cwr_token_type operator_type;

        if (!cwr_lexer_configuration_try_get_token_char(lexer->configuration, current, &operator_type)) {
            char* value = cwr_string_buffer_copy(lexer->buffer);
            cwr_token_type type;

            if (!cwr_lexer_configuration_try_get_token(lexer->configuration, value, &type)) {
                free(value);

                continue;
            }

            if (cwr_lexer_not_ended(lexer) && current != ' ') {
                free(value);

                continue;
            }

            cwr_lexer_add_token_string(lexer, type, value, true);
            continue;
        }

        char* value = cwr_string_buffer_copy_and_clear(lexer->buffer);

        if (current != value[0]) {
            // Remove the operator
            value[strlen(value) - 1] = '\0';
            cwr_lexer_add_token_by_str(lexer, value, true);
        }
        else {
            free(value);
        }

        cwr_lexer_add_token_char(lexer, operator_type, current);
    }

    cwr_lexer_add_token_by_str(lexer, cwr_string_buffer_copy_and_clear(lexer->buffer), false);
    return (cwr_tokens_list) {
        .source = lexer->source,
        .executor = lexer->executor,
        .tokens = lexer->tokens,
        .count = lexer->capacity
    };
}

void cwr_lexer_destroy(cwr_lexer* lexer) {
    cwr_string_buffer_destroy(lexer->buffer);
    free(lexer);
}
