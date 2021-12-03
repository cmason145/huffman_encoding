#include "node.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

Node *node_create(uint8_t symbol, uint64_t frequency) {
    Node *n = (Node *) malloc(sizeof(Node));
    if (n) {
        n->symbol = symbol;
        n->frequency = frequency;
        n->left = n->right = NULL;
    }
    return n;
}

void node_delete(Node **n) {
    if (*n) {
        free(*n);
        *n = NULL;
    }
    return;
}

Node *node_join(Node *left, Node *right) {
    Node *new_node = node_create('$', left->frequency + right->frequency);
    new_node->right = right;
    new_node->left = left;
    return new_node;
}

void node_print(Node *n) {
    printf("The symbol of the node is: ");
    printf("%" PRIu8, n->symbol);
    printf("\n");
    printf("The frequency of the node is: ");
    printf("%" PRIu64, n->frequency);
    printf("\n");
    return;
}
