#ifndef CWR_LEXER_CONFIGURATION_H
#define CWR_LEXER_CONFIGURATION_H

#include <string.h>
#include <cwr_token.h>

#define CWR_LEXER_CONFIGURATION_TOKENS_COUNT 27

typedef struct cwr_lexer_token_config {
    cwr_token_type type;
    char* value;
} cwr_lexer_token_config;

typedef struct cwr_lexer_configuration {
    cwr_lexer_token_config tokens[CWR_LEXER_CONFIGURATION_TOKENS_COUNT];
    size_t count;
} cwr_lexer_configuration;

static cwr_lexer_token_config cwr_lexer_configuration_create_token(cwr_token_type type, char* value) {
    return (cwr_lexer_token_config) { .type = type, .value = value };
}

static cwr_lexer_configuration cwr_lexer_configuration_default() {
    cwr_lexer_configuration configuration;
    configuration.tokens[0] = cwr_lexer_configuration_create_token(cwr_token_return_type, "return");
    configuration.tokens[1] = cwr_lexer_configuration_create_token(cwr_token_int_type, "int");
    configuration.tokens[2] = cwr_lexer_configuration_create_token(cwr_token_float_type, "float");
    configuration.tokens[3] = cwr_lexer_configuration_create_token(cwr_token_struct_type, "struct");
    configuration.tokens[4] = cwr_lexer_configuration_create_token(cwr_token_void_type, "void");
    configuration.tokens[5] = cwr_lexer_configuration_create_token(cwr_token_char_word_type, "char");
    configuration.tokens[6] = cwr_lexer_configuration_create_token(cwr_token_if_type, "if");
    configuration.tokens[7] = cwr_lexer_configuration_create_token(cwr_token_for_type, "for");
    configuration.tokens[8] = cwr_lexer_configuration_create_token(cwr_token_include_type, "#include");
    configuration.tokens[9] = cwr_lexer_configuration_create_token(cwr_token_left_par_type, "(");
    configuration.tokens[10] = cwr_lexer_configuration_create_token(cwr_token_right_par_type, ")");
    configuration.tokens[11] = cwr_lexer_configuration_create_token(cwr_token_left_curly_type, "{");
    configuration.tokens[12] = cwr_lexer_configuration_create_token(cwr_token_right_curly_type, "}");
    configuration.tokens[13] = cwr_lexer_configuration_create_token(cwr_token_left_square_type, "[");
    configuration.tokens[14] = cwr_lexer_configuration_create_token(cwr_token_right_square_type, "]");
    configuration.tokens[15] = cwr_lexer_configuration_create_token(cwr_token_plus_type, "+");
    configuration.tokens[16] = cwr_lexer_configuration_create_token(cwr_token_minus_type, "-");
    configuration.tokens[17] = cwr_lexer_configuration_create_token(cwr_token_slash_type, "/");
    configuration.tokens[18] = cwr_lexer_configuration_create_token(cwr_token_greater_than_type, ">");
    configuration.tokens[19] = cwr_lexer_configuration_create_token(cwr_token_less_than_type, "<");
    configuration.tokens[20] = cwr_lexer_configuration_create_token(cwr_token_equals_type, "=");
    configuration.tokens[21] = cwr_lexer_configuration_create_token(cwr_token_dot_type, ".");
    configuration.tokens[22] = cwr_lexer_configuration_create_token(cwr_token_asterisk_type, "*");
    configuration.tokens[23] = cwr_lexer_configuration_create_token(cwr_token_ampersand_type, "&");
    configuration.tokens[24] = cwr_lexer_configuration_create_token(cwr_token_semicolon_type, ";");
    configuration.tokens[25] = cwr_lexer_configuration_create_token(cwr_token_colon_type, ":");
    configuration.tokens[CWR_LEXER_CONFIGURATION_TOKENS_COUNT - 1] = cwr_lexer_configuration_create_token(cwr_token_comma_type, ",");

    return configuration;
}

static bool cwr_lexer_configuration_try_get_token(cwr_lexer_configuration configuration, char* name, cwr_token_type* type) {
    for (size_t i = 0;i < CWR_LEXER_CONFIGURATION_TOKENS_COUNT;i++) {
        cwr_lexer_token_config token = configuration.tokens[i];

        if (strcmp(token.value, name) != 0) {
            continue;
        }

        *type = token.type;
        return true;
    }

    return false;
}

static bool cwr_lexer_configuration_try_get_token_char(cwr_lexer_configuration configuration, char name, cwr_token_type* type) {
    for (size_t i = 0;i < CWR_LEXER_CONFIGURATION_TOKENS_COUNT;i++) {
        cwr_lexer_token_config token = configuration.tokens[i];

        if (token.value == NULL || token.value[1] != '\0' || token.value[0] != name) {
            continue;
        }

        *type = token.type;
        return true;
    }

    return false;
}

static char* cwr_lexer_configuration_get_value(cwr_lexer_configuration configuration, cwr_token_type type) {
    for (size_t i = 0;i < CWR_LEXER_CONFIGURATION_TOKENS_COUNT;i++) {
        cwr_lexer_token_config token = configuration.tokens[i];

        if (token.type != type) {
            continue;
        }

        return token.value;
    }

    return NULL;
}

#endif //CWR_LEXER_CONFIGURATION_H