#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

enum Token {
    MUL,
    DIV,
    ADD,
    SUB,
    NUM,
    SPACE,
    BRACKET,
    INVALID,
};

struct Token_Pair {
    enum Token t;
    char* sym;
    size_t sym_len;
};

struct Node {
    struct Node* right;
    struct Node* left;
    struct Token_Pair op;
};

struct QNode {
    struct QNode* next;
    struct QNode* prev;
    char* sym;
    size_t sym_len;
    int count;
};

struct Node_queue {
    struct QNode* front;
    struct QNode* back;
};

struct QNode* insert_into_queue(struct Node_queue* queue, char* sym, size_t sym_len, int count) {
    struct QNode* new_node = malloc(sizeof(struct QNode));
    if (!new_node) {
        fprintf(stderr, "Critical error during allocation\n");
        return NULL;
    }
    new_node->sym = sym;
    new_node->sym_len = sym_len;
    new_node->count = count;
    new_node->next = queue->back;
    new_node->prev = NULL;
    if (queue->back) {
        queue->back->prev = new_node;
    } else {
        queue->front = new_node;
    }
    queue->back = new_node;
    return queue->back;
}

struct QNode* pop_from_queue(struct Node_queue* queue) {
    if (!queue->front) return NULL;
    struct QNode* temp = queue->front;
    struct Node* node = create_node((struct Token_Pair){.t = INVALID, .sym = temp->sym, .sym_len = temp->sym_len});
    queue->front = temp->prev;
    if (queue->front) {
        queue->front->next = NULL;
    } else {
        queue->back = NULL;
    }
    free(temp);
    return node;
}

// Helper function to check if character is an operator
static inline int is_operator(char c) {
    return c == '*' || c == '/' || c == '+' || c == '-';
}

char* sub(char* src, size_t pos, size_t len) {
    char* ret = malloc(len);

    if (!ret) {
        fprintf(stderr, "Critical error during allocation\n");
        return NULL;
    }

    for (size_t i = 0; i < len; ++i) {
        ret[i] = src[i + pos];
    }

    return ret;
}

// Helper function to create a token pair
static struct Token_Pair create_token(enum Token type, char* symbol, size_t sym_len) {
    struct Token_Pair pair = {type, symbol, sym_len};
    return pair;
}

struct Token_Pair* tokenize(char* str, size_t str_len, size_t* out_token_count) {
    if (!str || !out_token_count) return NULL;
    
    // Pre-allocate maximum possible tokens
    struct Token_Pair* pairs = malloc(sizeof(struct Token_Pair) * str_len);
    if (!pairs) return NULL;
    
    size_t index = 0;
    
    size_t concurrent_sym_len = 0;

    int minus_read = 0;

    for (size_t i = 0; i < str_len; ++i) {
        if (isspace(str[i])) {
            if (concurrent_sym_len > 0) {
                pairs[index++] = create_token(NUM, 
                    sub(str, i - concurrent_sym_len, concurrent_sym_len), 
                    concurrent_sym_len);
            }
            concurrent_sym_len = 0;
            minus_read = 0;
            continue;
        }
        if (isdigit(str[i])) {
            printf("%c\n", str[i]);
            concurrent_sym_len++;
            if (i == str_len - 1) {
                pairs[index++] = create_token(NUM, 
                    sub(str, i - concurrent_sym_len + 1, concurrent_sym_len), 
                    concurrent_sym_len);
            }
        } else if (is_operator(str[i])) {
            enum Token type;
            switch (str[i]) {
                case '*': type = MUL; break;
                case '/': type = DIV; break;
                case '+': type = ADD; break;
                case '-': type = SUB; break;
                default: type = INVALID;
            }
            char* op_str = malloc(1);
            op_str[0] = str[i];
            pairs[index++] = create_token(type, op_str, 1);
            concurrent_sym_len = 0;
        } else if (str[i] == '-') {
            minus_read++;

            if (minus_read > 1) {
                fprintf(stderr, "Critical error during parsing! Consecutive minus signs are not allowed");
                return NULL;
            }
        } else {
            // Invalid character encountered
            free(pairs);
            concurrent_sym_len = 0;
            return NULL;
        }
    }
    
    // Resize to actual token count
    struct Token_Pair* resized = realloc(pairs, sizeof(struct Token_Pair) * index);
    *out_token_count = index;
    return resized ? resized : pairs;
}

void free_ast(struct Node* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

struct Node* create_node(struct Token_Pair op) {
    struct Node* node = malloc(sizeof(struct Node));
    if (!node) return NULL;
    node->op = op;
    node->left = NULL;
    node->right = NULL;
    return node;
}

int traverse_ast(struct Node* ast_node, int depth) {
	if(!ast_node->right && !ast_node->left) {
	   printf("Token found\nDepth: %d, Symbol: %s\n", depth, ast_node->op.sym);
	   return atoi(ast_node->op.sym);
	}

	int left_val = traverse_ast(ast_node->left, depth + 1);
	int right_val = traverse_ast(ast_node->right, depth + 1);

    int comb_val = 0;

    switch (ast_node->op.t) {
        case MUL: 
            comb_val = left_val * right_val;
            break;
        case DIV:
            comb_val = left_val / right_val;
            break;
        case ADD:
            comb_val = left_val + right_val;
            break;
        case SUB:
            comb_val = left_val - right_val;
            break;
        default:
            fprintf(stderr, "Critical error! Mismatched tokens during tree traversal\n");
            break;
    }

    return comb_val;
}

void print_ast(struct Node* ast_node, int depth, int left) {
    



    if (!ast_node) return;
    
    if (left) {
        printf("( %s )", ast_node->op.sym);
    }
    
}


struct Node* parse_to_ast(struct Token_Pair* tokens, size_t tokens_len, size_t* pos) {
    if (!tokens || *pos >= tokens_len) return NULL;

    struct Node* root = NULL;
    struct Node* current = NULL;

    while (*pos < tokens_len) {
        struct Token_Pair token = tokens[*pos];
        
        switch (token.t) {
            case NUM: {
                struct Node* num_node = create_node(token);
                if (!root) {
                    root = num_node;
                    current = root;
                } else if (!current->right) {
                    current->right = num_node;
                }
                (*pos)++;
                break;
            }
            
            case ADD:
            case SUB: {
                struct Node* op_node = create_node(token);
                if (!root) {
                    root = op_node;
                } else {
                    op_node->left = root;
                    root = op_node;
                }
                current = root;
                (*pos)++;
                break;
            }
            
            case MUL:
            case DIV: {
                struct Node* op_node = create_node(token);
                (*pos)++;
                if (*pos >= tokens_len || tokens[*pos].t != NUM) {
                    free_ast(op_node);
                    free_ast(root);
                    return NULL;
                }
                op_node->left = current->right;
                op_node->right = create_node(tokens[*pos]);
                current->right = op_node;
                (*pos)++;
                break;
            }
            
            default:
                free_ast(root);
                return NULL;
        }
    }
    
    return root;
}

int main(int argc, char** argv) {
    char* input = "25 - 4 * 5 + 6 * 2 + 1";
    size_t token_count = 0;
    struct Token_Pair* tokens = tokenize(input, strlen(input), &token_count);

    for (size_t i = 0; i < token_count; ++i) {
        printf("Token symbol: %s\n", tokens[i].sym);
    }

    size_t pos = 0;
    struct Node* ast = parse_to_ast(tokens, token_count, &pos);
    
    int result = traverse_ast(ast, 0);	
    printf("Result of the calculation: %d\n", result);
    assert(result == 18);
    
    // Clean up
    free_ast(ast);
    free(tokens);
    
    return 0;
}
