#ifndef CWR_PARSER_H
#define CWR_PARSER_H

#include <cwr_node.h>

#define CWR_PARSER_FAILED_AND_RETURN(parser, type) { \
    if (parser->is_failed) { \
        return (type) {};\
    } \
}

#define CWR_PARSER_FAILED_AND_RETURN_V(parser) { \
    if (parser->is_failed) { \
        return;\
    } \
}

typedef struct cwr_parser cwr_parser;

typedef struct cwr_parser_variable {
    size_t identifier;
    char* name;
    cwr_func_body_expression* root;
    cwr_expression_type_value type;
    cwr_expression* static_value;
} cwr_parser_variable;

typedef struct cwr_parser_function {
    size_t identifier;
    char* name;
    cwr_argument* arguments;
    size_t count;
    cwr_expression_type_value return_type;
} cwr_parser_function;

typedef struct cwr_parser_result {
    cwr_nodes_list nodes_list;
    cwr_parser_variable* global_variables;
    size_t global_variables_count;
    cwr_parser_function* functions;
    size_t functions_count;
    cwr_parser_error error;
    bool is_failed;
} cwr_parser_result;

cwr_parser* cwr_parser_create(cwr_tokens_list tokens_list);

cwr_parser_result cwr_parser_parse(cwr_parser* parser);

cwr_statement_type cwr_parser_get_statement(cwr_parser* parser);

bool cwr_parser_add_statement(cwr_parser* parser, cwr_statement node);

bool cwr_parser_add_included(cwr_parser* parser, cwr_parser_result result);

bool cwr_parser_add_function(cwr_parser* parser, cwr_parser_function function);

bool cwr_parser_add_variable(cwr_parser* parser, cwr_parser_variable variable);

void cwr_parser_clear_scope(cwr_parser* parser, cwr_func_body_expression* root);

cwr_statement cwr_parser_parse_declaration(cwr_parser* parser);

cwr_statement cwr_parser_parse_statement(cwr_parser* parser);

cwr_statement cwr_parser_parse_only_statement(cwr_parser* parser);

cwr_assign_statement cwr_parser_parse_assign(cwr_parser* parser);

cwr_var_decl_statement cwr_parser_parse_variable_declaration(cwr_parser* parser, cwr_expression_type_value type);

cwr_func_decl_statement cwr_parser_parse_function_declaration(cwr_parser* parser, cwr_expression_type_value type);

cwr_if_statement cwr_parser_parse_if(cwr_parser* parser);

cwr_for_loop_statement cwr_parser_parse_for_loop(cwr_parser* parser);

cwr_func_call_statement cwr_parser_parse_function_call(cwr_parser* parser);

cwr_return_statement cwr_parser_parse_return(cwr_parser* parser, cwr_expression_type_value type);

void cwr_parser_parse_function_body_by_created(cwr_parser* parser, cwr_func_body_expression* body);

cwr_func_body_expression cwr_parser_parse_function_body(cwr_parser* parser);

cwr_expression_type_value cwr_parser_parse_type(cwr_parser* parser);

cwr_expression cwr_parser_parse_binary(cwr_parser* parser);

cwr_expression cwr_parser_parse_multiplicative(cwr_parser* parser);

cwr_expression cwr_parser_parse_unary(cwr_parser* parser, bool only_value);

cwr_expression cwr_parser_parse_array_element(cwr_parser* parser);

cwr_expression cwr_parser_parse_value(cwr_parser* parser);

bool cwr_parser_get_function(cwr_parser* parser, char* name, cwr_expression* argument, size_t count, cwr_parser_function* function);

bool cwr_parser_get_variable(cwr_parser* parser, char* name, cwr_parser_variable* variable);

cwr_token cwr_parser_except(cwr_parser* parser, cwr_token_type token_type);

bool cwr_parser_peek(cwr_parser* parser, size_t offset, cwr_token_type token_type);

bool cwr_parser_match(cwr_parser* parser, cwr_token_type token_type);

cwr_token cwr_parser_current(cwr_parser* parser);

bool cwr_parser_ended(cwr_parser* parser);

void cwr_parser_skip(cwr_parser* parser);

void cwr_parser_throw_out_of_memory(cwr_parser* parser, cwr_location location);

void cwr_parser_throw_error(cwr_parser* parser, cwr_parser_error_type type, char* message, cwr_location location);

void cwr_parser_result_destroy(cwr_parser_result parser_result);

void cwr_parser_destroy(cwr_parser* parser);

#endif //CWR_PARSER_H