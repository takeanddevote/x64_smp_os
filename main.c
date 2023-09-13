#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT 100

typedef struct {
    int left_operand;
    char operator;
    int right_operand;
} AST;

void lexer(const char *input, char **tokens, int *count) {
    char *token = strtok(strdup(input), " ");
    while (token != NULL) {
        tokens[*count] = token;
        (*count)++;
        token = strtok(NULL, " ");
    }
}

AST parser(char **tokens) {
    AST ast;
    ast.left_operand = atoi(tokens[0]);
    ast.operator = tokens[1][0];
    ast.right_operand = atoi(tokens[2]);
    return ast;
}

int interpreter(AST ast) {
    switch (ast.operator) {
        case '+':
            return ast.left_operand + ast.right_operand;
        case '-':
            return ast.left_operand - ast.right_operand;
        default:
            printf("Unknown operator: %c\n", ast.operator);
            exit(1);
    }
}

int main() {
    while (1) {
        printf("calc> ");
        char input[MAX_INPUT];
        fgets(input, MAX_INPUT, stdin);
        
        // Remove newline character
        input[strcspn(input, "\n")] = '\0';

        char *tokens[3];
        int count = 0;

        lexer(input, tokens, &count);
        if (count != 3) {
            printf("Error: Invalid input.\n");
            continue;
        }

        AST ast = parser(tokens);
        int result = interpreter(ast);
        printf("%d\n", result);

        // Free tokens
        for (int i = 0; i < count; i++) {
            free(tokens[i]);
        }
    }
    return 0;
}
