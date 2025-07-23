#ifndef CWR_INSTANCE_H
#define CWR_INSTANCE_H

#include <stdbool.h>
#include <cwr_program_context.h>
#include <cwr_value.h>
#include <cwr_node.h>

typedef struct cwr_func_call_context
{
    cwr_program_context context;
    cwr_value **arguments;
    size_t count;
    cwr_location location;
} cwr_func_call_context;

typedef cwr_value *(*cwr_function_instance_bind)(cwr_func_call_context context);

typedef enum cwr_instance_type
{
    cwr_instance_variable_type,
    cwr_instance_function_type
} cwr_instance_type;

typedef enum cwr_function_instance_type
{
    cwr_function_instance_user_type,
    cwr_function_instance_bind_type
} cwr_function_instance_type;

typedef struct cwr_function_instance
{
    cwr_function_instance_type type;
    cwr_argument *arguments;
    size_t count;

    union
    {
        cwr_function_instance_bind bind;
        cwr_func_body_expression body;
    };
} cwr_function_instance;

typedef struct cwr_variable_instance
{
    cwr_expression_type_value type;
    cwr_value *value;
} cwr_variable_instance;

typedef struct cwr_instance
{
    cwr_instance_type type;
    cwr_func_body_expression *root;
    size_t identifier;
    char *name;

    union
    {
        cwr_variable_instance variable;
        cwr_function_instance function;
    };
} cwr_instance;

static bool cwr_instance_can_access(cwr_instance instance, cwr_func_body_expression *requester)
{
    if (instance.root == NULL)
    {
        return true;
    }

    if (requester == NULL)
    {
        return false;
    }

    if (instance.root != requester)
    {
        return cwr_instance_can_access(instance, requester->root);
    }

    return true;
}

static void cwr_func_call_context_destroy(cwr_func_call_context context)
{
    cwr_value **arguments = context.arguments;

    if (context.count > 0)
    {
        for (size_t i = 0; i < context.count; i++)
        {
            cwr_value_runtime_destroy(arguments[i]);
        }

        free(arguments);
    }
}

static void cwr_variable_instance_destroy(cwr_instance instance)
{
    cwr_value_instance_destroy(instance.variable.value);
}

static void cwr_instance_destroy(cwr_instance instance)
{
    switch (instance.type)
    {
    case cwr_instance_variable_type:
        cwr_variable_instance_destroy(instance);
        break;
    case cwr_instance_function_type:
        break;
    }
}

#endif // CWR_INSTANCE_H