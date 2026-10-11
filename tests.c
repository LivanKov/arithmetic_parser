#define TESTING

#include "tree_parser.h"
#include <stdio.h>


int main(void) {
    char* input = "25 - 4 * (5 + 6) * 2 + 2";
    size_t token_count = 0;
    struct Token_Pair* tokens = tokenize(input, strlen(input), &token_count);

    for (size_t i = 0; i < token_count; ++i) {
        printf("Token symbol: %s, Privilege: %d\n", tokens[i].sym, tokens[i].privilege);
    }

    size_t pos = 0;

    struct Node* ast = parse_to_ast(tokens, token_count, &pos);
    int result = traverse_ast(ast, 0);
    printf("Result of the calculation: %d\n", result);
    print_ast(ast);

    // Clean up
    free_ast(ast);
    free(tokens);

    return 0;
}
