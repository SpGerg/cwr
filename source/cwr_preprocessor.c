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
    cwr_preprocessor_macros* macroses;
    size_t macroses_count;
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
    preprocessor->macroses = NULL;
    preprocessor->macroses_count = 0;

    return preprocessor;
}

cwr_preprocessor_result cwr_preprocessor_run(cwr_preprocessor* preprocessor) {
    preprocessor->is_failed = false;
    preprocessor->position = 0;

    while (cwr_preprocessor_is_not_ended(preprocessor)) {
        cwr_token current = cwr_preprocessor_current(preprocessor);

        if (current.type != cwr_token_directive_prefix_type) {
            if (current.type == cwr_token_word_type) {
                cwr_preprocessor_macros* macro = cwr_preprocessor_find_macros(preprocessor, current.value);
                if (macro != NULL && macro->value != NULL) {
                    size_t position = preprocessor->position;
                    size_t new_count = preprocessor->count - 1 + macro->value_count;
                    size_t tail_count = preprocessor->count - position - 1;
                    
                    cwr_token* new_buffer = realloc(preprocessor->tokens, new_count * sizeof(cwr_token));
                    if (new_buffer == NULL) {
                        cwr_preprocessor_throw_out_of_memory(preprocessor, current.location);
                        break;
                    }
                    
                    preprocessor->tokens = new_buffer;
                    
                    cwr_token_destroy(new_buffer[position]);
                    
                    if (tail_count > 0) {
                        memmove(
                            new_buffer + position + macro->value_count,
                            new_buffer + position + 1,
                            tail_count * sizeof(cwr_token)
                        );
                    }
                    
                    for (size_t i = 0; i < macro->value_count; i++) {
                        new_buffer[position + i] = cwr_token_clone(macro->value[i]);
                    }
                    
                    preprocessor->count = new_count;
                    preprocessor->position = position;
                    
                    continue;
                }
            }

            cwr_preprocessor_skip(preprocessor);
            continue;
        }

        cwr_preprocessor_skip(preprocessor);

        size_t directive_start = preprocessor->position - 1;
        if (cwr_preprocessor_match(preprocessor, cwr_token_include_type)) {
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

            // # include < [name].h >
            const size_t include_statement_tokens_count = 7;

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
        else if (cwr_preprocessor_match(preprocessor, cwr_token_define_type)) {
            cwr_token name = cwr_preprocessor_except(preprocessor, cwr_token_word_type);
            CWR_PREPROCESSOR_FAILED_AND_BREAK(preprocessor);

            cwr_preprocessor_macros macro = {
                .name = name.value,
                .value = NULL,
                .value_count = 0
            };

            bool has_value = false;
            size_t body_count = 0;

            if (!cwr_preprocessor_match(preprocessor, cwr_token_new_line_type)) {
                has_value = true;
                size_t body_start = preprocessor->position;
                
                while (cwr_preprocessor_is_not_ended(preprocessor)) {
                    cwr_token current = cwr_preprocessor_current(preprocessor);
                    if (current.type == cwr_token_new_line_type) {
                        break;
                    }

                    body_count++;
                    cwr_preprocessor_skip(preprocessor);
                }

                if (body_count > 0) {
                    cwr_token* tokens = malloc(body_count * sizeof(cwr_token));
                    if (tokens == NULL) {
                        cwr_preprocessor_throw_out_of_memory(preprocessor, name.location);
                        break;
                    }
                    
                    for (size_t i = 0; i < body_count; i++) {
                        tokens[i] = preprocessor->tokens[body_start + i];
                    }
                    
                    macro.value = tokens;
                    macro.value_count = body_count;
                }
            }

            if (!cwr_preprocessor_add_macros(preprocessor, macro)) {
                cwr_preprocessor_throw_out_of_memory(preprocessor, name.location);
                break;
            }

            size_t directive_token_count = 3; // # define [name] [new_line]
            if (has_value) {
                directive_token_count += body_count;
            }

            if (cwr_preprocessor_current(preprocessor).type == cwr_token_new_line_type) {
                directive_token_count++;
            }

            for (size_t i = 0; i < directive_token_count; i++) {
                cwr_token_destroy(preprocessor->tokens[directive_start + i]);
            }

            size_t elements_to_move = preprocessor->count - (directive_start + directive_token_count);
            if (elements_to_move > 0) {
                memmove(
                    preprocessor->tokens + directive_start,
                    preprocessor->tokens + directive_start + directive_token_count,
                    elements_to_move * sizeof(cwr_token)
                );
            }

            preprocessor->count -= directive_token_count;
            preprocessor->position = directive_start;
            continue;
        }
    }

    if (preprocessor->included_count > 0) { 
        free(preprocessor->included);
    }

    if (preprocessor->macroses_count > 0) { 
        free(preprocessor->macroses);
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
    char** new_included = realloc(preprocessor->included, (preprocessor->included_count + 1) * sizeof(char*));
    if (new_included == NULL) {
        return false;
    }
        
    preprocessor->included = new_included;
    preprocessor->included[preprocessor->included_count++] = name;
    return true;
}

bool cwr_preprocessor_add_macros(cwr_preprocessor* preprocessor, cwr_preprocessor_macros macros) {
    cwr_preprocessor_macros* buffer = realloc(preprocessor->macroses, (preprocessor->macroses_count + 1) * sizeof(cwr_preprocessor_macros));
    if (buffer == NULL) {
        return false;
    }
    
    preprocessor->macroses = buffer;
    preprocessor->macroses[preprocessor->macroses_count++] = macros;
    return true;
}

cwr_preprocessor_macros* cwr_preprocessor_find_macros(cwr_preprocessor* preprocessor, char* name) {
    for (size_t i = 0;i < preprocessor->macroses_count;i++) {
        cwr_preprocessor_macros* macros = &preprocessor->macroses[i];
        if (strcmp(macros->name, name) != 0) {
            continue;
        }

        return macros;
    }

    return NULL;
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
    free(preprocessor);
}