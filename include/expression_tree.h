#ifndef EXPRESSION_TREE_H
#define EXPRESSION_TREE_H

#include <stddef.h>
#include <stdio.h>
#include <cvec/cvec.h>


typedef enum {
    TOK_VAR,
    TOK_CONST,
    TOK_OP,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_END
} TokenType;

typedef struct {
    TokenType type;
    union {
        double value;
        char op;
        char var[8];
    } data;
    bool is_unary;
} Token;


typedef enum {
    NODE_VAR,
    NODE_CONST,
    NODE_OP
} NodeType;

typedef enum {
    TREE_OK,
    TREE_ERROR_SYNTAX_NUM_TOO_LONG,
    TREE_ERROR_SYNTAX_VAR_TOO_LONG,
    TREE_ERROR_SYNTAX_OP_ORDER,
    TREE_ERROR_MEM,
} TreeStatus;

typedef struct Node Node;

struct Node {
    size_t id;
    NodeType type;
    union {
        double value;
        char op;
        char var[8];
    } data;
    Node *left;
    Node *right;
};

typedef Node* ExpressionTree;

Vec(Token) tokenize_stream(FILE* stream, TreeStatus* status);

[[nodiscard]] TreeStatus tree_build(ExpressionTree* out_tree, FILE* stream);

void tree_simplify(ExpressionTree tree);

void tree_print_infix(ExpressionTree tree);

void tree_print_graph(ExpressionTree tree);

void tree_free(ExpressionTree tree);

#endif // EXPRESSION_TREE_H