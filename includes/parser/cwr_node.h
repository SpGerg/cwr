#ifndef CWR_NODE_H
#define CWR_NODE_H

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <cwr_parser_error.h>
#include <cwr_token.h>

typedef struct cwr_statement cwr_statement;
typedef struct cwr_expression cwr_expression;
typedef struct cwr_binary_expression cwr_binary_expression;
typedef struct cwr_func_body_expression cwr_func_body_expression;
typedef struct cwr_expression_type_value cwr_expression_type_value;

static void cwr_statement_destroy(cwr_statement statement);
static void cwr_expression_destroy(cwr_expression expression);

typedef enum cwr_binary_operator_type {
    cwr_binary_operator_none_type,
    cwr_binary_operator_plus_type,
    cwr_binary_operator_minus_type,
    cwr_binary_operator_multiplicative_type,
    cwr_binary_operator_division_type,
    cwr_binary_operator_equals_type,
    cwr_binary_operator_greater_than_type,
    cwr_binary_operator_less_than_type,
    cwr_binary_operator_greater_equals_than_type,
    cwr_binary_operator_less_equals_than_type,
} cwr_binary_operator_type;

typedef enum cwr_statement_type {
    cwr_statement_struct_decl_type,
    cwr_statement_func_decl_type,
    cwr_statement_func_call_type,
    cwr_statement_var_decl_type,
    cwr_statement_assign_type,
    cwr_statement_for_loop_type,
    cwr_statement_if_type,
    cwr_statement_return_type
} cwr_statement_type;

typedef enum cwr_expression_type {
    cwr_expression_type_type,
    cwr_expression_binary_type,
    cwr_expression_func_call_type,
    cwr_expression_array_type,
    cwr_expression_array_element_type,
    cwr_expression_var_type,
    cwr_expression_dereference_type,
    cwr_expression_reference_type,
    cwr_expression_character_type,
    cwr_expression_unary_type,
    cwr_expression_float_type,
    cwr_expression_integer_type
} cwr_expression_type;

typedef enum cwr_value_type {
    cwr_value_character_type,
    cwr_value_float_type,
    cwr_value_integer_type,
    cwr_value_structure_type,
    cwr_value_array_type,
    cwr_value_pointer_type,
    cwr_value_void_type
} cwr_value_type;

typedef struct cwr_expression_type_value {
    cwr_value_type value_type;
    int identifier;
    char* name;
    cwr_expression_type_value* target_type;
} cwr_expression_type_value;

typedef struct cwr_character_expression {
    char value;
} cwr_character_expression;

typedef struct cwr_float_expression {
    float value;
} cwr_float_expression;

typedef struct cwr_integer_expression {
    int value;
} cwr_integer_expression;

typedef struct cwr_argument {
    int identifier;
    char* name;
    cwr_expression_type_value type;
} cwr_argument;

typedef struct cwr_func_body_expression {
    cwr_func_body_expression* root;
    cwr_statement* statements;
    size_t count;
} cwr_func_body_expression;

typedef struct cwr_array_expression {
    cwr_expression_type_value type;
    cwr_expression* elements;
    size_t count;
} cwr_array_expression;

typedef struct cwr_array_element_expression {
    // First value, second index
    cwr_expression* children;
} cwr_array_element_expression;

typedef struct cwr_func_decl_statement {
    int identifier;
    char* name;
    cwr_argument* arguments;
    size_t count;
    cwr_func_body_expression func_body;
    bool with_body;
    cwr_expression_type_value return_type;
} cwr_func_decl_statement;

typedef struct cwr_dereference_expression {
    cwr_expression* value;
} cwr_dereference_expression;

typedef struct cwr_reference_expression {
    cwr_expression* value;
} cwr_reference_expression;

typedef struct cwr_var_expression {
    int identifier;
    char* name;
} cwr_var_expression;

typedef struct cwr_func_call_statement {
    int identifier;
    char* name;
    cwr_expression_type_value return_type;
    cwr_expression* arguments;
    size_t count;
} cwr_func_call_statement;

typedef struct cwr_binary_expression {
    cwr_binary_operator_type type;
    // First left, second right
    cwr_expression* children;
} cwr_binary_expression;

typedef struct cwr_unary_expression {
    cwr_binary_operator_type type;
    // Value
    cwr_expression* child;
} cwr_unary_expression;

typedef struct cwr_expression {
    cwr_expression_type type;
    cwr_expression_type_value value_type;
    cwr_func_body_expression root;
    cwr_location location;

    union {
        cwr_func_call_statement func_call;
        cwr_binary_expression binary;
        cwr_unary_expression unary;
        cwr_var_expression var;
        cwr_float_expression float_n;
        cwr_integer_expression integer_n;
        cwr_character_expression character;
        cwr_array_expression array;
        cwr_array_element_expression array_element;
        cwr_dereference_expression dereference;
        cwr_reference_expression reference;
    };
} cwr_expression;

typedef struct cwr_var_decl_statement {
    int identifier;
    char* name;
    cwr_expression_type_value value_type;
    cwr_expression value;
} cwr_var_decl_statement;

typedef struct cwr_assign_statement {
    cwr_expression identifier;
    cwr_expression value;
    bool is_dereference;
} cwr_assign_statement;

typedef struct cwr_for_loop_statement {
    cwr_var_decl_statement variable;
    bool with_variable;
    cwr_expression condition;
    bool with_condition;
    cwr_statement* statement;
    bool with_statement;
    cwr_func_body_expression body;
    bool with_body;
} cwr_for_loop_statement;

typedef struct cwr_if_statement {
    cwr_expression condition;
    cwr_func_body_expression body;
} cwr_if_statement;

typedef struct cwr_return_statement {
    cwr_expression value;
    cwr_func_body_expression body;
    bool with_body;
} cwr_return_statement;

typedef struct cwr_statement {
    cwr_statement_type type;
    cwr_func_body_expression root;
    cwr_location location;

    union {
        cwr_func_decl_statement func_decl;
        cwr_func_call_statement func_call;
        cwr_var_decl_statement var_decl;
        cwr_assign_statement assign;
        cwr_return_statement ret;
        cwr_for_loop_statement for_loop;
        cwr_if_statement if_stat;
    };
} cwr_statement;

typedef struct cwr_nodes_list {
    cwr_statement* statements;
    size_t count;
} cwr_nodes_list;

static bool cwr_expression_type_value_equals(cwr_expression_type_value type, cwr_expression_type_value target) {
    if (type.value_type == cwr_value_array_type || type.value_type == cwr_value_pointer_type) {
        if (target.value_type != cwr_value_array_type && target.value_type != cwr_value_pointer_type) {
            return false;
        }

        return cwr_expression_type_value_equals(*type.target_type, *target.target_type);
    }

    if (type.identifier != -1) {
        return type.identifier == target.identifier && type.value_type == target.value_type;
    }

    if (type.value_type == cwr_value_float_type && target.value_type == cwr_value_integer_type) {
        return true;
    }

    return type.value_type == target.value_type;
}

static bool cwr_func_body_can_access(cwr_func_body_expression* target, cwr_func_body_expression* requester) {
    if (target == NULL) {
        return true;
    }

    if (requester == NULL) {
        return false;
    }

    if (target != requester) {
        return cwr_func_body_can_access(target, requester->root);
    }

    return true;
}

static cwr_binary_operator_type cwr_binary_operator_type_from_token(cwr_token_type type) {
    switch (type) {
        case cwr_token_plus_type:
            return cwr_binary_operator_plus_type;
        case cwr_token_minus_type:
            return cwr_binary_operator_minus_type;
        case cwr_token_asterisk_type:
            return cwr_binary_operator_multiplicative_type;
        case cwr_token_slash_type:
            return cwr_binary_operator_division_type;
        case cwr_token_equals_type:
            return cwr_binary_operator_equals_type;
        case cwr_token_greater_than_type:
            return cwr_binary_operator_greater_than_type;
        case cwr_token_less_than_type:
            return cwr_binary_operator_less_than_type;
        default:
            return cwr_binary_operator_none_type;
    }
}

static cwr_expression_type_value cwr_expression_type_value_create_from_type(cwr_value_type value_type) {
    return (cwr_expression_type_value) {
        .value_type = value_type
    };
}

static cwr_expression_type_value cwr_expression_type_value_create_array_from_type(cwr_value_type value_type) {
    cwr_expression_type_value array_type = cwr_expression_type_value_create_from_type(value_type);

    return (cwr_expression_type_value) {
        .value_type = cwr_value_array_type,
        .target_type = &array_type
    };
}

static cwr_expression_type_value cwr_expression_type_value_create_array(cwr_expression_type_value* type_value) {
    return (cwr_expression_type_value) {
        .value_type = cwr_value_array_type,
        .target_type = type_value
    };
}

static cwr_float_expression cwr_expression_create_float(float value) {
    return (cwr_float_expression) {
        .value = value
    };
}

static cwr_integer_expression cwr_expression_create_integer(int value) {
    return (cwr_integer_expression) {
        .value = value
    };
}

static void cwr_expression_type_value_destroy(cwr_expression_type_value type_value) {
    if (type_value.name != NULL) {
        free(type_value.name);
    }

    if (type_value.target_type) {
        cwr_expression_type_value_destroy(*type_value.target_type);
        free(type_value.target_type);
    }
}

static void cwr_expression_destroy_body(cwr_func_body_expression expression) {
    for (size_t i = 0;i < expression.count;i++) {
        cwr_statement_destroy(expression.statements[i]);
    }

    free(expression.statements);
}

static void cwr_statement_destroy_func_call(cwr_func_call_statement func_call) {
    free(func_call.name);

    if (func_call.count > 0) {
        for (size_t i = 0;i < func_call.count;i++) {
            cwr_expression argument = func_call.arguments[i];

            cwr_expression_destroy(argument);
        }
    }

    free(func_call.arguments);
}

static void cwr_expression_destroy(cwr_expression expression) {
    switch (expression.type)
    {
        case cwr_expression_binary_type:
            cwr_expression_destroy(expression.binary.children[0]);
            cwr_expression_destroy(expression.binary.children[1]);
            free(expression.binary.children);
            break;
        case cwr_expression_unary_type:
            cwr_expression_destroy(expression.unary.child[0]);
            free(expression.unary.child);
            break;
        case cwr_expression_dereference_type:
            cwr_expression_destroy(*expression.dereference.value);
            free(expression.dereference.value);
            break;
        case cwr_expression_reference_type:
            cwr_expression_destroy(*expression.reference.value);
            free(expression.reference.value);
            break;
        case cwr_expression_var_type:
            free(expression.var.name);
            break;
        case cwr_expression_type_type:
            cwr_expression_type_value_destroy(expression.value_type);
            break;
        case cwr_expression_array_type:
            cwr_expression_type_value_destroy(expression.value_type);
            for (size_t i = 0;i < expression.array.count;i++) {
                cwr_expression_destroy(expression.array.elements[i]);
            }
            
            free(expression.array.elements);
            break;
        case cwr_expression_array_element_type:
            cwr_expression_destroy(expression.array_element.children[0]);
            cwr_expression_destroy(expression.array_element.children[1]);
            free(expression.array_element.children);
            break;
        case cwr_expression_func_call_type:
            cwr_statement_destroy_func_call(expression.func_call);
            break;
    }
}

static void cwr_func_body_expression_destroy(cwr_func_body_expression expression) {
    if (expression.count > 0) {
        for (size_t i = 0;i < expression.count;i++) {
            cwr_statement statement = expression.statements[i];

            cwr_statement_destroy(statement);
        }

        free(expression.statements);
    }
}

static void cwr_var_decl_destroy(cwr_var_decl_statement var_decl) {
    free(var_decl.name);
    cwr_expression_type_value_destroy(var_decl.value_type);
    cwr_expression_destroy(var_decl.value);
}

static void cwr_func_decl_destroy(cwr_func_decl_statement func_decl) {
    if (func_decl.count > 0) {
        for (size_t i = 0;i < func_decl.count;i++) {
            cwr_argument argument = func_decl.arguments[i];
            free(argument.name);
            cwr_expression_type_value_destroy(argument.type);
        }

        free(func_decl.arguments);
    }

    free(func_decl.name);
    cwr_expression_type_value_destroy(func_decl.return_type);

    if (func_decl.with_body) {
        cwr_expression_destroy_body(func_decl.func_body);
    }
}

static void cwr_statement_destroy_for_loop(cwr_for_loop_statement statement) {
    if (statement.with_variable) {
        cwr_var_decl_destroy(statement.variable);
    }

    if (statement.with_condition) {
        cwr_expression_destroy(statement.condition);
    }

    if (statement.with_statement) {
        cwr_statement_destroy(*statement.statement);
        free(statement.statement);
    }
    
    if (statement.with_body) {
        cwr_expression_destroy_body(statement.body);
    }
}

static void cwr_statement_destroy(cwr_statement statement) {
    switch (statement.type) {
        case cwr_statement_func_decl_type:
            free(statement.func_decl.name);
            if (statement.func_decl.count > 0) {
                for (size_t i = 0;i < statement.func_decl.count;i++) {
                    cwr_argument argument = statement.func_decl.arguments[i];

                    free(argument.name);
                    cwr_expression_type_value_destroy(argument.type);
                }

                free(statement.func_decl.arguments);
            }
            
            if (statement.func_decl.with_body) {
                cwr_func_body_expression_destroy(statement.func_decl.func_body);
            }
            
            break;
        case cwr_statement_func_call_type:
            cwr_statement_destroy_func_call(statement.func_call);
            break;
        case cwr_statement_for_loop_type:
            cwr_statement_destroy_for_loop(statement.for_loop);
            break;
        case cwr_statement_return_type:
            cwr_expression_destroy(statement.ret.value);

            if (statement.ret.with_body) {
                cwr_expression_destroy_body(statement.ret.body);
            }
            break;
        case cwr_statement_var_decl_type:
            cwr_var_decl_destroy(statement.var_decl);
            break;
        case cwr_statement_assign_type:
            cwr_expression_destroy(statement.assign.identifier);
            cwr_expression_destroy(statement.assign.value);
            break;
    }
}

static void cwr_nodes_list_destroy(cwr_nodes_list nodes_list) {
    for (size_t i = 0;i < nodes_list.count;i++) {
        cwr_statement statement = nodes_list.statements[i];

        cwr_statement_destroy(statement);
    }

    if (nodes_list.statements != NULL) {
        free(nodes_list.statements);
    }
}

#endif //CWR_NODE_H