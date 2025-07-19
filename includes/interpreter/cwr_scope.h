#ifndef CWR_SCOPE_H
#define CWR_SCOPE_H

#include <cwr_instance.h>

#define CWR_SCOPE_DEFAULT_SIZE 8

typedef struct cwr_scope cwr_scope;

cwr_scope* cwr_scope_create();

bool cwr_scope_add(cwr_scope* scope, cwr_instance instance);

cwr_instance* cwr_scope_get(cwr_scope* scope, cwr_func_body_expression* root, size_t identifier);

void cwr_scope_destroy(cwr_scope* scope);

void cwr_scope_root_destroy(cwr_scope* scope, cwr_func_body_expression* root);

#endif //CWR_SCOPE_H