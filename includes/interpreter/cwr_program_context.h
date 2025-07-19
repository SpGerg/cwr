#ifndef CWR_PROGRAM_CONTEXT_H
#define CWR_PROGRAM_CONTEXT_H

typedef struct cwr_program_context {
    struct cwr_scope* functions;
    struct cwr_scope* variables;
} cwr_program_context;

cwr_program_context cwr_program_context_default();

void cwr_program_context_destroy(cwr_program_context context);

#endif //CWR_PROGRAM_CONTEXT_H