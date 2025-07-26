#include <stdio.h>
#include <cwr_interpreter.h>
#include <cwr_parser.h>
#include <cwr_preprocessor.h>
#include <cwr_lexer.h>

int main() {
    char* source;
    FILE* target = fopen("script.cwr", "rb");
    if (!target) {
        perror("Failed to open file");
        return 1;
    }

    fseek(target, 0, SEEK_END);
    long file_size = ftell(target);
    if (file_size < 0) {
        perror("Failed to get file size");
        fclose(target);
        return 1;
    }

    if (file_size == 0) {
        fclose(target);
        source = strdup("");
    } else {
        fseek(target, 0, SEEK_SET);
        source = malloc(file_size + 1);

        if (!source) {
            fclose(target);
            perror("Memory allocation failed");
            return 1;
        }
        
        size_t bytes_read = fread(source, 1, file_size, target);
        if (bytes_read != (size_t) file_size) {
            source[bytes_read] = '\0';
        } else {
            source[file_size] = '\0';
        }
    }

    fclose(target);

    cwr_lexer_configuration configuration = cwr_lexer_configuration_default();
    cwr_lexer* lexer = cwr_lexer_create("console", source, &configuration);
    cwr_tokens_list tokens_list = cwr_lexer_tokenize(lexer);

    cwr_preprocessor* preprocessor = cwr_preprocessor_create(tokens_list);
    cwr_preprocessor_result pr_result = cwr_preprocessor_run(preprocessor);

    tokens_list = pr_result.tokens_list;
    if (pr_result.is_failed) {
        printf("Preprocessor error");
        printf(pr_result.error.message);

        cwr_tokens_list_destroy(tokens_list);
        cwr_lexer_destroy(lexer);
        return -1;
    }

    free(source);

    for (size_t i = 0;i < tokens_list.count;i++) {
        printf(tokens_list.tokens[i].value);
    }

    cwr_parser* parser = cwr_parser_create(tokens_list);
    cwr_parser_result statements = cwr_parser_parse(parser);

    if (statements.is_failed) {
        printf("Parser error");
        printf(statements.error.message);

        cwr_tokens_list_destroy(tokens_list);
        cwr_lexer_destroy(lexer);
        cwr_preprocessor_destroy(preprocessor);
        
        cwr_parser_result_destroy(statements);
        cwr_parser_destroy(parser);
        return -1;
    }

    cwr_interpreter* interpreter = cwr_intepreter_create(statements);
    cwr_interpreter_error error = (cwr_interpreter_error) {
        .is_failed = false
    };
    cwr_interpreter_result result = cwr_intepreter_interpret(interpreter, &error);

    if (error.is_failed) {
        printf("Interpreter error");
    }
    else {  
        cwr_value* value = cwr_interpreter_evaluate_entry_point(result, &error);
        if (error.is_failed) {
            printf("Entry point error");
        }
        else { 
            cwr_value_instance_destroy(value);
        }
    }

    cwr_interpreter_result_destroy(result);
    cwr_tokens_list_destroy(tokens_list);
    cwr_parser_result_destroy(statements);

    cwr_lexer_destroy(lexer);
    cwr_preprocessor_destroy(preprocessor);
    cwr_parser_destroy(parser);
    cwr_intepreter_destroy(interpreter);
    return 0;
}