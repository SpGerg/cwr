#ifndef CWR_LEXER_CONFIGURATION_H
#define CWR_LEXER_CONFIGURATION_H

#include <string.h>
#include <cwr_token.h>

#define CWR_LEXER_CONFIGURATION_TOKENS_COUNT 28
#define CWR_LEXER_CONFIGURATION_ADD_TOKEN(type, value) cwr_lexer_configuration_add_token(configuration_pointer, type, value); // internal

typedef struct cwr_lexer_token_config
{
    cwr_token_type type;
    char *value;
} cwr_lexer_token_config;

typedef struct cwr_lexer_configuration
{
    cwr_lexer_token_config tokens[CWR_LEXER_CONFIGURATION_TOKENS_COUNT];
    size_t count;
} cwr_lexer_configuration;

static cwr_lexer_token_config cwr_lexer_configuration_create_token(cwr_token_type type, char *value)
{
    return (cwr_lexer_token_config){.type = type, .value = value};
}

static void cwr_lexer_configuration_add_token(cwr_lexer_configuration *configuration, cwr_token_type type, char *value)
{
    if (configuration->count >= CWR_LEXER_CONFIGURATION_TOKENS_COUNT) {
        return;
    }

    configuration->tokens[configuration->count++] = cwr_lexer_configuration_create_token(type, value);
}

static cwr_lexer_configuration cwr_lexer_configuration_default()
{
    cwr_lexer_configuration configuration = { .count = 0 };
    cwr_lexer_configuration *configuration_pointer = &configuration;
    CWR_LEXER_CONFIGURATION_ADD_TOKEN(cwr_token_return_type, "return");
    CWR_LEXER_CONFIGURATION_ADD_TOKEN(cwr_token_int_type, "int");
    CWR_LEXER_CONFIGURATION_ADD_TOKEN(cwr_token_float_type, "float");
    CWR_LEXER_CONFIGURATION_ADD_TOKEN(cwr_token_struct_type, "struct");
    CWR_LEXER_CONFIGURATION_ADD_TOKEN(cwr_token_void_type, "void");
    CWR_LEXER_CONFIGURATION_ADD_TOKEN(cwr_token_char_word_type, "char");
    CWR_LEXER_CONFIGURATION_ADD_TOKEN(cwr_token_if_type, "if");
    CWR_LEXER_CONFIGURATION_ADD_TOKEN(cwr_token_for_type, "for");
    CWR_LEXER_CONFIGURATION_ADD_TOKEN(cwr_token_directive_prefix_type, "#");
    CWR_LEXER_CONFIGURATION_ADD_TOKEN(cwr_token_left_par_type, "(");
    CWR_LEXER_CONFIGURATION_ADD_TOKEN(cwr_token_right_par_type, ")");
    CWR_LEXER_CONFIGURATION_ADD_TOKEN(cwr_token_left_curly_type, "{");
    CWR_LEXER_CONFIGURATION_ADD_TOKEN(cwr_token_right_curly_type, "}");
    CWR_LEXER_CONFIGURATION_ADD_TOKEN(cwr_token_left_square_type, "[");
    CWR_LEXER_CONFIGURATION_ADD_TOKEN(cwr_token_right_square_type, "]");
    CWR_LEXER_CONFIGURATION_ADD_TOKEN(cwr_token_plus_type, "+");
    CWR_LEXER_CONFIGURATION_ADD_TOKEN(cwr_token_minus_type, "-");
    CWR_LEXER_CONFIGURATION_ADD_TOKEN(cwr_token_slash_type, "/");
    CWR_LEXER_CONFIGURATION_ADD_TOKEN(cwr_token_greater_than_type, ">");
    CWR_LEXER_CONFIGURATION_ADD_TOKEN(cwr_token_less_than_type, "<");
    CWR_LEXER_CONFIGURATION_ADD_TOKEN(cwr_token_exclamation_mark_type, "!");
    CWR_LEXER_CONFIGURATION_ADD_TOKEN(cwr_token_equals_type, "=");
    CWR_LEXER_CONFIGURATION_ADD_TOKEN(cwr_token_dot_type, ".");
    CWR_LEXER_CONFIGURATION_ADD_TOKEN(cwr_token_asterisk_type, "*");
    CWR_LEXER_CONFIGURATION_ADD_TOKEN(cwr_token_ampersand_type, "&");
    CWR_LEXER_CONFIGURATION_ADD_TOKEN(cwr_token_semicolon_type, ";");
    CWR_LEXER_CONFIGURATION_ADD_TOKEN(cwr_token_colon_type, ":");
    CWR_LEXER_CONFIGURATION_ADD_TOKEN(cwr_token_comma_type, ",");

    return configuration;
}

static bool cwr_lexer_configuration_try_get_token(cwr_lexer_configuration* configuration, char *name, cwr_token_type *type)
{
    for (size_t i = 0; i < configuration->count; i++)
    {
        cwr_lexer_token_config token = configuration->tokens[i];

        if (strcmp(token.value, name) != 0)
        {
            continue;
        }

        *type = token.type;
        return true;
    }

    return false;
}

static bool cwr_lexer_configuration_try_get_token_char(cwr_lexer_configuration* configuration, char name, cwr_token_type *type)
{
    for (size_t i = 0; i < configuration->count; i++)
    {
        cwr_lexer_token_config token = configuration->tokens[i];

        if (token.value == NULL || token.value[0] != name || token.value[1] != '\0')
        {
            continue;
        }

        *type = token.type;
        return true;
    }

    return false;
}

static char *cwr_lexer_configuration_get_value(cwr_lexer_configuration* configuration, cwr_token_type type)
{
    for (size_t i = 0; i < configuration->count; i++)
    {
        cwr_lexer_token_config token = configuration->tokens[i];

        if (token.type != type)
        {
            continue;
        }

        return token.value;
    }

    return NULL;
}

#endif // CWR_LEXER_CONFIGURATION_H