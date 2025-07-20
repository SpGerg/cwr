#ifndef CWR_PREPROCESSOR_H
#define CWR_PREPROCESSOR_H

#include <stdbool.h>
#include <cwr_preprocessor_error.h>
#include <cwr_token.h>

#define CWR_PREPROCESSOR_MAX_INCLUDED 64

#define CWR_PREPROCESSOR_FAILED_AND_BREAK(preprocessor) { \
    if (preprocessor->is_failed) { \
        break;\
    } \
}

typedef struct cwr_preprocessor cwr_preprocessor;

typedef struct cwr_preprocessor_result {
    cwr_tokens_list tokens_list;
    cwr_preprocessor_error error;
    bool is_failed;
} cwr_preprocessor_result;

cwr_preprocessor* cwr_preprocessor_create(cwr_tokens_list tokens_list);

cwr_preprocessor_result cwr_preprocessor_run(cwr_preprocessor* preprocessor);

bool cwr_preprocessor_is_included(cwr_preprocessor* preprocessor, const char* name);

bool cwr_preprocessor_add_included(cwr_preprocessor* preprocessor, char* name);

void cwr_preprocessor_skip(cwr_preprocessor* preprocessor);

cwr_token cwr_preprocessor_except(cwr_preprocessor* preprocessor, cwr_token_type type);

bool cwr_preprocessor_match(cwr_preprocessor* preprocessor, cwr_token_type type);

cwr_token cwr_preprocessor_current(cwr_preprocessor* preprocessor);

bool cwr_preprocessor_is_not_ended(cwr_preprocessor* preprocessor);

void cwr_preprocessor_throw_out_of_memory(cwr_preprocessor* preprocessor, cwr_location location);

void cwr_preprocessor_throw_error(cwr_preprocessor* preprocessor, cwr_preprocessor_error_type type, char* message, cwr_location location);

void cwr_preprocessor_destroy(cwr_preprocessor* preprocessor);

#endif //CWR_PREPROCESSOR_H