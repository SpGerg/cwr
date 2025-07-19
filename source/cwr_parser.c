#include <string.h>
#include <cwr_parser.h>
#include <cwr_lexer.h>
#include <cwr_string.h>

typedef struct cwr_parser {
    cwr_func_body_expression* root;
    cwr_token* tokens;
    size_t count;
    cwr_statement* statements;
    size_t capacity;
    cwr_parser_function* functions;
    size_t functions_capacity;
    cwr_parser_variable* variables;
    size_t variables_capacity;
    size_t position;
    cwr_parser_error error;
    bool is_failed;
} cwr_parser;

static cwr_expression_type_value cwr_parser_parse_multidimensional_array(cwr_parser* parser, cwr_expression_type_value type) {
    if (cwr_parser_match(parser, cwr_token_asterisk_type)) {
        cwr_expression_type_value* target = malloc(sizeof(cwr_expression_type_value));
        target->value_type = type.value_type;
        target->identifier = type.identifier;
        target->name = type.name;
        target->target_type = type.target_type;
        type = (cwr_expression_type_value) {
            .value_type = cwr_value_pointer_type,
            .name = NULL,
            .target_type = target
        };

        return cwr_parser_parse_multidimensional_array(parser, type);
    }

    return type;
}

cwr_parser* cwr_parser_create(cwr_tokens_list tokens_list) {
    cwr_parser* parser = malloc(sizeof(cwr_parser));

    if (parser == NULL) {
        return NULL;
    }

    parser->capacity = 0;
    parser->statements = NULL;
    parser->tokens = tokens_list.tokens;
    parser->count = tokens_list.count;
    return parser;
}

cwr_parser_result cwr_parser_parse(cwr_parser* parser) {
    parser->root = NULL;
    parser->capacity = 0;
    parser->statements = NULL;
    parser->position = 0;
    parser->is_failed = false;
    parser->functions_capacity = 0;
    parser->functions = NULL;
    parser->variables_capacity = 0;
    parser->variables = NULL;

    while (!cwr_parser_ended(parser)) {
        cwr_statement declaration = cwr_parser_parse_declaration(parser);
        if (parser->is_failed) {
            break;
        }

        cwr_parser_add_statement(parser, declaration);
    }

    free(parser->variables);
    return (cwr_parser_result) {
        .nodes_list = (cwr_nodes_list) {
            .statements = parser->statements,
            .count = parser->capacity,
        },
        .functions = parser->functions,
        .functions_count = parser->functions_capacity,
        .error = parser->error,
        .is_failed = parser->is_failed
    };
}

cwr_statement_type cwr_parser_get_statement(cwr_parser* parser) {
    cwr_token current = cwr_parser_current(parser);

    switch (current.type) {
        case cwr_token_for_type:
            return cwr_statement_for_loop_type;
        case cwr_token_if_type:
            return cwr_statement_if_type;
        case cwr_token_word_type:
            if (cwr_parser_peek(parser, 1, cwr_token_left_par_type)) {
                return cwr_statement_func_call_type;
            }

            if (cwr_parser_peek(parser, 1, cwr_token_equals_type)) {
                return cwr_statement_assign_type;
            }

            return cwr_statement_func_decl_type;
        case cwr_token_asterisk_type:
            return cwr_statement_assign_type;
        case cwr_token_struct_type:
            break;
        case cwr_token_return_type:
            return cwr_statement_return_type;
        default:
            // Because of 'struct' type of variable
            if (cwr_parser_peek(parser, 2, cwr_token_equals_type) || cwr_parser_peek(parser, 3, cwr_token_equals_type)) {
                return cwr_statement_var_decl_type;
            }

            return cwr_statement_func_decl_type;
    }
}

bool cwr_parser_add_statement(cwr_parser* parser, cwr_statement node) {
    cwr_statement* buffer = realloc(parser->statements, (parser->capacity + 1) * sizeof(cwr_statement));

    if (buffer == NULL) {
        return false;
    }

    parser->statements = buffer;
    parser->statements[parser->capacity++] = node;
    return true;
}

bool cwr_parser_add_function(cwr_parser* parser, cwr_parser_function function) {
    cwr_parser_function* buffer = realloc(parser->functions, (parser->functions_capacity + 1) * sizeof(cwr_parser_function));

    if (buffer == NULL) {
        return false;
    }

    parser->functions = buffer;
    parser->functions[parser->functions_capacity++] = function;
    return true;
}

bool cwr_parser_add_variable(cwr_parser* parser, cwr_parser_variable variable) {
    cwr_parser_variable* buffer = realloc(parser->variables, (parser->variables_capacity + 1) * sizeof(cwr_parser_variable));

    if (buffer == NULL) {
        return false;
    }

    parser->variables = buffer;
    parser->variables[parser->variables_capacity++] = variable;
    return true;
}

void cwr_parser_clear_scope(cwr_parser* parser, cwr_func_body_expression* root) {
    for (int i = parser->variables_capacity - 1;i >= 0;i--) {
        cwr_parser_variable variable = parser->variables[i];
        if (variable.root != root) {
            continue;
        }

        parser->variables = realloc(parser->variables, --parser->variables_capacity * sizeof(cwr_parser_variable));
    }
}

cwr_statement cwr_parser_parse_declaration(cwr_parser* parser) {
    cwr_statement_type statement_type = cwr_parser_get_statement(parser);
    cwr_statement statement;

    switch (statement_type) {
        case cwr_statement_func_decl_type: {
            cwr_expression_type_value type = cwr_parser_parse_type(parser);
            CWR_PARSER_FAILED_AND_RETURN(parser, cwr_statement);

            cwr_func_decl_statement decl = cwr_parser_parse_function_declaration(parser, type);
            statement = (cwr_statement) {
                .type = statement_type,
                .func_decl = decl
            };
        }
        case cwr_statement_struct_decl_type:
            break;
        default:
            cwr_parser_throw_error(parser, cwr_parser_error_unknown_statement_type, "Unknown statement", cwr_parser_current(parser).location);
    }

    if (parser->is_failed) {
        return (cwr_statement) {};
    }

    cwr_parser_match(parser, cwr_token_semicolon_type);
    return statement;
}

cwr_statement cwr_parser_parse_statement(cwr_parser* parser) {
    cwr_statement_type current = cwr_parser_get_statement(parser);
    cwr_location location = cwr_parser_current(parser).location;
    cwr_statement statement;

    switch (current) {
        case cwr_statement_func_call_type: {
            cwr_func_call_statement func_call = cwr_parser_parse_function_call(parser);
            statement = (cwr_statement) {
                .type = cwr_statement_func_call_type,
                .location = location,
                .func_call = func_call
            };

            break;
        }
        case cwr_statement_if_type:
            cwr_if_statement if_stat = cwr_parser_parse_if(parser);
            statement = (cwr_statement) {
                .type = cwr_statement_if_type,
                .location = location,
                .if_stat = if_stat
            };

            break;
        case cwr_statement_for_loop_type: {
            cwr_for_loop_statement for_loop = cwr_parser_parse_for_loop(parser);
            statement = (cwr_statement) {
                .type = cwr_statement_for_loop_type,
                .location = location,
                .for_loop = for_loop
            };

            break;
        }
        case cwr_statement_return_type: {
            cwr_return_statement return_statement = cwr_parser_parse_return(parser, cwr_expression_type_value_create_from_type(cwr_value_void_type));
            statement = (cwr_statement) {
                .type = cwr_statement_return_type,
                .location = location,
                .ret = return_statement
            };

            break;
        }
        case cwr_statement_var_decl_type: {
            cwr_expression_type_value type = cwr_parser_parse_type(parser);
            CWR_PARSER_FAILED_AND_RETURN(parser, cwr_statement);

            cwr_var_decl_statement var_decl = cwr_parser_parse_variable_declaration(parser, type);
            statement = (cwr_statement) {
                .type = current,
                .var_decl = var_decl
            };

            break;
        }
        case cwr_statement_assign_type: {
            cwr_assign_statement assign = cwr_parser_parse_assign(parser);
            statement = (cwr_statement) {
                .type = current,
                .assign = assign
            };

            break;
        }
        default:
            cwr_parser_throw_error(parser, cwr_parser_error_unknown_statement_type, "Unknown statement", location);
            break;
    }

    if (parser->is_failed) {
        return (cwr_statement) {};
    }

    if (statement.type != cwr_statement_for_loop_type && statement.type != cwr_statement_if_type) {   
        cwr_parser_except(parser, cwr_token_semicolon_type);
        if (parser->is_failed) {
            cwr_statement_destroy(statement);
            return (cwr_statement) {};
        }
    }

    return statement;
}

cwr_assign_statement cwr_parser_parse_assign(cwr_parser* parser) {
    bool is_dereference = cwr_parser_current(parser).type == cwr_token_asterisk_type;
    cwr_expression identifier = cwr_parser_parse_value(parser);
    CWR_PARSER_FAILED_AND_RETURN(parser, cwr_assign_statement);

    cwr_parser_except(parser, cwr_token_equals_type);
    if (parser->is_failed) {
        cwr_expression_destroy(identifier);
        return (cwr_assign_statement) {};
    }

    cwr_expression value = cwr_parser_parse_binary(parser);
    if (parser->is_failed) {
        cwr_expression_destroy(identifier);
        return (cwr_assign_statement) {};
    }

    if (!cwr_expression_type_value_equals(identifier.value_type, value.value_type)) {
        cwr_expression_destroy(identifier);
        cwr_expression_destroy(value);
        cwr_parser_throw_error(parser, cwr_parser_error_incorrect_type_type, "Incorrect type", identifier.location);
        return (cwr_assign_statement) {};
    }

    return (cwr_assign_statement) {
        .identifier = identifier,
        .value = value,
        .is_dereference = is_dereference
    };
}

cwr_var_decl_statement cwr_parser_parse_variable_declaration(cwr_parser* parser, cwr_expression_type_value type) {
    if (type.value_type == cwr_value_void_type) {
        cwr_expression_type_value_destroy(type);
        cwr_parser_throw_error(parser, cwr_parser_error_incorrect_type_type, "Incorrect type", cwr_parser_current(parser).location);
        return (cwr_var_decl_statement) {};
    }

    cwr_token name = cwr_parser_except(parser, cwr_token_word_type);
    CWR_PARSER_FAILED_AND_RETURN(parser, cwr_var_decl_statement);

    char* name_copy = cwr_string_duplicate(name.value);
    if (name_copy == NULL) {
        cwr_expression_type_value_destroy(type);
        cwr_parser_throw_low_memory_error(parser, name.location);
        return (cwr_var_decl_statement) {};
    }

    cwr_parser_except(parser, cwr_token_equals_type);
    if (parser->is_failed) {
        free(name_copy);
        cwr_expression_type_value_destroy(type);
        return (cwr_var_decl_statement) {};
    }

    cwr_expression value = cwr_parser_parse_binary(parser);
    if (parser->is_failed) {
        free(name_copy);
        cwr_expression_type_value_destroy(type);
        return (cwr_var_decl_statement) {};
    }

    if (value.value_type.value_type == cwr_value_void_type || !cwr_expression_type_value_equals(type, value.value_type)) {
        free(name_copy);
        cwr_expression_destroy(value);
        cwr_expression_type_value_destroy(type);
        cwr_parser_throw_error(parser, cwr_parser_error_incorrect_type_type, "Incorrect type", cwr_parser_current(parser).location);
        return (cwr_var_decl_statement) {};
    }

    cwr_parser_variable variable = (cwr_parser_variable) {
        .type = type,
        .identifier = parser->variables_capacity,
        .root = parser->root,
        .name = name_copy,
        .static_value = NULL
    };

    if (!cwr_parser_add_variable(parser, variable)) {
        free(name_copy);
        cwr_expression_destroy(value);
        cwr_expression_type_value_destroy(type);
        cwr_parser_throw_low_memory_error(parser, cwr_parser_current(parser).location);
        return (cwr_var_decl_statement) {};
    }

    return (cwr_var_decl_statement) {
        .identifier = variable.identifier,
        .name = name_copy,
        .value_type = type,
        .value = value
    };
}

cwr_func_decl_statement cwr_parser_parse_function_declaration(cwr_parser* parser, cwr_expression_type_value type) {
    cwr_token name = cwr_parser_except(parser, cwr_token_word_type);
    CWR_PARSER_FAILED_AND_RETURN(parser, cwr_func_decl_statement);

    char* name_copy = cwr_string_duplicate(name.value);
    if (name_copy == NULL) {
        cwr_expression_type_value_destroy(type);
        cwr_parser_throw_low_memory_error(parser, name.location);
        return (cwr_func_decl_statement) {};
    }

    cwr_parser_except(parser, cwr_token_left_par_type);
    if (parser->is_failed) {
        free(name_copy);
        cwr_expression_type_value_destroy(type);
        return (cwr_func_decl_statement) {};
    }

    cwr_location location = cwr_parser_current(parser).location;

    cwr_func_body_expression body = (cwr_func_body_expression) {
        .root = parser->root,
        .statements = NULL,
        .count = 0
    };
    cwr_func_body_expression* body_pointer = &body;

    cwr_func_decl_statement func_decl = (cwr_func_decl_statement) {
        .identifier = parser->functions_capacity,
        .name = name_copy,
        .arguments = NULL,
        .count = 0,
        .with_body = false,
        .return_type = type
    };

    while (!cwr_parser_match(parser, cwr_token_right_par_type)) {
        if (cwr_parser_ended(parser)) {
            cwr_func_decl_destroy(func_decl);
            cwr_parser_throw_error(parser, cwr_parser_error_except_token_type, "Except )", location);
            return (cwr_func_decl_statement) {};
        }

        cwr_expression_type_value argument_type = cwr_parser_parse_type(parser);
        if (parser->is_failed) {
            cwr_func_decl_destroy(func_decl);
            return (cwr_func_decl_statement) {};
        }

        cwr_token argument_name = cwr_parser_except(parser, cwr_token_word_type);
        if (parser->is_failed) {
            cwr_func_decl_destroy(func_decl);
            cwr_expression_type_value_destroy(argument_type);
            return (cwr_func_decl_statement) {};
        }

        cwr_argument* buffer = realloc(func_decl.arguments, (func_decl.count + 1) * sizeof(cwr_argument));
        if (buffer == NULL) {
            cwr_func_decl_destroy(func_decl);
            cwr_expression_type_value_destroy(argument_type);
            return (cwr_func_decl_statement) {};
        }

        cwr_parser_variable variable = (cwr_parser_variable) {
            .name = argument_name.value,
            .identifier = parser->variables_capacity,
            .root = body_pointer,
            .type = argument_type
        };
        if (!cwr_parser_add_variable(parser, variable)) {
            cwr_func_decl_destroy(func_decl);
            cwr_expression_type_value_destroy(argument_type);
            return (cwr_func_decl_statement) {};
        }

        char* copy = cwr_string_duplicate(argument_name.value);
        if (copy == NULL) {
            cwr_func_decl_destroy(func_decl);
            cwr_expression_type_value_destroy(argument_type);
            return (cwr_func_decl_statement) {};
        }

        func_decl.arguments = buffer;
        func_decl.arguments[func_decl.count++] = (cwr_argument) {
            .identifier = variable.identifier,
            .name = copy,
            .type = argument_type
        };
        cwr_parser_match(parser, cwr_token_comma_type);
    }
    
    cwr_parser_function function = (cwr_parser_function) {
        .identifier = parser->functions_capacity,
        .name = name_copy,
        .arguments = func_decl.arguments,
        .count = func_decl.count,
        .return_type = type
    };
    if (!cwr_parser_add_function(parser, function)) {
        cwr_func_decl_destroy(func_decl);
        cwr_parser_throw_low_memory_error(parser, name.location);
        return (cwr_func_decl_statement) {};
    }

    bool with_body = false;
    if (!cwr_parser_match(parser, cwr_token_semicolon_type)) {       
        cwr_parser_parse_function_body_by_created(parser, body_pointer);
        if (parser->is_failed) {
            cwr_func_decl_destroy(func_decl);
            return (cwr_func_decl_statement) {};
        }

        with_body = true;
    }
    else {
        cwr_parser_clear_scope(parser, body_pointer);
    }

    func_decl.func_body = body;
    func_decl.with_body = with_body;
    return func_decl;
}

cwr_if_statement cwr_parser_parse_if(cwr_parser* parser) {
    cwr_parser_match(parser, cwr_token_if_type);
    cwr_expression condition = cwr_parser_parse_binary(parser);
    CWR_PARSER_FAILED_AND_RETURN(parser, cwr_if_statement);

    cwr_func_body_expression body = cwr_parser_parse_function_body(parser);
    if (parser->is_failed) {
        cwr_expression_destroy(condition);
        return  (cwr_if_statement) {};
    }

    return (cwr_if_statement) {
        .condition = condition,
        .body = body
    };
}

cwr_for_loop_statement cwr_parser_parse_for_loop(cwr_parser* parser) {
    cwr_parser_match(parser, cwr_token_for_type);
    cwr_parser_except(parser, cwr_token_left_par_type);
    CWR_PARSER_FAILED_AND_RETURN(parser, cwr_for_loop_statement);

    cwr_for_loop_statement for_stat = (cwr_for_loop_statement) {
        .with_variable = false,
        .with_condition = false,
        .with_statement = false,
        .with_body = false
    };

    if (!cwr_parser_match(parser, cwr_token_semicolon_type)) {
        cwr_expression_type_value type = cwr_parser_parse_type(parser);
        CWR_PARSER_FAILED_AND_RETURN(parser, cwr_for_loop_statement);

        for_stat.variable = cwr_parser_parse_variable_declaration(parser, type);
        if (parser->is_failed) {
            cwr_expression_type_value_destroy(type);
            return (cwr_for_loop_statement) {};
        }

        for_stat.with_variable = true;

        cwr_parser_except(parser, cwr_token_semicolon_type);
        if (parser->is_failed) {
            cwr_statement_destroy_for_loop(for_stat);
            return (cwr_for_loop_statement) {};
        }

        cwr_parser_variable variable = (cwr_parser_variable) {
            .identifier = for_stat.variable.identifier,
            .name = for_stat.variable.name,
            .type = for_stat.variable.value_type
        };

        if (!cwr_parser_add_variable(parser, variable)) {
            cwr_statement_destroy_for_loop(for_stat);
            cwr_parser_throw_low_memory_error(parser, cwr_parser_current(parser).location);
            return (cwr_for_loop_statement) {};
        }
    }

    if (!cwr_parser_match(parser, cwr_token_semicolon_type)) {
        for_stat.condition = cwr_parser_parse_binary(parser);
        if (parser->is_failed) {
            cwr_statement_destroy_for_loop(for_stat);
            return (cwr_for_loop_statement) {};
        }

        for_stat.with_condition = true;

        if (for_stat.condition.value_type.value_type != cwr_value_integer_type) {
            cwr_statement_destroy_for_loop(for_stat);
            return (cwr_for_loop_statement) {};
        }

        cwr_parser_except(parser, cwr_token_semicolon_type);
        if (parser->is_failed) {
            cwr_statement_destroy_for_loop(for_stat);
            return (cwr_for_loop_statement) {};
        }
    }

    if (!cwr_parser_match(parser, cwr_token_right_par_type)) {
        for_stat.statement = malloc(sizeof(cwr_statement));
        if (for_stat.statement == NULL) {
            cwr_statement_destroy_for_loop(for_stat);
            return (cwr_for_loop_statement) {};
        }

        *for_stat.statement = cwr_parser_parse_statement(parser);
        if (parser->is_failed) {
            cwr_statement_destroy_for_loop(for_stat);
            free(for_stat.statement);
            return (cwr_for_loop_statement) {};
        }

        for_stat.with_statement = true;

        cwr_parser_except(parser, cwr_token_right_par_type);
        if (parser->is_failed) {
            cwr_statement_destroy_for_loop(for_stat);
            return (cwr_for_loop_statement) {};
        }
    }

    cwr_func_body_expression body = cwr_parser_parse_function_body(parser);
    if (parser->is_failed) {
        cwr_statement_destroy_for_loop(for_stat);
        return (cwr_for_loop_statement) {};
    }

    for_stat.with_body = true;
    for_stat.body = body;
    return for_stat;
}

cwr_func_call_statement cwr_parser_parse_function_call(cwr_parser* parser) {
    cwr_token name = cwr_parser_except(parser, cwr_token_word_type);
    CWR_PARSER_FAILED_AND_RETURN(parser, cwr_func_call_statement);

    char* name_copy = cwr_string_duplicate(name.value);
    if (name_copy == NULL) {
        cwr_parser_throw_low_memory_error(parser, name.location);
        return (cwr_func_call_statement) {};
    }

    cwr_parser_except(parser, cwr_token_left_par_type);
    if (parser->is_failed) {
        free(name_copy);
        return (cwr_func_call_statement) {};
    }

    cwr_func_call_statement func_call = (cwr_func_call_statement) {
        .identifier = 0,
        .name = name_copy,
        .arguments = NULL,
        .count = 0
    };
    cwr_location location = cwr_parser_current(parser).location;

    while (!cwr_parser_match(parser, cwr_token_right_par_type)) {
        if (cwr_parser_ended(parser)) {
            cwr_statement_destroy_func_call(func_call);
            cwr_parser_throw_error(parser, cwr_parser_error_except_token_type, "Except )", location);
            return (cwr_func_call_statement) {};
        }
        
        cwr_expression value = cwr_parser_parse_binary(parser);
        if (parser->is_failed) {
            cwr_statement_destroy_func_call(func_call);
            return (cwr_func_call_statement) {};
        }

        cwr_expression* buffer = realloc(func_call.arguments, (func_call.count + 1) * sizeof(cwr_expression));
        if (buffer == NULL) {
            cwr_statement_destroy_func_call(func_call);
            cwr_expression_destroy(value);
            return (cwr_func_call_statement) {};
        }

        func_call.arguments = buffer;
        func_call.arguments[func_call.count++] = value;
        cwr_parser_match(parser, cwr_token_comma_type);
    }

    cwr_parser_function target;
    if (!cwr_parser_get_function(parser, name_copy, func_call.arguments, func_call.count, &target)) {
        cwr_statement_destroy_func_call(func_call);
        cwr_parser_throw_error(parser, cwr_parser_error_unknown_function_type, "Unknown function", location);
        return (cwr_func_call_statement) {};
    }

    func_call.identifier = target.identifier;
    func_call.return_type = target.return_type;
    return func_call;
}

cwr_return_statement cwr_parser_parse_return(cwr_parser* parser, cwr_expression_type_value type) {
    cwr_parser_match(parser, cwr_token_return_type);

    cwr_expression value = cwr_parser_parse_binary(parser);
    CWR_PARSER_FAILED_AND_RETURN(parser, cwr_return_statement);

    cwr_func_body_expression body;
    bool with_body = cwr_parser_match(parser, cwr_token_left_curly_type);

    if (with_body) {
        body = cwr_parser_parse_function_body(parser);
        if (parser->is_failed) {
            cwr_expression_destroy(value);
            return (cwr_return_statement) {};
        }
    }

    return (cwr_return_statement) {
        .value = value,
        .body = body,
        .with_body = with_body
    };
}

void cwr_parser_parse_function_body_by_created(cwr_parser* parser, cwr_func_body_expression* body) {
    cwr_parser_match(parser, cwr_token_left_curly_type);

    cwr_location location = cwr_parser_current(parser).location;
    parser->root = body;

    while (!cwr_parser_match(parser, cwr_token_right_curly_type)) {
        if (cwr_parser_ended(parser)) {
            cwr_parser_throw_error(parser, cwr_parser_error_except_token_type, "Except )", location);
            return;
        }

        cwr_statement statement = cwr_parser_parse_statement(parser);
        if (parser->is_failed) {
            cwr_func_body_expression_destroy(*body);
            return ;
        }

        cwr_statement* buffer = realloc(body->statements, (body->count + 1) * sizeof(cwr_statement));
        if (buffer == NULL) {
            cwr_func_body_expression_destroy(*body);
            cwr_statement_destroy(statement);
            return;
        }

        body->statements = buffer;
        body->statements[body->count++] = statement;
    }

    cwr_parser_clear_scope(parser, body);
    parser->root = body->root;
}

cwr_func_body_expression cwr_parser_parse_function_body(cwr_parser* parser) {
    cwr_func_body_expression body = (cwr_func_body_expression) {
        .root = parser->root,
        .statements = NULL,
        .count = 0
    };

    cwr_parser_parse_function_body_by_created(parser, &body);
    return body;
}

cwr_expression_type_value cwr_parser_parse_type(cwr_parser* parser) {
    cwr_token current = cwr_parser_current(parser);
    cwr_parser_skip(parser);

    char* name = NULL;
    size_t identifier = -1;
    cwr_value_type type;

    switch (current.type) {
        case cwr_token_void_type:
            type = cwr_value_void_type;
            break;
        case cwr_token_int_type:
            type = cwr_value_integer_type;
            break;
        case cwr_token_float_type:
            type = cwr_value_float_type;
            break;
        case cwr_token_char_word_type:
            type = cwr_value_character_type;
            break;
        default:
            cwr_parser_throw_error(parser, cwr_parser_error_except_token_type, "Except type", current.location);
            return (cwr_expression_type_value) {};
    }

    cwr_expression_type_value result = (cwr_expression_type_value) {
        .value_type = type,
        .identifier = identifier,
        .name = name,
        .target_type = NULL
    };
    return cwr_parser_parse_multidimensional_array(parser, result);
}

cwr_expression cwr_parser_parse_binary(cwr_parser* parser) {
    cwr_expression left = cwr_parser_parse_multiplicative(parser);
    
    while (true) {
        cwr_binary_operator_type type = cwr_binary_operator_type_from_token(cwr_parser_current(parser).type);

        if (type == cwr_binary_operator_none_type) {
            break;
        }

        if (cwr_parser_peek(parser, 1, cwr_token_equals_type)) {
            switch (type) {
                case cwr_binary_operator_equals_type:
                    type = cwr_binary_operator_equals_type;
                    break;
                case cwr_binary_operator_greater_than_type:
                    type = cwr_binary_operator_greater_equals_than_type;
                    break;
                case cwr_binary_operator_less_than_type:
                    type = cwr_binary_operator_less_equals_than_type;
                    break;
            }

            cwr_parser_skip(parser);
        }
        else {
            if (type == cwr_binary_operator_equals_type) {
                break;
            }
        }
        
        cwr_parser_skip(parser);
        cwr_expression right = cwr_parser_parse_binary(parser);

        if (right.value_type.value_type != cwr_value_float_type && right.value_type.value_type != cwr_value_integer_type) {
            cwr_expression_destroy(left);
            cwr_parser_throw_error(parser, cwr_parser_error_incorrect_type_type, "Incorrect type", cwr_parser_current(parser).location);
            return (cwr_expression) {};
        }

        cwr_expression* children = malloc(sizeof(cwr_expression) * 2);
        if (children == NULL) {
            cwr_expression_destroy(left);
            cwr_expression_destroy(right);
            cwr_parser_throw_low_memory_error(parser, left.location);
            return (cwr_expression) {};
        }

        children[0] = left;
        children[1] = right;
        if (parser->is_failed) {
            free(children);
            cwr_expression_destroy(left);
            cwr_expression_destroy(right);
            return (cwr_expression) {};
        }

        left = (cwr_expression) {
            .type = cwr_expression_binary_type,
            .value_type = left.value_type,
            .binary = (cwr_binary_expression) {
                .type = type,
                .children = children
            }
        };

        continue;
    }
    
    return left;
}

cwr_expression cwr_parser_parse_multiplicative(cwr_parser* parser) {
    cwr_expression left = cwr_parser_parse_array_element(parser);
    
    while (true) {
        cwr_binary_operator_type type = cwr_binary_operator_type_from_token(cwr_parser_current(parser).type);

        if (type != cwr_binary_operator_multiplicative_type && type != cwr_binary_operator_division_type) {
            break;
        }
        
        if (type != cwr_binary_operator_none_type) {
            cwr_parser_skip(parser);

            cwr_expression right = cwr_parser_parse_binary(parser);
            if (right.value_type.value_type != cwr_value_float_type && right.value_type.value_type != cwr_value_integer_type) {
                cwr_expression_destroy(left);
                cwr_parser_throw_error(parser, cwr_parser_error_incorrect_type_type, "Incorrect type", cwr_parser_current(parser).location);
                return (cwr_expression) {};
            }

            cwr_expression* children = malloc(sizeof(cwr_expression) * 2);
            if (children == NULL) {
                cwr_expression_destroy(left);
                cwr_expression_destroy(right);
                cwr_parser_throw_low_memory_error(parser, left.location);
                return (cwr_expression) {};
            }

            children[0] = left;
            children[1] = right;
            if (parser->is_failed) {
                free(children);
                cwr_expression_destroy(left);
                cwr_expression_destroy(right);
                return (cwr_expression) {};
            }

            left = (cwr_expression) {
                .type = cwr_expression_binary_type,
                .value_type = left.value_type,
                .binary = (cwr_binary_expression) {
                    .type = type,
                    .children = children
                }
            };

            continue;
        }

        break;
    }
    
    return left;
}

cwr_expression cwr_parser_parse_array_element(cwr_parser* parser) {
    cwr_expression value = cwr_parser_parse_unary(parser);

    if (cwr_parser_match(parser, cwr_token_left_square_type)) {
        cwr_expression index = cwr_parser_parse_binary(parser);
        if (parser->is_failed) {
            cwr_expression_destroy(value);
            return (cwr_expression) {};
        }

        cwr_parser_except(parser, cwr_token_right_square_type);
        if (parser->is_failed) {
            cwr_expression_destroy(value);
            cwr_expression_destroy(index);
            return (cwr_expression) {};
        }

        cwr_expression* children = malloc(2 * sizeof(cwr_expression));
        if (children == NULL) {
            cwr_expression_destroy(value);
            cwr_expression_destroy(index);
            cwr_parser_throw_low_memory_error(parser, value.location);
            return (cwr_expression) {};
        }

        children[0] = value;
        children[1] = index;
        return (cwr_expression) {
            .type = cwr_expression_array_element_type,
            .value_type = cwr_expression_type_value_create_from_type(cwr_value_character_type),
            .array_element = (cwr_array_element_expression) {
                .children = children
            }
        };
    }

    return value;
}

cwr_expression cwr_parser_parse_unary(cwr_parser* parser) {
    if (cwr_parser_match(parser, cwr_token_minus_type)) {
        cwr_expression* child = malloc(sizeof(cwr_expression));
        if (child == NULL) {
            cwr_parser_throw_low_memory_error(parser, cwr_parser_current(parser).location);
            return (cwr_expression) {};
        }

        cwr_expression value = cwr_parser_parse_value(parser);
        if (parser->is_failed) {
            free(child);
            return (cwr_expression) {};
        }

        *child = value;
        return (cwr_expression) {
            .type = cwr_expression_unary_type,
            .value_type = value.value_type,
            .unary = (cwr_unary_expression) {
                .type = cwr_binary_operator_minus_type,
                .child = child
            }
        };
    }

    return cwr_parser_parse_value(parser);
}

cwr_expression cwr_parser_parse_value(cwr_parser* parser) {
    cwr_token current = cwr_parser_current(parser);
    cwr_parser_skip(parser);

    switch (current.type)
    {
        case cwr_token_word_type: {
            if (cwr_parser_current(parser).type == cwr_token_left_par_type) {
                parser->position--;

                cwr_func_call_statement func_call = cwr_parser_parse_function_call(parser);
                return (cwr_expression) {
                    .type = cwr_expression_func_call_type,
                    .value_type = func_call.return_type,
                    .func_call = func_call
                };
            }

            cwr_parser_variable variable;
            if (!cwr_parser_get_variable(parser, current.value, &variable)) {
                cwr_parser_throw_error(parser, cwr_parser_error_unknown_variable_type, "Unknown variable", current.location);
                return (cwr_expression) {};
            }

            char* name = cwr_string_duplicate(current.value);
            if (name == NULL) {
                cwr_parser_throw_low_memory_error(parser, current.location);
                return (cwr_expression) {};
            }

            return (cwr_expression) {
                .type = cwr_expression_var_type,
                .value_type = variable.type,
                .var = (cwr_var_expression) {
                    .identifier = variable.identifier,
                    .name = name
                }
            };
        }
        case cwr_token_number_type: {
            double value = atof(current.value);

            // Check if number int or float
            if (value - ((int) value) > 0) {
                return (cwr_expression) {
                    .type = cwr_expression_float_type,
                    .value_type = cwr_expression_type_value_create_from_type(cwr_value_float_type),
                    .float_n = (cwr_float_expression) {
                        .value = value
                    }
                };
            }

            return (cwr_expression) {
                .type = cwr_expression_integer_type,
                .value_type = cwr_expression_type_value_create_from_type(cwr_value_integer_type),
                .integer_n = (cwr_integer_expression) {
                    .value = value
                }
            };
        }
        case cwr_token_asterisk_type:
            cwr_expression* target = malloc(sizeof(cwr_expression));
            if (target == NULL) {
                cwr_parser_throw_low_memory_error(parser, current.location);
                return (cwr_expression) {};
            }

            cwr_expression value = cwr_parser_parse_binary(parser);
            if (parser->is_failed) {
                free(target);
                return value;
            }

            *target = value;
            return (cwr_expression) {
                .type = cwr_expression_dereference_type,
                .value_type = *value.value_type.target_type,
                .dereference = (cwr_dereference_expression) {
                    .value = target
                }
            };
        case cwr_token_ampersand_type:
            cwr_expression* reference_target = malloc(sizeof(cwr_expression));
            if (reference_target == NULL) {
                cwr_parser_throw_low_memory_error(parser, current.location);
                return (cwr_expression) {};
            }

            cwr_expression reference_value = cwr_parser_parse_binary(parser);
            if (parser->is_failed) {
                free(reference_target);
                return reference_value;
            }

            *reference_target = reference_value;
            return (cwr_expression) {
                .type = cwr_expression_reference_type,
                .value_type = (cwr_expression_type_value) {
                    .value_type = cwr_value_pointer_type,
                    .identifier = -1,
                    .name = NULL,
                    .target_type = &reference_target->value_type
                },
                .reference = (cwr_reference_expression) {
                    .value = reference_target
                }
            };
        case cwr_token_string_type: {
            size_t count = strlen(current.value) + 1;
            cwr_expression* content = malloc(count * sizeof(cwr_expression) + sizeof(cwr_expression));

            if (content == NULL) {
                cwr_parser_throw_low_memory_error(parser, current.location);
                return (cwr_expression) {};
            }

            for (size_t i = 0;i < count;i++) {
                content[i] = (cwr_expression) {
                    .type = cwr_expression_character_type,
                    .character = (cwr_character_expression) {
                        .value = current.value[i]
                    }
                };
            }
            content[count] = (cwr_expression) {
                .type = cwr_expression_character_type,
                .character = (cwr_character_expression) {
                    .value = '\0'
                }
            };

            cwr_expression_type_value* array_type = malloc(sizeof(cwr_expression_type_value));
            *array_type = cwr_expression_type_value_create_from_type(cwr_value_character_type);
            return (cwr_expression) {
                .type = cwr_expression_array_type,
                .value_type = cwr_expression_type_value_create_array(array_type),
                .array = (cwr_array_expression) {
                    .type = *array_type,
                    .elements = content,
                    .count = count
                }
            };
        }
        case cwr_token_char_type:
            return (cwr_expression) {
                .type = cwr_expression_character_type,
                .value_type = cwr_expression_type_value_create_from_type(cwr_value_character_type),
                .character = (cwr_character_expression) {
                    .value = current.value[0]
                }
            };
        case cwr_token_left_par_type:
            cwr_expression binary = cwr_parser_parse_binary(parser);
            cwr_parser_except(parser, cwr_token_right_par_type);
            if (parser->is_failed) {
                cwr_expression_destroy(binary);
                return (cwr_expression) {};
            }

            return binary;
        default:
            cwr_parser_throw_error(parser, cwr_parser_error_except_value_type, "Except value", current.location);
            return (cwr_expression) {};
    }
}

bool cwr_parser_get_function(cwr_parser* parser, char* name, cwr_expression* argument, size_t count, cwr_parser_function* function) {
    for (size_t i = 0;i < parser->functions_capacity;i++) {
        cwr_parser_function member = parser->functions[i];

        if (strcmp(member.name, name) != 0) {
            continue;
        }

        if (member.count != count) {
            continue;
        }

        bool breaked = false;
        for (size_t i = 0;i < count;i++) {
            if (cwr_expression_type_value_equals(member.arguments[i].type, argument[i].value_type)) {
                continue;
            }

            breaked = true;
            break;
        }

        if (breaked) {
            continue;
        }

        *function = member;
        return true;
    }

    return false;
}

bool cwr_parser_get_variable(cwr_parser* parser, char* name, cwr_parser_variable* variable) {
    for (size_t i = 0;i < parser->variables_capacity;i++) {
        cwr_parser_variable member = parser->variables[i];

        if (strcmp(member.name, name) != 0) {
            continue;
        }

        if (!cwr_func_body_can_access(member.root, parser->root)) {
            continue;
        }

        *variable = member;
        return true;
    }

    return false;
}

cwr_token cwr_parser_except(cwr_parser* parser, cwr_token_type token_type) {
    cwr_token current = cwr_parser_current(parser);

    if (current.type == token_type) {
        cwr_parser_skip(parser);
        return current;
    }

    cwr_parser_throw_error(parser, cwr_parser_error_except_token_type, "Except token", current.location);
    return current;
}

bool cwr_parser_peek(cwr_parser* parser, size_t offset, cwr_token_type token_type) {
    if (parser->position + offset > parser->count - 1) {
        return false;
    }

    return parser->tokens[parser->position + offset].type == token_type;
}

bool cwr_parser_match(cwr_parser* parser, cwr_token_type token_type) {
    if (cwr_parser_current(parser).type == token_type) {
        cwr_parser_skip(parser);
        return true;
    }

    return false;
}

cwr_token cwr_parser_current(cwr_parser* parser) {
    if (cwr_parser_ended(parser)) {
        return parser->tokens[parser->count - 1];
    }

    return parser->tokens[parser->position];
}

bool cwr_parser_ended(cwr_parser* parser) {
    return parser->position > parser->count - 1;
}

void cwr_parser_skip(cwr_parser* parser) {
    parser->position++;
}

void cwr_parser_throw_low_memory_error(cwr_parser* parser, cwr_location location) {
    cwr_parser_throw_error(parser, cwr_parser_error_not_enough_memory_type, "Not enough memory", location);
}

void cwr_parser_throw_error(cwr_parser* parser, cwr_parser_error_type type, char* message, cwr_location location) {
    parser->error = (cwr_parser_error) {
        .error_type = type,
        .message = message,
        .is_free_message = false,
        .location = location
    };
    parser->is_failed = true;
}

void cwr_parser_result_destroy(cwr_parser_result parser_result) {
    cwr_nodes_list_destroy(parser_result.nodes_list);

    if (parser_result.global_variables_count > 0) {
        free(parser_result.global_variables);
    }

    if (parser_result.functions_count > 0) {
        free(parser_result.functions);
    }

    if (parser_result.is_failed) {
        if (parser_result.error.is_free_message) {
            free(parser_result.error.message);
        }
    }
}

void cwr_parser_destroy(cwr_parser* parser) {
    free(parser);
}