#include "huffman.h"
#include "pq.h"
#include "stack.h"
#include "io.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <inttypes.h>

Node *build_tree(uint64_t hist[static ALPHABET]) {
    PriorityQueue *pq_tree = pq_create(ALPHABET);
    // Iterate through alphabet,
    for (int i = 0; i < ALPHABET; i += 1) {
        // If the freq is greater than 0, add that to our pq
        if (hist[i] > 0) {
            Node *hist_node = node_create(i, hist[i]);
            //node_print(hist_node);
            enqueue(pq_tree, hist_node);
        }
    }

    while (pq_size(pq_tree) > 1) {
        Node *dequeue_left = NULL;
        Node *dequeue_right = NULL;
        dequeue(pq_tree, &dequeue_left);
        dequeue(pq_tree, &dequeue_right);
        Node *join_dequeue = node_join(dequeue_left, dequeue_right);
        enqueue(pq_tree, join_dequeue);
    }

    // Get the root node
    Node *root = NULL;
    dequeue(pq_tree, &root);
    pq_delete(&pq_tree);
    return root;
}

void build_codes(Node *root, Code table[static ALPHABET]) {
    static Code c;
    if (root != NULL) {
        if ((root->left == NULL) && (root->right == NULL)) {
            table[root->symbol] = c;
        } else {
            uint8_t temp_bit = 0;
            code_push_bit(&c, 0);
            build_codes(root->left, table);
            code_pop_bit(&c, &temp_bit);

            code_push_bit(&c, 1);
            build_codes(root->right, table);
            code_pop_bit(&c, &temp_bit);
        }
    }
    return;
}

void dump_tree(int outfile, Node *root) {
    uint8_t leaf = 'L';
    uint8_t interior = 'I';

    if (root != NULL) {
        dump_tree(outfile, root->left);
        dump_tree(outfile, root->right);

        if ((root->left == NULL) && (root->right == NULL)) {
            write_bytes(outfile, &leaf, 1);
            write_bytes(outfile, &(root->symbol), 1);
        } else {
            write_bytes(outfile, &interior, 1);
        }
    }
    return;
}

Node *rebuild_tree(uint16_t nbytes, uint8_t tree[static nbytes]) {
    Stack *nodes = stack_create(ALPHABET);
    for (uint16_t i = 0; i < nbytes; i += 1) {
        if (tree[i] == 'L') {
            i += 1;
            Node *n = node_create(tree[i], 1);
            stack_push(nodes, n);
        } else {
            Node *temp_right = NULL;
            Node *temp_left = NULL;
            stack_pop(nodes, &temp_right);
            stack_pop(nodes, &temp_left);
            Node *temp_join = node_join(temp_left, temp_right);
            stack_push(nodes, temp_join);
        }
    }
    Node *root = NULL;
    stack_pop(nodes, &root);
    stack_delete(&nodes);
    return root;
}

void delete_tree(Node **root) {
    if ((*root) != NULL) {
        delete_tree(&(*root)->left);
        delete_tree(&(*root)->right);
        node_delete(root);
    }
    return;
}
