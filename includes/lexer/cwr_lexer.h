#ifndef CWR_LEXER_H
#define CWR_LEXER_H

#include <stdlib.h>
#include <cwr_token.h>
#include <cwr_lexer_configuration.h>

typedef struct cwr_lexer cwr_lexer;

cwr_lexer* cwr_lexer_create(char* executor, char* source, cwr_lexer_configuration configuration);

char cwr_lexer_current(cwr_lexer* lexer);

void cwr_lexer_skip(cwr_lexer* lexer);

bool cwr_lexer_not_ended(cwr_lexer* lexer);

void cwr_lexer_add_token_by_str(cwr_lexer* lexer, char* value, bool check_token);

void cwr_lexer_add_buffer(cwr_lexer* lexer, bool check_token);

bool cwr_lexer_add_token(cwr_lexer* lexer, cwr_token token);

bool cwr_lexer_add_token_string(cwr_lexer* lexer, cwr_token_type type, char* value, bool is_free_name);

bool cwr_lexer_add_token_char(cwr_lexer* lexer, cwr_token_type type, char value);

cwr_tokens_list cwr_lexer_tokenize(cwr_lexer* lexer);

void cwr_lexer_destroy(cwr_lexer* lexer);

#endif //CWR_LEXER_H
