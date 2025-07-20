#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <cwr_preprocessor.h>
#include <cwr_preprocessor_includer.h>
#include <cwr_string.h>
#include <cwr_lexer.h>

typedef struct cwr_preprocessor {
    cwr_tokens_list source;
    cwr_token* tokens;
    size_t count;
    size_t position;
    char** included;
    size_t included_count;
    cwr_preprocessor_error error;
    bool is_failed;
} cwr_preprocessor;

cwr_preprocessor* cwr_preprocessor_create(cwr_tokens_list tokens_list) {
    cwr_preprocessor* preprocessor = malloc(sizeof(cwr_preprocessor));
    if (preprocessor == NULL) {
        return NULL;
    }

    preprocessor->tokens = tokens_list.tokens;
    preprocessor->count = tokens_list.count;
    preprocessor->source = tokens_list;
    preprocessor->included = NULL;
    preprocessor->included_count = 0;

    return preprocessor;
}

cwr_preprocessor_result cwr_preprocessor_run(cwr_preprocessor* preprocessor) {
    preprocessor->is_failed = false;
    preprocessor->position = 0;

    while (cwr_preprocessor_is_not_ended(preprocessor)) {
        if (cwr_preprocessor_match(preprocessor, cwr_token_include_type)) {
            size_t directive_start = preprocessor->position - 1;
            
            cwr_preprocessor_except(preprocessor, cwr_token_less_than_type);
            CWR_PREPROCESSOR_FAILED_AND_BREAK(preprocessor);

            cwr_token name = cwr_preprocessor_except(preprocessor, cwr_token_word_type);
            CWR_PREPROCESSOR_FAILED_AND_BREAK(preprocessor);

            cwr_preprocessor_except(preprocessor, cwr_token_dot_type);
            CWR_PREPROCESSOR_FAILED_AND_BREAK(preprocessor);

            cwr_token header = cwr_preprocessor_except(preprocessor, cwr_token_word_type);
            CWR_PREPROCESSOR_FAILED_AND_BREAK(preprocessor);

            if (strcmp(header.value, "h") != 0) {
                cwr_preprocessor_throw_error(preprocessor, cwr_preprocessor_error_except_token_type, "Except header type", header.location);
                break;
            }

            cwr_preprocessor_except(preprocessor, cwr_token_greater_than_type);
            CWR_PREPROCESSOR_FAILED_AND_BREAK(preprocessor);

            const size_t include_statement_tokens_count = 6;

            if (cwr_preprocessor_is_included(preprocessor, name.value)) {
                for (size_t i = 0; i < include_statement_tokens_count; i++) {
                    cwr_token_destroy(preprocessor->tokens[directive_start + i]);
                }
                
                size_t elements_to_move = preprocessor->count - (directive_start + include_statement_tokens_count);
                if (elements_to_move > 0) {
                    memmove(
                        preprocessor->tokens + directive_start,
                        preprocessor->tokens + directive_start + include_statement_tokens_count,
                        elements_to_move * sizeof(cwr_token)
                    );
                }
                
                preprocessor->count -= include_statement_tokens_count;
                preprocessor->position = directive_start;
                continue;
            }

            if (!cwr_preprocessor_add_included(preprocessor, name.value)) {
                cwr_preprocessor_throw_out_of_memory(preprocessor, name.location);
                break;
            }

            char* source = cwr_preprocessor_includer_get_from_std(name.value);
            if (source == NULL) {
                cwr_preprocessor_throw_error(preprocessor, cwr_preprocessor_error_module_not_found_type, "Module not found", name.location);
                break;
            }

            cwr_lexer* lexer = cwr_lexer_create(preprocessor->source.executor, source, cwr_lexer_configuration_default());
            if (lexer == NULL) {
                cwr_preprocessor_throw_out_of_memory(preprocessor, name.location);
                break;
            }

            cwr_tokens_list included_tokens = cwr_lexer_tokenize(lexer);
            
            size_t start_index = preprocessor->position - include_statement_tokens_count;
            size_t new_count = preprocessor->count - include_statement_tokens_count + included_tokens.count;
            size_t tail_count = preprocessor->count - preprocessor->position;

            for (size_t i = 0; i < include_statement_tokens_count; i++) {
                cwr_token_destroy(preprocessor->tokens[start_index + i]);
            }

            cwr_token* new_buffer = realloc(preprocessor->tokens, new_count * sizeof(cwr_token));
            if (new_buffer == NULL) {
                for (size_t i = 0; i < included_tokens.count; i++) {
                    cwr_token_destroy(included_tokens.tokens[i]);
                }

                free(included_tokens.tokens);
                cwr_lexer_destroy(lexer);
                
                cwr_preprocessor_throw_out_of_memory(preprocessor, name.location);
                break;
            }
            
            preprocessor->tokens = new_buffer;
            preprocessor->count = new_count;
            
            if (tail_count > 0) {
                memmove(
                    new_buffer + start_index + included_tokens.count,
                    new_buffer + start_index + include_statement_tokens_count,
                    tail_count * sizeof(cwr_token)
                );
            }

            for (size_t i = 0; i < included_tokens.count; i++) {
                new_buffer[start_index + i] = included_tokens.tokens[i];
            }
            
            preprocessor->position = directive_start;

            free(included_tokens.tokens);
            cwr_lexer_destroy(lexer);
            continue;
        }

        cwr_preprocessor_skip(preprocessor);
    }

    return (cwr_preprocessor_result) {
        .tokens_list = (cwr_tokens_list) {
            .source = preprocessor->source.source,
            .tokens = preprocessor->tokens,
            .count = preprocessor->count
        },
        .error = preprocessor->error,
        .is_failed = preprocessor->is_failed
    };
}

bool cwr_preprocessor_is_included(cwr_preprocessor* preprocessor, const char* name) {
    for (size_t i = 0; i < preprocessor->included_count; i++) {
        if (strcmp(preprocessor->included[i], name) != 0) {
            continue;
        }

        return true;
    }

    return false;
}

bool cwr_preprocessor_add_included(cwr_preprocessor* preprocessor, char* name) {
    char* duplicate = cwr_string_duplicate(name);
    if (duplicate == NULL) {
        return false;
    }

    char** new_included = realloc(preprocessor->included, (preprocessor->included_count + 1) * sizeof(char*));
    if (new_included == NULL) {
        free(duplicate);
        return false;
    }
        
    preprocessor->included = new_included;
    preprocessor->included[preprocessor->included_count++] = duplicate;
    return true;
}

void cwr_preprocessor_skip(cwr_preprocessor* preprocessor) {
    preprocessor->position++;
}

cwr_token cwr_preprocessor_except(cwr_preprocessor* preprocessor, cwr_token_type type) {
    cwr_token current = cwr_preprocessor_current(preprocessor);

    if (current.type == type) {
        cwr_preprocessor_skip(preprocessor);
        return current;
    }

    cwr_preprocessor_throw_error(preprocessor, cwr_preprocessor_error_except_token_type, "Except token", current.location);
    return current;
}

bool cwr_preprocessor_match(cwr_preprocessor* preprocessor, cwr_token_type type) {
    if (cwr_preprocessor_current(preprocessor).type == type) {
        cwr_preprocessor_skip(preprocessor);
        return true;
    }

    return false;
}

cwr_token cwr_preprocessor_current(cwr_preprocessor* preprocessor) {
    if (!cwr_preprocessor_is_not_ended(preprocessor)) {
        return preprocessor->tokens[preprocessor->count - 1];
    }

    return preprocessor->tokens[preprocessor->position];
}

bool cwr_preprocessor_is_not_ended(cwr_preprocessor* preprocessor) {
    return preprocessor->position < preprocessor->count - 1;
}

void cwr_preprocessor_throw_out_of_memory(cwr_preprocessor* preprocessor, cwr_location location) {
    cwr_preprocessor_throw_error(preprocessor, cwr_preprocessor_error_out_of_memory_type, "Out of memory", location);
}

void cwr_preprocessor_throw_error(cwr_preprocessor* preprocessor, cwr_preprocessor_error_type type, char* message, cwr_location location) {
    preprocessor->error = (cwr_preprocessor_error) {
        .error_type = type,
        .message = message,
        .is_free_message = false,
        .location = location
    };
    preprocessor->is_failed = true;
}

void cwr_preprocessor_destroy(cwr_preprocessor* preprocessor) {
    if (preprocessor->included_count > 0) { 
        for (size_t i = 0; i < preprocessor->included_count; i++) {
            free(preprocessor->included[i]);
        }

        free(preprocessor->included);
        preprocessor->included = NULL;
        preprocessor->included_count = 0;
    }

    free(preprocessor);
}