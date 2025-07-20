#ifndef CWR_INTERPRETER_ERROR_H
#define CWR_INTERPRETER_ERROR_H

#include <cwr_token.h>

typedef enum cwr_interpreter_error_type {
    cwr_interpreter_error_incorrect_type_type,
    cwr_interpreter_error_out_of_memory_type,
    cwr_interpreter_error_entry_point_not_found_type,
    cwr_interpreter_error_negative_index_type,
    cwr_interpreter_error_index_out_of_range_type,
    cwr_interpreter_error_division_by_zero_type,
    cwr_interpreter_error_unknown_statement_type,
    cwr_interpreter_error_unknown_expression_type
} cwr_interpreter_error_type;

typedef struct cwr_interpreter_error {
    cwr_interpreter_error_type type;
    char* message;
    bool is_free_message;
    cwr_location location;
    bool is_failed;
} cwr_interpreter_error;

static void cwr_interpreter_error_throw(cwr_interpreter_error* error, cwr_interpreter_error_type type, char* message, cwr_location location) {
    error->type = type;
    error->message = message;
    error->is_free_message = false;
    error->location = location;
    error->is_failed = true;
}

static void cwr_interpreter_error_throw_out_of_memory(cwr_interpreter_error* error,  cwr_location location) {
    cwr_interpreter_error_throw(error, cwr_interpreter_error_out_of_memory_type, "Out of memory", location);
}

#endif //CWR_INTERPRETER_ERROR_H