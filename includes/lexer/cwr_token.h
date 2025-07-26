#ifndef CWR_TOKEN_H
#define CWR_TOKEN_H

#include <stdlib.h>
#include <stdbool.h>
#include <cwr_string.h>

typedef enum cwr_token_type
{
    // Does not exist at parsing, only preprocessor uses it
    cwr_token_new_line_type,
    cwr_token_word_type,
    cwr_token_string_type,
    cwr_token_character_type,
    cwr_token_number_type,
    cwr_token_return_type,
    cwr_token_int_type,
    cwr_token_float_type,
    cwr_token_struct_type,
    cwr_token_char_word_type,
    cwr_token_void_type,
    cwr_token_if_type,
    cwr_token_for_type,
    cwr_token_directive_prefix_type,
    cwr_token_left_par_type,
    cwr_token_right_par_type,
    cwr_token_left_curly_type,
    cwr_token_right_curly_type,
    cwr_token_left_square_type,
    cwr_token_right_square_type,
    cwr_token_plus_type,
    cwr_token_minus_type,
    cwr_token_slash_type,
    cwr_token_greater_than_type,
    cwr_token_less_than_type,
    cwr_token_exclamation_mark_type,
    cwr_token_equals_type,
    cwr_token_dot_type,
    cwr_token_asterisk_type,
    cwr_token_ampersand_type,
    cwr_token_semicolon_type,
    cwr_token_colon_type,
    cwr_token_comma_type
} cwr_token_type;

typedef struct cwr_location
{
    char *executor;
    size_t position;
} cwr_location;

typedef struct cwr_token
{
    cwr_token_type type;
    char *value;
    cwr_location location;
    bool is_free_value;
} cwr_token;

typedef struct cwr_tokens_list
{
    char *source;
    char *executor;
    cwr_token *tokens;
    size_t count;
} cwr_tokens_list;

static cwr_location cwr_location_create(char *executor, size_t position)
{
    return (cwr_location){
        .executor = executor,
        .position = position};
}

static cwr_token cwr_token_create(cwr_token_type type, char *value, cwr_location location, bool is_free_value)
{
    return (cwr_token){
        .type = type,
        .value = value,
        .location = location,
        .is_free_value = is_free_value};
}

static cwr_token cwr_token_clone(cwr_token token)
{
    return (cwr_token){
        .type = token.type,
        .value = cwr_string_duplicate(token.value),
        .location = token.location,
        .is_free_value = true};
}

static void cwr_token_destroy(cwr_token token)
{
    if (token.is_free_value)
    {
        free(token.value);
    }
}

static void cwr_tokens_list_destroy(cwr_tokens_list tokens_list)
{
    for (size_t i = 0; i < tokens_list.count; i++)
    {
        cwr_token token = tokens_list.tokens[i];

        cwr_token_destroy(token);
    }

    free(tokens_list.tokens);
}

static inline bool cwr_token_type_is_value(cwr_token_type type)
{
    switch (type)
    {
    case cwr_token_string_type:
        return true;
    case cwr_token_number_type:
        return true;
    case cwr_token_character_type:
        return true;
    default:
        return false;
    }
}

#endif // CWR_TOKEN_H
