#ifndef CWR_INTERPRETER_H
#define CWR_INTERPRETER_H

#include <cwr_program_context.h>
#include <cwr_instance.h>
#include <cwr_value.h>
#include <cwr_parser.h>
#include <cwr_interpreter_error.h>

#define CWR_INTERPRETER_ENTRY_POINT_FUNC "main" 

typedef struct cwr_interpreter cwr_interpreter;

typedef struct cwr_interpreter_result {
    cwr_parser_result source;
    cwr_program_context context;
} cwr_interpreter_result;

cwr_interpreter* cwr_intepreter_create(cwr_parser_result result);

cwr_interpreter_result cwr_intepreter_interpret(cwr_interpreter* interpreter, cwr_interpreter_error* error);

cwr_value* cwr_interpreter_evaluate_entry_point(cwr_interpreter_result result, cwr_interpreter_error* error);

cwr_value* cwr_intepreter_evaluate_stat(cwr_program_context program_context, cwr_statement statement, cwr_func_body_expression* root, cwr_interpreter_error* error);

cwr_value* cwr_intepreter_evaluate_stat_func_call(cwr_program_context program_context, cwr_func_call_statement statement, cwr_func_body_expression* root, cwr_location location, cwr_interpreter_error* error);

cwr_value* cwr_intepreter_evaluate_func(cwr_function_instance instance, cwr_func_call_context context, cwr_interpreter_error* error);

cwr_value* cwr_intepreter_evaluate_expr(cwr_program_context program_context, cwr_expression expression, cwr_func_body_expression* root, cwr_interpreter_error* error);

cwr_value* cwr_intepreter_evaluate_expr_body(cwr_program_context program_context, cwr_func_body_expression expression, cwr_func_body_expression* root, cwr_interpreter_error* error);

void cwr_interpreter_result_destroy(cwr_interpreter_result result);

void cwr_intepreter_destroy(cwr_interpreter* interpreter);

#endif //CWR_INTERPRETER_H