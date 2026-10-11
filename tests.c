#define TESTING

#include "tree_parser.h"
#include <stdio.h>
#include <assert.h>

void free_data(struct Node* ast, struct Token_Pair* tokens) {
    free_ast(ast);
    free(tokens);
}

int run_test(char* input, int expected_result) {
    size_t token_count = 0;
    struct Token_Pair* tokens = tokenize(input, strlen(input), &token_count);
    size_t pos = 0;
    struct Node* ast = parse_to_ast(tokens, token_count, &pos);
    int result = traverse_ast(ast, 0);
    printf("Input: %s\nResult of the calculation: %d\nExpected: %d\n", input, result, expected_result);
    free_data(ast, tokens);
    return result;
}

void test_1() {
    char* input = "5 + 4";
    int expected_result = 9;
    int result = run_test(input, expected_result);
    assert(result == expected_result);
}

void test_2() {
    char* input = "5 - 4";
    int expected_result = 1;
    int result = run_test(input, expected_result);
    assert(result == expected_result);
}

void test_3() {
    char* input = "25 - 4 * 5";
    int expected_result = 5;
    int result = run_test(input, expected_result);
    assert(result == expected_result);
}

void test_4() {
    char* input = "4 * 5 - 25";
    int expected_result = -5;
    int result = run_test(input, expected_result);
    assert(result == expected_result);
}


void test_5() {
    char* input = "25 - 4 * (5 + 6) * 2 + 2";
    int expected_result = -61;
    int result = run_test(input, expected_result);
    assert(result == expected_result);
}

int main(void) {
    printf("Running a basic test suite\n\n");
    //test_1();
    //test_2();
    //test_3();
    test_4();
    //test_5();
    return 0;
}
