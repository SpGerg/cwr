#ifndef CWR_PARSER_ERROR_H
#define CWR_PARSER_ERROR_H

#include <cwr_token.h>

typedef enum cwr_parser_error_type {
    cwr_parser_error_unknown_statement_type,
    cwr_parser_error_unknown_function_type,
    cwr_parser_error_unknown_variable_type,
    cwr_parser_error_out_of_memory_type,
    cwr_parser_error_except_token_type,
    cwr_parser_error_incorrect_type_type,
    cwr_parser_error_except_value_type
} cwr_parser_error_type;

typedef struct cwr_parser_error {
    cwr_parser_error_type error_type;
    char* message;
    bool is_free_message;
    cwr_location location;
} cwr_parser_error;

#endif //CWR_PARSER_ERROR_H