#ifndef PARSER
#define PARSER

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>

enum Token {
    MUL,
    DIV,
    ADD,
    SUB,
    NUM,
    SPACE,
    L_BRACKET,
    R_BRACKET,
    INVALID,
};

struct Token_Pair {
    enum Token t;
    char* sym;
    size_t sym_len;
    int privilege;
};

struct Node {
    struct Node* right;
    struct Node* left;
    struct Token_Pair op;
};

struct QNode {
    struct QNode* next;
    struct QNode* prev;
    struct Node* node;
    int depth;
    int count;
};

struct Node_queue {
    struct QNode* front;
    struct QNode* back;
};

struct Node_queue* create_node_queue();

struct QNode* insert_into_queue(
    struct Node_queue* queue,
    struct Node* node,
    int count,
    int depth
);

struct QNode* pop_from_queue(struct Node_queue* queue);

bool is_queue_empty(struct Node_queue* queue);

void print_ast(struct Node* ast_node);

int is_operator(char c);

int is_bracket(char c);

char* sub(char* src, size_t pos, size_t len);

struct Token_Pair create_token(enum Token type, char* symbol, size_t sym_len);

struct Token_Pair create_token_priv(enum Token type, char* symbol, size_t sym_len, int privilege);

struct Token_Pair* tokenize(char* str, size_t str_len, size_t* out_token_count);

void free_ast(struct Node* node);

struct Node* create_node(struct Token_Pair op);

int traverse_ast(struct Node* ast_node, int depth);

struct Node* parse_to_ast(struct Token_Pair* tokens, size_t tokens_len, size_t* pos);

#endif
