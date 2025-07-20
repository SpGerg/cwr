#include <stdlib.h>
#include <string.h>
#include <cwr_scope.h>

typedef struct cwr_scope {
    cwr_instance* instances;
    size_t size;
    size_t capacity;
} cwr_scope;

cwr_scope* cwr_scope_create() {
    cwr_scope* scope = malloc(sizeof(cwr_scope));
    if (scope == NULL) {
        return NULL;
    }

    scope->instances = malloc(sizeof(cwr_instance) * CWR_SCOPE_DEFAULT_SIZE);
    if (scope->instances == NULL) {
        free(scope);
        return NULL;
    }

    scope->size = CWR_SCOPE_DEFAULT_SIZE;
    scope->capacity = 0;
    return scope;
}

bool cwr_scope_add(cwr_scope* scope, cwr_instance instance) {
    if (scope->capacity >= scope->size) {
        cwr_instance* instances = realloc(scope->instances, sizeof(cwr_instance) * scope->size * 2);
        if (instances == NULL) {
            return false;
        }

        scope->size *= 2;
        scope->instances = instances;
    }

    scope->instances[scope->capacity++] = instance;
    return true;
}

cwr_instance* cwr_scope_get(cwr_scope* scope, cwr_func_body_expression* root, size_t identifier) {
    for (int i = scope->capacity - 1;i >= 0;i--) {
        cwr_instance* instance = &scope->instances[i];
        if (instance->identifier != identifier) {
            continue;
        }

        if (!cwr_instance_can_access(*instance, root)) {
            continue;
        }

        return instance;
    }

    // That cant happen because of parser checks
    return NULL;
}

cwr_instance* cwr_scope_get_by_name(cwr_scope* scope, cwr_func_body_expression* root, char* name) {
    for (int i = scope->capacity - 1;i >= 0;i--) {
        cwr_instance* instance = &scope->instances[i];
        if (strcmp(instance->name, name) != 0) {
            continue;
        }

        if (!cwr_instance_can_access(*instance, root)) {
            continue;
        }

        return instance;
    }

    // That cant happen because of parser checks
    return NULL;
}

void cwr_scope_destroy(cwr_scope* scope) {
    for (size_t i = 0;i < scope->capacity;i++) {
        cwr_instance_destroy(scope->instances[i]);
    }

    free(scope->instances);
    free(scope);
}

void cwr_scope_root_destroy(cwr_scope* scope, cwr_func_body_expression* root) {
    for (int i = scope->capacity - 1;i >= 0;i--) {
        cwr_instance instance = scope->instances[i];
        if (instance.root != root) {
            continue;
        }

        if (instance.type == cwr_instance_variable_type) {
            cwr_value_remove_reference(instance.variable.value);
        }

        cwr_instance_destroy(instance);
        scope->capacity--;
    }
}