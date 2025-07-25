#ifndef CWR_PREPROCESSOR_H
#define CWR_PREPROCESSOR_H

#include <stdbool.h>
#include <cwr_preprocessor_error.h>
#include <cwr_token.h>

#define CWR_PREPROCESSOR_ENDIF "endif"
#define CWR_PREPROCESSOR_DEFINED_FUNC "defined"

#define CWR_PREPROCESSOR_FAILED_AND_BREAK(preprocessor) \
    {                                                   \
        if (preprocessor->is_failed)                    \
        {                                               \
            break;                                      \
        }                                               \
    }

#define CWR_PREPROCESSOR_FAILED_AND_RETURN(preprocessor) \
    {                                                    \
        if (preprocessor->is_failed)                     \
        {                                                \
            return false;                                \
        }                                                \
    }

typedef struct cwr_preprocessor cwr_preprocessor;

typedef struct cwr_preprocessor_macros
{
    char *name;
    cwr_token *value;
    size_t value_count;
    long number;
    bool with_number;
} cwr_preprocessor_macros;

typedef struct cwr_preprocessor_result
{
    cwr_tokens_list tokens_list;
    cwr_preprocessor_error error;
    bool is_failed;
} cwr_preprocessor_result;

cwr_preprocessor *cwr_preprocessor_create(cwr_tokens_list tokens_list);

cwr_preprocessor_result cwr_preprocessor_run(cwr_preprocessor *preprocessor);

bool cwr_preprocessor_parse_include(cwr_preprocessor *preprocessor, size_t directive_start);

bool cwr_preprocessor_parse_if(cwr_preprocessor *preprocessor, cwr_location location, size_t directive_start);

bool cwr_preprocessor_parse_binary(cwr_preprocessor *preprocessor, int *result);

bool cwr_preprocessor_parse_multiplicative(cwr_preprocessor *preprocessor, int *result);

bool cwr_preprocessor_parse_unary(cwr_preprocessor *preprocessor, int *result);

bool cwr_preprocessor_parse_value(cwr_preprocessor *preprocessor, int *result);

bool cwr_preprocessor_parse_macros_definition(cwr_preprocessor *preprocessor, size_t directive_start);

bool cwr_preprocessor_parse_string_concatenation(cwr_preprocessor *preprocessor, cwr_token current, cwr_token *previous);

bool cwr_preprocessor_parse_macros_expansion(cwr_preprocessor *preprocessor, cwr_preprocessor_macros macros, cwr_location location);

bool cwr_preprocessor_is_included(cwr_preprocessor *preprocessor, const char *name);

bool cwr_preprocessor_add_included(cwr_preprocessor *preprocessor, char *name);

bool cwr_preprocessor_add_macros(cwr_preprocessor *preprocessor, cwr_preprocessor_macros macros);

cwr_preprocessor_macros *cwr_preprocessor_find_macros(cwr_preprocessor *preprocessor, char *name);

void cwr_preprocessor_add(cwr_preprocessor *preprocessor, cwr_token token);

void cwr_preprocessor_skip(cwr_preprocessor *preprocessor);

cwr_token cwr_preprocessor_except(cwr_preprocessor *preprocessor, cwr_token_type type);

bool cwr_preprocessor_match(cwr_preprocessor *preprocessor, cwr_token_type type);

cwr_token cwr_preprocessor_peek(cwr_preprocessor *preprocessor, size_t offset);

cwr_token cwr_preprocessor_current(cwr_preprocessor *preprocessor);

bool cwr_preprocessor_is_not_ended(cwr_preprocessor *preprocessor);

void cwr_preprocessor_throw_out_of_memory(cwr_preprocessor *preprocessor, cwr_location location);

void cwr_preprocessor_throw_error(cwr_preprocessor *preprocessor, cwr_preprocessor_error_type type, char *message, cwr_location location);

void cwr_preprocessor_macros_destroy(cwr_preprocessor_macros macros);

void cwr_preprocessor_destroy(cwr_preprocessor *preprocessor);

#endif // CWR_PREPROCESSOR_H