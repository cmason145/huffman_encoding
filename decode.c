#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "code.h"
#include "defines.h"
#include "header.h"
#include "huffman.h"
#include "io.h"
#include "node.h"
#include "pq.h"
#include "stack.h"

void help(void) {
    fprintf(stderr, "SYNOPSIS\n"
                    " A Huffman encoder.\n"
                    " Compress a file using the Huffman coding algorithm.\n"
                    "\n"
                    "USAGE\n"
                    " ./encode [-h] [-i infile] [-o outfile]\n"
                    "\n"
                    "OPTIONS\n"
                    " -h             Program usage and help.\n"
                    " -v             Print compression statistics.\n"
                    " -i infile      Input file to compress.\n"
                    " -o outfile     Output of compressed data.\n");
}

#define OPTIONS "hvi:o:"

static bool stats = false;

int main(int argc, char **argv) {
    int opt = 0;
    int infile = STDIN_FILENO;
    int outfile = STDOUT_FILENO;
    // Structure and some coding liberties taken from Prof Long's Lecture
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'o':
            if ((outfile = open(optarg, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1) {
                fprintf(stderr, "Error: Failed to open outfile.\n");
                return EXIT_FAILURE;
            }
            break;
        case 'i':
            if ((infile = open(optarg, O_RDONLY)) == -1) {
                fprintf(stderr, "Error: Failed to open infile.\n");
                return EXIT_FAILURE;
            }
            break;
        case 'v': stats = true; break;
        case 'h': help(); return EXIT_SUCCESS;
        default: help(); return EXIT_FAILURE;
        }
    }

    Header h = { 0, 0, 0, 0 };

    // First read the bytes into the header
    read_bytes(infile, (uint8_t *) &h, sizeof(h));

    // Check if the magic number isn't equal to our specified magic number
    if (h.magic != MAGIC) {
        fprintf(stderr, "Invalid MAGIC number.\n");
        close(infile);
        close(outfile);
        return EXIT_FAILURE;
    }

    // Set permissions of outfile to infile
    fchmod(outfile, h.permissions);

    // Type-cast to int of of the header tree size
    int tree_size = (int) h.tree_size;

    // Our tree "buffer", which is of the tree size of infile
    uint8_t *tree = (uint8_t *) calloc(tree_size, sizeof(uint8_t));

    // Read the bytes of infile into our tree "buffer"
    int read_b;
    read_b = read_bytes(infile, tree, tree_size);

    // Get thr root from rebuild_tree
    Node *root = rebuild_tree(h.tree_size, tree);

    // New variable type-casted
    int file_size = (int) h.file_size;

    // Number of unique symbols, to fill our symbol buffer with
    int unique_symb = (tree_size + 1) / 3;

    // Buffer symbol
    uint8_t *symbol_buf = (uint8_t *) calloc(unique_symb, sizeof(uint8_t));

    // bits_re var to hold our bits from read_bit
    uint8_t bits_re;

    Node *curr_n = root;
    int decoded = 0;
    while (decoded < file_size) {
        read_bit(infile, &bits_re);
        if (bits_re == 0) {
            // Walk down left of tree
            curr_n = curr_n->left;
        } else if (bits_re == 1) {
            // Walk down right of tree
            curr_n = curr_n->right;
        }
        if (!curr_n->left && !curr_n->right) {
            //write_bit(outfile,curr_n->symbol);
            *symbol_buf = curr_n->symbol;
            write_bytes(outfile, symbol_buf, 1);
            curr_n = root;
            decoded += 1;
        }
    }

    if (stats) {
        printf("Stats.\n");
    }

    delete_tree(&root);
    free(tree);
    free(symbol_buf);
    close(infile);
    close(outfile);

    return 0;
}
