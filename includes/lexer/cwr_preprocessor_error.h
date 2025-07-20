#ifndef CWR_PREPROCESSOR_ERROR_H
#define CWR_PREPROCESSOR_ERROR_H

#include <cwr_token.h>

typedef enum cwr_preprocessor_error_type {
    cwr_preprocessor_error_incorrect_include_type,
    cwr_preprocessor_error_except_token_type,
    cwr_preprocessor_error_module_not_found_type,
    cwr_preprocessor_error_out_of_memory_type
} cwr_preprocessor_error_type;

typedef struct cwr_preprocessor_error {
    cwr_preprocessor_error_type error_type;
    char* message;
    bool is_free_message;
    cwr_location location;
} cwr_preprocessor_error;

#endif //CWR_PREPROCESSOR_ERROR_H