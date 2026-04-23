#include <string.h>
#include <cvec/cvec.h>
#include "../include/expression_tree.h"


static int get_priority(Token tok) {
    if (tok.is_unary) return 3;
    char op = tok.data.op;
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    if (op == '^') return 4;
    return 0;
}

static TreeStatus build_sub_tree(Vec(Node*) *node_stack, Token tok, size_t id) {
    size_t needed = tok.is_unary ? 1 : 2;
    if (v_len(*node_stack) < needed) return TREE_ERROR_SYNTAX_OP_ORDER;

    Node* new_node = calloc(1, sizeof(Node));
    if (!new_node) return TREE_ERROR_MEM;
    new_node->id = id;

    new_node->type = NODE_OP;
    new_node->data.op = tok.data.op;

    if (tok.is_unary) {
        if (v_len(*node_stack) < 1) return TREE_ERROR_SYNTAX_OP_ORDER;
        new_node->left = v_at(*node_stack, v_len(*node_stack) - 1);
        new_node->right = NULL;
        v_pop(*node_stack);
    } else {
        if (v_len(*node_stack) < 2) return TREE_ERROR_SYNTAX_OP_ORDER;
        new_node->right = v_at(*node_stack, v_len(*node_stack) - 1);
        v_pop(*node_stack);
        new_node->left = v_at(*node_stack, v_len(*node_stack) - 1);
        v_pop(*node_stack);
    }

    v_push(*node_stack, new_node);
    return TREE_OK;
}

TreeStatus tree_build(ExpressionTree* out_tree, FILE* stream) {
    size_t id = 0;
    TreeStatus status = TREE_OK;
    Vec(Token) tokens = tokenize_stream(stream, &status);
    if (!tokens) {
        *out_tree = NULL;
        return TREE_OK;
    }
    if (status != TREE_OK) return status;

    Vec(Token) op_stack = NULL;
    Vec(Node*) node_stack = NULL;

    for (size_t i = 0; i < v_len(tokens); i++) {
        Token tok = tokens[i];

        if (tok.type == TOK_CONST || tok.type == TOK_VAR) {
            Node* n = calloc(1, sizeof(Node));
            if (!n) { status = TREE_ERROR_MEM; goto cleanup; }
            n->id = id++;

            if (tok.type == TOK_CONST) {
                n->type = NODE_CONST;
                n->data.value = tok.data.value;
            } else {
                n->type = NODE_VAR;
                strncpy(n->data.var, tok.data.var, 8);
            }
            v_push(node_stack, n);
        }
        else if (tok.type == TOK_LPAREN) {
            Token n_tok = {.type = TOK_LPAREN, .data.op = '('};
            v_push(op_stack, n_tok);
        }
        else if (tok.type == TOK_RPAREN) {
            while (v_len(op_stack) > 0 && v_at(op_stack, v_len(op_stack)-1).data.op != '(') {
                status = build_sub_tree(&node_stack, v_at(op_stack, v_len(op_stack)-1), id++);
                v_pop(op_stack);
                if (status != TREE_OK) goto cleanup;
            }
            if (v_len(op_stack) == 0) { status = TREE_ERROR_SYNTAX_OP_ORDER; goto cleanup; }
            v_pop(op_stack);
        }
        else if (tok.type == TOK_OP) {
            int p = get_priority(tok);
            while (v_len(op_stack) > 0 && v_at(op_stack, v_len(op_stack)-1).data.op != '(') {
                int top_p = get_priority(v_at(op_stack, v_len(op_stack)-1));
                if (tok.data.op == '^') {
                    if (top_p <= p) break;
                } else {
                    if (top_p < p) break;
                }

                status = build_sub_tree(&node_stack, v_at(op_stack, v_len(op_stack)-1), id++);
                v_pop(op_stack);
                if (status != TREE_OK) goto cleanup;
            }
            v_push(op_stack, tok);
        }
    }

    while (v_len(op_stack) > 0) {
        Token top = v_at(op_stack, v_len(op_stack)-1);
        if (top.data.op == '(') { status = TREE_ERROR_SYNTAX_OP_ORDER; goto cleanup; }
        status = build_sub_tree(&node_stack, top, id++);
        v_pop(op_stack);
        if (status != TREE_OK) goto cleanup;
    }

    if (v_len(node_stack) != 1) {
        status = TREE_ERROR_SYNTAX_OP_ORDER;
        goto cleanup;
    }

    *out_tree = v_at(node_stack, 0);

cleanup:
    v_free(tokens);
    v_free(op_stack);
    if (status != TREE_OK) {
        for (size_t i = 0; i < v_len(node_stack); i++) {
            tree_free(v_at(node_stack, i));
        }
    }
    v_free(node_stack);
    return status;
}


static int nodecmp(const void* a, const void* b) {
    Node* n1 = *(Node**)a;
    Node* n2 = *(Node**)b;

    if (n1->type == NODE_CONST && n2->type != NODE_CONST) return -1;
    if (n1->type != NODE_CONST && n2->type == NODE_CONST) return 1;

    if (n1->id < n2->id) return -1;
    if (n1->id > n2->id) return 1;

    return 0;
}

static void collect_operands(Node* root, char op, Vec(Node*) *list) {
    if (!root) return;

    if (root->type == NODE_OP && root->data.op == op && root->right != NULL) {
        collect_operands(root->left, op, list);
        collect_operands(root->right, op, list);
        free(root);
    } else {
        v_push(*list, root);
    }
}


void tree_simplify(ExpressionTree tree) {
    if (!tree || tree->type != NODE_OP) return;

    tree_simplify(tree->left);
    tree_simplify(tree->right);

    char op = tree->data.op;
    if (op == '+' || op == '*') {
        Vec(Node*) operands = NULL;

        collect_operands(tree->left, op, &operands);
        collect_operands(tree->right, op, &operands);

        qsort(operands, v_len(operands), sizeof(Node*), nodecmp);

        Node* current = v_at(operands, 0);
        for (size_t i = 1; i < v_len(operands); i++) {
            Node* next_op = calloc(1, sizeof(Node));
            next_op->type = NODE_OP;
            next_op->data.op = op;
            next_op->left = current;
            next_op->right = operands[i];
            current = next_op;
        }

        tree->left = current->left;
        tree->right = current->right;
        free(current);

        v_free(operands);
    }
}


void tree_print_infix(ExpressionTree tree) {
    if (!tree) return;

    if (tree->type == NODE_CONST) {
        printf("%g", tree->data.value);
    } else if (tree->type == NODE_VAR) {
        printf("%s", tree->data.var);
    }
    else if (tree->type == NODE_OP) {
        if (tree->right == NULL) {
            printf("%c", tree->data.op);
            tree_print_infix(tree->left);
        } else {
            printf("(");
            tree_print_infix(tree->left);
            printf(" %c ", tree->data.op);
            tree_print_infix(tree->right);
            printf(")");
        }
    }
}

static void tree_print_internal(Node* root, int level, char* prefix) {
    if (root == NULL) return;

    printf("%s", prefix);
    printf("%s", (level > 0 ? "|- " : ""));

    if (root->type == NODE_CONST) {
        printf("[%.2f]\n", root->data.value);
    } else if (root->type == NODE_VAR) {
        printf("{%s}\n", root->data.var);
    } else {
        printf("(%c)\n", root->data.op);
    }

    if (root->type == NODE_OP) {
        char new_prefix[256];
        sprintf(new_prefix, "%s%s", prefix, (level > 0 ? "    " : ""));
        tree_print_internal(root->left, level + 1, new_prefix);
        tree_print_internal(root->right, level + 1, new_prefix);
    }
}

void tree_print_graph(ExpressionTree tree) {
    if (tree == NULL) {
        printf("(Segfault)\n");
        return;
    }
    tree_print_internal(tree, 0, "");
}

void tree_free(ExpressionTree tree) {
    if (!tree) return;

    tree_free(tree->left);
    tree_free(tree->right);

    free(tree);
}