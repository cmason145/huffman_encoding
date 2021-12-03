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

    uint64_t histogram[ALPHABET] = { 0 };
    uint32_t unique_symb = 0;
    uint8_t buffer[BLOCK] = { 0 };

    // Builds the histogram from buffer
    int bytes_read;
    while ((bytes_read = read_bytes(infile, buffer, BLOCK)) > 0) {
        for (int i = 0; i < bytes_read; i += 1) {
            if (histogram[buffer[i]] == 0) {
                unique_symb += 1;
            }
            histogram[buffer[i]] += 1;
        }
    }

    // Handling case when file has no bytes to read in
    if (!histogram[0]) {
        unique_symb += 1;
    }

    if (!histogram[255]) {
        unique_symb += 1;
    }

    histogram[0] += 1;
    histogram[255] += 1;

    // Construct the Huffman Tree
    Node *root = build_tree(histogram);

    // Constructing the code table
    Code code_table[ALPHABET] = { 0 };
    build_codes(root, code_table);

    // Get file permission from infile and outfile
    struct stat srcstats;

    fstat(infile, &srcstats); // Get permissions of infile

    fchmod(outfile, srcstats.st_mode); // Copy permissions of infile to outfile

    Header h = { .magic = MAGIC,
        .permissions = srcstats.st_mode,
        .tree_size = (3 * unique_symb) - 1,
        .file_size = srcstats.st_size };

    write_bytes(outfile, (uint8_t *) &h, sizeof(h));

    dump_tree(outfile, root); // dump the tree

    lseek(infile, 0, SEEK_SET);

    uint8_t buffer2[BLOCK] = { 0 };
    int bytes_re;

    while ((bytes_re = read_bytes(infile, buffer2, BLOCK)) > 0) {
        for (int i = 0; i < bytes_re; i += 1) {
            write_code(outfile, &(code_table[buffer2[i]]));
        }
    }

    flush_codes(outfile);

    struct stat dststats;

    fstat(outfile, &dststats); // Get permissions of outfile

    if (stats) {
        //print statistics
        fprintf(stdout, "Uncompressed file size: ");
        fprintf(stdout, "%d", bytes_read);
        fprintf(stdout, " bytes");

        fprintf(stdout, "\nCompressed file size: ");
        fprintf(stdout, "%" PRIu64, dststats.st_size);
        fprintf(stdout, " bytes");

        fprintf(stdout, "\nSpace Saving: ");
        fprintf(stdout, "%lu", (1 - (dststats.st_size / bytes_read)) * 100);
        fprintf(stdout, " %%");
        fprintf(stdout, "\n");
    }

    delete_tree(&root);

    close(infile);
    close(outfile);

    return 0;
}
