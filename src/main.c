#include <stdio.h>
#include "../include/expression_tree.h"

int main() {
    ExpressionTree tree = NULL;
    int c;
    printf("> ");
    while ((c = getchar()) != EOF) {
        ungetc(c, stdin);
        TreeStatus status = tree_build(&tree, stdin);

        if (status == TREE_OK) {
            printf("Original: ");
            tree_print_infix(tree);
            printf("\n");

            tree_simplify(tree);

            printf("Simplified: ");
            tree_print_infix(tree);
            printf("\n\nGraph view:\n\n");
            tree_print_graph(tree);

            tree_free(tree);
        } else {
            fprintf(stderr, "Error status: %d\n", status);
        }
        printf("> ");
    }

    return 0;
}