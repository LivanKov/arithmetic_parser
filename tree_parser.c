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

struct Node_queue* create_node_queue() {
    struct Node_queue* new_queue = (struct Node_queue*) malloc(sizeof(struct Node_queue));
    
    if (!new_queue) {
        fprintf(stderr, "Critical error during allocation\n");
        return NULL;
    }

    new_queue->front = NULL;
    new_queue->back = NULL;

    return new_queue;
} 

struct QNode* insert_into_queue(
    struct Node_queue* queue, 
    struct Node* node,
    int count,
    int depth
) {
    struct QNode* new_node = malloc(sizeof(struct QNode));
    
    if (!new_node) {
        fprintf(stderr, "Critical error during allocation\n");
        return NULL;
    }
    
    new_node->node = node;
    new_node->count = count;
    new_node->depth = depth;
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
    queue->front = temp->prev;
    if (queue->front) {
        queue->front->next = NULL;
    } else {
        queue->back = NULL;
    }
    return temp;
}

bool is_queue_empty(struct Node_queue* queue) {
    return !queue->back && !queue->front; 
}

// Refactor later
void print_ast(struct Node* ast_node) {
    if (!ast_node) {
        printf("(empty)\n");
        return;
    }

    struct Node_queue* queue = create_node_queue();

    if (!queue) { 
        fprintf(stderr, "Critical error during queue allocation\n");
        exit(2);
    }

    struct Level {
        struct Node** nodes;
        size_t size;
    };

    insert_into_queue(queue, ast_node, 1, 1);

    struct Level* levels = NULL;
    size_t levels_size = 0;
    size_t max_label_len = 0;

    while (!is_queue_empty(queue)) {
        struct QNode* current = pop_from_queue(queue);
        size_t depth_index = (size_t) current->depth - 1;
        size_t count_index = (size_t) current->count - 1;

        if (depth_index >= levels_size) {
            size_t old_size = levels_size;
            struct Level* resized = realloc(levels, (depth_index + 1) * sizeof(struct Level));

            if (!resized) {
                fprintf(stderr, "Critical error during reallocation\n");
                exit(2);
            }

            levels = resized;
            levels_size = depth_index + 1;

            for (size_t i = old_size; i < levels_size; ++i) {
                levels[i].nodes = NULL;
                levels[i].size = 0;
            }
        }

        if (count_index >= levels[depth_index].size) {
            size_t old_size = levels[depth_index].size;
            size_t new_size = count_index + 1;
            struct Node** resized = realloc(levels[depth_index].nodes, new_size * sizeof(struct Node*));

            if (!resized) {
                fprintf(stderr, "Critical error during reallocation\n");
                exit(2);
            }

            levels[depth_index].nodes = resized;
            levels[depth_index].size = new_size;
            memset(levels[depth_index].nodes + old_size, 0, (new_size - old_size) * sizeof(struct Node*));
        }

        levels[depth_index].nodes[count_index] = current->node;

        size_t label_len = strlen(current->node->op.sym);
        if (label_len > max_label_len) {
            max_label_len = label_len;
        }

        if (current->node->left) {
            insert_into_queue(queue, current->node->left, current->count * 2 - 1, current->depth + 1);
        }

        if (current->node->right) {
            insert_into_queue(queue, current->node->right, current->count * 2, current->depth + 1);
        }

        free(current);
    }

    size_t leaf_slots = (size_t) 1 << (levels_size - 1);
    size_t cell_width = max_label_len + 4;
    size_t canvas_width = leaf_slots * cell_width;
    size_t canvas_height = levels_size * 2 - 1;
    char** text_rows = malloc(canvas_height * sizeof(char*));
    const char*** connector_rows = malloc(canvas_height * sizeof(const char**));

    if (!text_rows || !connector_rows) {
        fprintf(stderr, "Critical error during memory allocation\n");
        exit(2);
    }

    for (size_t row = 0; row < canvas_height; ++row) {
        text_rows[row] = NULL;
        connector_rows[row] = NULL;

        if (row % 2 == 0) {
            text_rows[row] = malloc(canvas_width + 1);

            if (!text_rows[row]) {
                fprintf(stderr, "Critical error during memory allocation\n");
                exit(2);
            }

            memset(text_rows[row], ' ', canvas_width);
            text_rows[row][canvas_width] = '\0';
        } else {
            connector_rows[row] = calloc(canvas_width, sizeof(const char*));

            if (!connector_rows[row]) {
                fprintf(stderr, "Critical error during memory allocation\n");
                exit(2);
            }
        }
    }

    for (size_t depth = 1; depth <= levels_size; ++depth) {
        size_t row = (depth - 1) * 2;
        size_t slots_on_level = (size_t) 1 << (depth - 1);

        for (size_t count = 1; count <= levels[depth - 1].size; ++count) {
            struct Node* node = levels[depth - 1].nodes[count - 1];

            if (!node) {
                continue;
            }

            size_t center = ((2 * count - 1) * canvas_width) / (2 * slots_on_level);
            size_t label_len = strlen(node->op.sym);
            size_t start = center > label_len / 2 ? center - label_len / 2 : 0;

            if (start + label_len > canvas_width) {
                start = canvas_width - label_len;
            }

            memcpy(text_rows[row] + start, node->op.sym, label_len);

            if (depth == levels_size) {
                continue;
            }

            bool has_left = count * 2 - 1 <= levels[depth].size && levels[depth].nodes[count * 2 - 2];
            bool has_right = count * 2 <= levels[depth].size && levels[depth].nodes[count * 2 - 1];

            if (!has_left && !has_right) {
                continue;
            }

            size_t connector_row = row + 1;

            if (has_left) {
                size_t left_center = ((2 * (count * 2 - 1) - 1) * canvas_width) / (2 * ((size_t) 1 << depth));
                connector_rows[connector_row][left_center] = "┌";
                for (size_t col = left_center + 1; col < center; ++col) {
                    connector_rows[connector_row][col] = "─";
                }
            }

            if (has_right) {
                size_t right_center = ((2 * (count * 2) - 1) * canvas_width) / (2 * ((size_t) 1 << depth));
                for (size_t col = center + 1; col < right_center; ++col) {
                    connector_rows[connector_row][col] = "─";
                }
                connector_rows[connector_row][right_center] = "┐";
            }

            if (has_left && has_right) {
                connector_rows[connector_row][center] = "┴";
            } else if (has_left) {
                connector_rows[connector_row][center] = "┘";
            } else {
                connector_rows[connector_row][center] = "└";
            }
        }
    }

    for (size_t row = 0; row < canvas_height; ++row) {
        if (row % 2 == 0) {
            size_t end = canvas_width;

            while (end > 0 && text_rows[row][end - 1] == ' ') {
                --end;
            }

            printf("%.*s\n", (int) end, text_rows[row]);
            free(text_rows[row]);
        } else {
            size_t end = canvas_width;

            while (end > 0 && !connector_rows[row][end - 1]) {
                --end;
            }

            for (size_t col = 0; col < end; ++col) {
                fputs(connector_rows[row][col] ? connector_rows[row][col] : " ", stdout);
            }
            putchar('\n');
            free(connector_rows[row]);
        }
    }

    //cleanup
    free(queue);
    free(text_rows);
    free(connector_rows);

    for (size_t i = 0; i < levels_size; ++i) {
        free(levels[i].nodes);
    }

    free(levels);
}

// Helper function to check if character is an operator
static inline int is_operator(char c) {
    return c == '*' || c == '/' || c == '+' || c == '-';
}

static inline int is_bracket(char c) {
    return c == '(' || c == ')';
}

// Return substring of length len
// Starting from position pos

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
    struct Token_Pair pair = {type, symbol, sym_len, 0};
    return pair;
}

static struct Token_Pair create_token_priv(enum Token type, char* symbol, size_t sym_len, int privilege) {
    struct Token_Pair pair = {type, symbol, sym_len, privilege};
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

    int bracket_index = 0;

    int last_left_bracket_index = -1;

    int last_left_bracket_token_index = -1;

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
                fprintf(stderr, "Critical error during parsing! Consecutive minus signs are not allowed\n");
                return NULL;
            }
        } else if (is_bracket(str[i])) {
            if (str[i] == '(') {
                bracket_index++;
                last_left_bracket_index = i;
                last_left_bracket_token_index = index;
            } else {
                if (last_left_bracket_index == -1) { 
                    fprintf(stderr, "Critical error: mismatching brackets\n");
                    return NULL;
                }

                if (concurrent_sym_len > 0) {
                pairs[index++] = create_token(NUM, 
                    sub(str, i - concurrent_sym_len, concurrent_sym_len), 
                    concurrent_sym_len);
                }

                // elevate privilege of all operator tokens
                for (size_t j = (size_t)last_left_bracket_token_index; j < index; ++j) {

                    if (pairs[j].t == MUL || 
                        pairs[j].t == ADD || 
                        pairs[j].t == SUB || 
                        pairs[j].t == DIV
                    ) {
                        printf("Check privilege\n");
                        pairs[j].privilege = bracket_index;
                    }
                }
                bracket_index--;
            }

            concurrent_sym_len = 0;
        } else {
            // Invalid character encountered
            free(pairs);
            concurrent_sym_len = 0;
            return NULL;
        }
    }

    if (bracket_index != 0) {
        fprintf(stderr, "Critical error: uneven number of brackets\n");
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

                if (token.privilege == 0) {
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
                } else {
                    printf("Privilege check\n");
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
