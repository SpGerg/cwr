#include <cwr_program_context.h>
#include <cwr_scope.h>

cwr_program_context cwr_program_context_default() {
    return (cwr_program_context) {
        .functions = cwr_scope_create(),
        .variables = cwr_scope_create()
    };
}

void cwr_program_context_destroy(cwr_program_context context) {
    cwr_scope_destroy(context.functions);
    cwr_scope_destroy(context.variables);
}
