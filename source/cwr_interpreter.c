#include <stdlib.h>
#include <stdio.h>
#include <cwr_interpreter.h>
#include <cwr_scope.h>
#include <cwr_string.h>

typedef struct cwr_interpreter {
    cwr_parser_result result;
} cwr_interpreter;

static cwr_value* printf_array_function(cwr_func_call_context context) {
    cwr_value** arguments = context.arguments;
    cwr_value* content = arguments[0];
    printf("%s\n", content->array.characters);

    cwr_func_call_context_destroy(context);
    return cwr_value_create_void();
}

static cwr_value* printf_character_function(cwr_func_call_context context) {
    cwr_value** arguments = context.arguments;
    cwr_value* content = arguments[0];
    printf("%c\n", content->character);

    cwr_func_call_context_destroy(context);
    return cwr_value_create_void();
}

static cwr_value* printf_number_function(cwr_func_call_context context) {
    cwr_value** arguments = context.arguments;
    cwr_value* content = arguments[0];
    printf("%f\n", cwr_value_as_float(content));

    cwr_func_call_context_destroy(context);
    return cwr_value_create_void();
}

cwr_interpreter* cwr_intepreter_create(cwr_parser_result result) {
    cwr_interpreter* interpreter = malloc(sizeof(cwr_interpreter));
    if (interpreter == NULL) {
        return NULL;
    }

    interpreter->result = result;
    return interpreter;
}

cwr_value* cwr_interpreter_evaluate_entry_point(cwr_interpreter_result result, cwr_interpreter_error* error) {
    cwr_instance* entry_point = cwr_scope_get_by_name(result.context.functions, CWR_SCOPE_GLOBAL_SCOPE, CWR_INTERPRETER_ENTRY_POINT_FUNC);
    if (entry_point == NULL || entry_point->type != cwr_instance_function_type) {
        cwr_interpreter_error_throw(error,
             cwr_interpreter_error_entry_point_not_found_type, "Entry point function with'" CWR_INTERPRETER_ENTRY_POINT_FUNC "' name not found", (cwr_location) {0});
        return NULL;
    }

    cwr_func_call_context context = (cwr_func_call_context) {
        .context = result.context,
        .arguments = NULL,
        .count = 0,
        .location = (cwr_location) {0}
    };

    return cwr_intepreter_evaluate_func(entry_point->function, context, error);
}

cwr_interpreter_result cwr_intepreter_interpret(cwr_interpreter* interpreter, cwr_interpreter_error* error) {
    cwr_program_context context = cwr_program_context_default();

    for (size_t i = 0;i < interpreter->result.nodes_list.count;i++) {
        cwr_statement statement = interpreter->result.nodes_list.statements[i];

        switch (statement.type) {
            case cwr_statement_func_decl_type: {
                char* name = statement.func_decl.name;
                cwr_instance instance = (cwr_instance) {
                    .type = cwr_instance_function_type,
                    .root = CWR_SCOPE_GLOBAL_SCOPE,
                    .identifier = statement.func_decl.identifier,
                    .name = name,
                    .function = (cwr_function_instance) {
                        .type = cwr_function_instance_user_type,
                        .arguments = statement.func_decl.arguments,
                        .body = statement.func_decl.func_body
                    }
                };

                if (strcmp(instance.name, "printf") == 0) {
                    switch (instance.function.arguments[0].type.value_type) {
                        case cwr_value_pointer_type:
                            instance.function.type = cwr_function_instance_bind_type;
                            instance.function.bind = printf_array_function;
                            break;
                        case cwr_value_float_type:
                            instance.function.type = cwr_function_instance_bind_type;
                            instance.function.bind = printf_number_function;
                            break;
                        case cwr_value_character_type:
                            instance.function.type = cwr_function_instance_bind_type;
                            instance.function.bind = printf_character_function;
                            break;
                    }
                }

                if (!cwr_scope_add(context.functions, instance)) {
                    cwr_instance_destroy(instance);
                    cwr_interpreter_error_throw_not_enough_memory(error, statement.location);
                }
                
                break;
            }
        }

        if (error->is_failed) {
            break;
        }
    }

    return (cwr_interpreter_result) {
        .source = interpreter->result,
        .context = context
    };
}

cwr_value* cwr_intepreter_evaluate_stat(cwr_program_context program_context, cwr_statement statement, cwr_func_body_expression* root, cwr_interpreter_error* error) {
    switch (statement.type) {
        case cwr_statement_func_call_type: {
            return cwr_intepreter_evaluate_stat_func_call(program_context, statement.func_call, root, statement.location, error);
        }
        case cwr_statement_if_type: {
            cwr_value* condition = cwr_intepreter_evaluate_expr(program_context, statement.if_stat.condition, root, error);
            if (error->is_failed) {
                return condition;
            }

            int result = cwr_value_as_integer(condition);
            cwr_value_runtime_destroy(condition);

            if (result) {
                return cwr_intepreter_evaluate_expr_body(program_context, statement.if_stat.body, root, error);
            }

            return NULL;
        }
        case cwr_statement_for_loop_type: {
            cwr_func_body_expression body = (cwr_func_body_expression) {
                .root = root,
                .statements = statement.for_loop.body.statements,
                .count = statement.for_loop.body.count
            };
            cwr_func_body_expression* body_pointer = &body;

            if (statement.for_loop.with_variable) {
                char* name = statement.for_loop.variable.name;
                cwr_value* value = cwr_intepreter_evaluate_expr(program_context, statement.for_loop.variable.value, root, error);

                if (error->is_failed) {
                    return value;
                }

                value->references_count++;
                cwr_instance variable = (cwr_instance) {
                    .type = cwr_instance_variable_type,
                    .root = body_pointer,
                    .identifier = statement.for_loop.variable.identifier,
                    .name = name,
                    .variable = (cwr_variable_instance) {
                        .type = statement.for_loop.variable.value_type,
                        .value = value
                    }
                };

                if (!cwr_scope_add(program_context.variables, variable)) {
                    cwr_instance_destroy(variable);
                    cwr_interpreter_error_throw_not_enough_memory(error, statement.location);
                    return value;
                }
            }

            while (true) {
                if (statement.for_loop.with_condition) {
                    cwr_value* condition = cwr_intepreter_evaluate_expr(program_context, statement.for_loop.condition, body_pointer, error);
                    if (error->is_failed) {
                        return condition;
                    }

                    int result = cwr_value_as_integer(condition);
                    cwr_value_runtime_destroy(condition);

                    if (!result) {
                        break;
                    }
                }

                if (statement.for_loop.with_statement) {
                    cwr_value* result = cwr_intepreter_evaluate_stat(program_context, *statement.for_loop.statement, body_pointer, error);
                    if (error->is_failed) {
                        return result;
                    }

                    cwr_value_runtime_destroy(result);
                }

                cwr_value* result = cwr_intepreter_evaluate_expr_body(program_context, statement.for_loop.body, body_pointer, error);
                if (error->is_failed) {
                    return result;
                }

                if (result != NULL) {
                    return result;
                }
            }
            
            cwr_scope_root_destroy(program_context.variables, body_pointer);
            return NULL;
        }
        case cwr_statement_var_decl_type: {
            char* name = statement.var_decl.name;
            cwr_value* value = cwr_intepreter_evaluate_expr(program_context, statement.var_decl.value, root, error);

            if (error->is_failed) {
                return NULL;
            }

            cwr_value_add_reference(value);
            cwr_instance variable = (cwr_instance) {
                .type = cwr_instance_variable_type,
                .root = root,
                .identifier = statement.var_decl.identifier,
                .name = name,
                .variable = (cwr_variable_instance) {
                    .type = statement.var_decl.value_type,
                    .value = value
                }
            };

            if (!cwr_scope_add(program_context.variables, variable)) {
                cwr_instance_destroy(variable);
                cwr_interpreter_error_throw_not_enough_memory(error, statement.location);
                return cwr_value_create_void();
            }

            return NULL;
        }
        case cwr_statement_assign_type: {
            cwr_value* value = cwr_intepreter_evaluate_expr(program_context, statement.assign.value, root, error);
            if (error->is_failed) {
                return NULL;
            }

            cwr_value_add_reference(value);
            if (!statement.assign.is_dereference) {
                cwr_instance* instance = cwr_scope_get(program_context.variables, root, statement.assign.identifier.var.identifier);
                cwr_value* variable_value = instance->variable.value;

                cwr_value_remove_reference(variable_value);
                instance->variable.value = value;
                return value;
            }

            cwr_value* identifier = cwr_intepreter_evaluate_expr(program_context, statement.assign.identifier, root, error);
            if (error->is_failed) {
                cwr_value_runtime_destroy(value);
                return NULL;
            }

            value->references_count += --identifier->references_count;
            cwr_value_set(identifier, value);
            free(value);
            return identifier;
        }
        case cwr_statement_return_type:
            cwr_value* ret_value = cwr_intepreter_evaluate_expr(program_context, statement.ret.value, root, error);
            if (error->is_failed) {
                return ret_value;
            }

            if (statement.ret.with_body) {
                cwr_value* body_result = cwr_intepreter_evaluate_expr_body(program_context, statement.ret.body, root, error);
                if (error->is_failed) {
                    cwr_value_runtime_destroy(ret_value);
                    return ret_value;
                }

                if (body_result != NULL) {
                    cwr_value_runtime_destroy(body_result);
                }
            }

            return ret_value;
    }

    return NULL;
}

cwr_value* cwr_intepreter_evaluate_stat_func_call(cwr_program_context program_context, cwr_func_call_statement statement, cwr_func_body_expression* root, cwr_location location, cwr_interpreter_error* error) {
    cwr_instance* function = cwr_scope_get(program_context.functions, root, statement.identifier);
    cwr_value** arguments = malloc(statement.count * sizeof(cwr_value*));

    if (arguments == NULL) {
        cwr_interpreter_error_throw_not_enough_memory(error, location);
        return NULL;
    }

    size_t count = 0;
    cwr_func_call_context context = (cwr_func_call_context) {
        .context = program_context,
        .arguments = arguments,
        .count = 0,
        .location = location
    };

    for (size_t i = 0;i < statement.count;i++) {
        cwr_value* argument = cwr_intepreter_evaluate_expr(program_context, statement.arguments[i], root, error);
        if (error->is_failed) {
            cwr_func_call_context_destroy(context);
            return NULL;
        }

        arguments[count] = argument;
        context.count = ++count;
    }

    return cwr_intepreter_evaluate_func(function->function, context, error);
}

cwr_value* cwr_intepreter_evaluate_func(cwr_function_instance instance, cwr_func_call_context context, cwr_interpreter_error* error) {
    if (instance.type == cwr_function_instance_bind_type) {
        return instance.bind(context);
    }

    cwr_program_context program_context = context.context;
    size_t count = context.count;

    // New scope
    cwr_func_body_expression body = (cwr_func_body_expression) {
        .root = instance.body.root,
        .statements = instance.body.statements,
        .count = instance.body.count
    };
    cwr_func_body_expression* body_pointer = &body;

    for (size_t i = 0;i < count;i++) {
        cwr_value* value = context.arguments[i];
        cwr_argument argument = instance.arguments[i];
        char* name = argument.name;

        cwr_value_add_reference(value);
        cwr_instance variable = (cwr_instance) {
            .type = cwr_instance_variable_type,
            .root = body_pointer,
            .identifier = argument.identifier,
            .name = name,
            .variable = (cwr_variable_instance) {
                .type = argument.type,
                .value = value
            }
        };

        if (!cwr_scope_add(program_context.variables, variable)) {
            cwr_instance_destroy(variable);
            cwr_scope_root_destroy(program_context.variables, body_pointer);
            cwr_interpreter_error_throw_not_enough_memory(error, context.location);
            return cwr_value_create_void();
        }
    }

    free(context.arguments);

    cwr_value* result = cwr_intepreter_evaluate_expr_body(program_context, body, body_pointer, error);
    cwr_scope_root_destroy(program_context.variables, body_pointer);
    return result;
}

cwr_value* cwr_intepreter_evaluate_expr(cwr_program_context program_context, cwr_expression expression, cwr_func_body_expression* root, cwr_interpreter_error* error) {
    switch (expression.type) {
        case cwr_expression_array_type: {
            switch (expression.array.type.value_type) {
                case cwr_value_character_type: {
                    char* characters = malloc(expression.array.count * sizeof(char));

                    if (characters == NULL) {
                        cwr_interpreter_error_throw_not_enough_memory(error, expression.location);
                        return NULL;
                    }

                    for (size_t i = 0;i < expression.array.count;i++) {
                        characters[i] = expression.array.elements[i].character.value;
                    }
                    
                    cwr_array_value array = (cwr_array_value) {
                        .type = expression.array.type.value_type,
                        .capacity = expression.array.count,
                        .is_reference = false,
                        .characters = characters
                    };

                    return cwr_value_create_array(array);
                }
            }
        }
        case cwr_expression_unary_type:
            cwr_value* value = cwr_intepreter_evaluate_expr(program_context, expression.unary.child[0], root, error);
            if (error->is_failed) {
                return value;
            }

            cwr_value* result = value;
            switch (expression.unary.type) {
                case cwr_binary_operator_minus_type:
                    if (result->type == cwr_value_float_type) {
                        result = cwr_value_create_float(-cwr_value_as_float(value)); 
                    }
                    else {
                        result = cwr_value_create_integer(-cwr_value_as_integer(value)); 
                    }

                    break;
                case cwr_binary_operator_negation_type:
                    if (result->type == cwr_value_float_type) {
                        result = cwr_value_create_float(!cwr_value_as_float(value)); 
                    }
                    else {
                        result = cwr_value_create_integer(!cwr_value_as_integer(value)); 
                    }

                    break;
                case cwr_binary_operator_reference_type:
                    result = cwr_value_reference(value);
                    break;
                case cwr_binary_operator_dereference_type:
                    result = cwr_value_dereference(value);
                    break;
            }

            cwr_value_runtime_destroy(value);
            if (result == NULL) {
                cwr_interpreter_error_throw_not_enough_memory(error, expression.location);
            }

            return result;
        case cwr_expression_float_type:
            cwr_value* number = cwr_value_create_float(expression.float_n.value);
            if (number == NULL) {
                cwr_interpreter_error_throw_not_enough_memory(error, expression.location);
            }

            return number;
        case cwr_expression_integer_type:
            cwr_value* integer_n = cwr_value_create_integer(expression.integer_n.value);
            if (integer_n == NULL) {
                cwr_interpreter_error_throw_not_enough_memory(error, expression.location);
            }

            return integer_n;
        case cwr_expression_character_type:
            return cwr_value_create_character(expression.character.value);
        case cwr_expression_var_type:
            return cwr_scope_get(program_context.variables, root, expression.var.identifier)->variable.value;
        case cwr_expression_func_call_type:
            return cwr_intepreter_evaluate_stat_func_call(program_context, expression.func_call, root, expression.location, error);
        case cwr_expression_binary_type: {
            cwr_value* left = cwr_intepreter_evaluate_expr(program_context, expression.binary.children[0], root, error);
            if (error->is_failed) {
                return NULL;
            }

            cwr_value* right = cwr_intepreter_evaluate_expr(program_context, expression.binary.children[1], root, error);
            if (error->is_failed) {
                return NULL;
            }

            float left_number = cwr_value_as_float(left);
            float right_number = cwr_value_as_float(right);
            float result;
            switch (expression.binary.type) {
                case cwr_binary_operator_plus_type:
                    result = left_number + right_number;
                    break;
                case cwr_binary_operator_minus_type:
                    result = left_number - right_number;
                    break;
                case cwr_binary_operator_multiplicative_type:
                    result = left_number * right_number;
                    break;
                case cwr_binary_operator_division_type:
                    result = left_number / right_number;
                    break;
                case cwr_binary_operator_not_equals_type:
                    result = left_number != right_number;
                    break;
                case cwr_binary_operator_equals_type:
                    result = left_number == right_number;
                    break;
                case cwr_binary_operator_greater_than_type:
                    result = left_number > right_number;
                    break;
                case cwr_binary_operator_less_than_type:
                    result = left_number < right_number;
                    break;
                case cwr_binary_operator_greater_equals_than_type:
                    result = left_number >= right_number;
                    break;
                case cwr_binary_operator_less_equals_than_type:
                    result = left_number <= right_number;
                    break;
            }

            cwr_value* value_result;

            if (left->type == cwr_value_float_type || right->type == cwr_value_float_type) {
                value_result = cwr_value_create_float(result);
            }
            else {
                value_result = cwr_value_create_integer(result);
            }
            
            cwr_value_runtime_destroy(left);
            cwr_value_runtime_destroy(right);

            if (value_result == NULL) {
                cwr_interpreter_error_throw_not_enough_memory(error, expression.location);
                return NULL;
            }

            return value_result;
        }
        case cwr_expression_array_element_type: {
            cwr_value* value = cwr_intepreter_evaluate_expr(program_context, expression.array_element.children[0], root, error);
            if (error->is_failed) {
                return NULL;
            }

            cwr_value* index = cwr_intepreter_evaluate_expr(program_context, expression.array_element.children[1], root, error);
            if (error->is_failed) {
                cwr_value_runtime_destroy(value);
                return NULL;
            }

            int target = cwr_value_as_float(index);
            if (target < 0) {
                cwr_value_runtime_destroy(value);
                cwr_value_runtime_destroy(index);
                cwr_interpreter_error_throw(error, cwr_interpreter_error_negative_index_type, "Index must be non-negative", expression.location);
                return NULL;
            }

            if (target > value->array.capacity - 1) {
                cwr_value_runtime_destroy(value);
                cwr_value_runtime_destroy(index);
                cwr_interpreter_error_throw(error, cwr_interpreter_error_index_out_of_range_type, "Index out of range", expression.location);
                return NULL;
            }

            cwr_value* element = cwr_value_at(value->array, target);
            cwr_value_runtime_destroy(value);
            cwr_value_runtime_destroy(index);
            return element;
        }
    }

    return cwr_value_create_void();
}

cwr_value* cwr_intepreter_evaluate_expr_body(cwr_program_context program_context, cwr_func_body_expression expression, cwr_func_body_expression* root, cwr_interpreter_error* error) {
    for (size_t i = 0;i < expression.count;i++) {
        cwr_statement statement = expression.statements[i];
        cwr_value* result = cwr_intepreter_evaluate_stat(program_context, statement, root, error);
        if (error->is_failed || statement.type == cwr_statement_return_type) {
            return result;
        }

        if (statement.is_block && result != NULL) {
            return result;
        }

        if (result != NULL) {
            cwr_value_runtime_destroy(result);
        }
    }

    return NULL;
}

void cwr_interpreter_result_destroy(cwr_interpreter_result result) {
    cwr_program_context_destroy(result.context);
}

void cwr_intepreter_destroy(cwr_interpreter* interpreter) {
    free(interpreter);
}