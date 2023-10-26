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

void vatest(char v0, char v1, short v2, int v3, long v4, char *v5)
{
    printf("%p\n%p\n%p\n%p\n%p\n%p.\n", &v0, &v1, &v2, &v3, &v4, &v5);
}

int main() {

    printf("%d.\n", sizeof(int));
    vatest(0, 1, 2, 3, 4, (char *)5);
    return 1;
    // char j = 0;
    // int i = 0;
    // for(;i > -100; i--, j--) {
    //     printf("[%d %x] [%d %x]\n", i, i, j, j);
    // }

    char j = 0x80;
    printf("%d %02x", j, j);

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
