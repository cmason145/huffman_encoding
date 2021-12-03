#include "io.h"
#include "defines.h"
#include "code.h"
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

static uint8_t buffer_write[BLOCK] = { 0 };
static int write_index = 0;

int read_bytes(int infile, uint8_t *buf, int nbytes) {
    // Code from Eugenes section Thursday 10/28
    int bytes_r = 0, bytes = 0;
    do {
        bytes = read(infile, buf + bytes_r, nbytes - bytes_r);
        // to keep track of bytes read
        bytes_r += bytes;
    } while (bytes > 0);

    return bytes_r;
}

int write_bytes(int outfile, uint8_t *buf, int nbytes) {
    // Code from Eugenes section Thursday 10/28
    int bytes_w = 0, bytes = 0;
    do {
        bytes = write(outfile, buf + bytes_w, nbytes - bytes_w);
        // to keep track of bytes read
        bytes_w += bytes;
    } while (bytes > 0);

    return bytes_w;
}

bool read_bit(int infile, uint8_t *bit) {
    static int bit_index = 0; // Keeps track of the index of the bit we are on
    static int end = -1; // Keeps track of the end of our buffer

    static uint8_t buffer[BLOCK] = { 0 }; // Buffer of 4KB, or 4096 bytes

    // Pseudocode from Eugene's section, 11/2
    if (bit_index == 0) {
        int bytes_re = read_bytes(infile, buffer, BLOCK);
        // If bytes is less than the block, we need to set a new end
        if (bytes_re < BLOCK) {
            end = bytes_re + 1; // end is now the next available slot in our buffer
        }
    }

    // Return a bit using bitwise arithmetic
    *bit = (((buffer[bit_index / 8] >> bit_index % 8) & 0x1));
    bit_index += 1;
    if (bit_index == BLOCK * 8) {
        bit_index = 0; // We have reached the end of the block, we need to reset the index
    }

    return (bit_index != end); // Returns whether or not we have more bits to read
}

void write_code(int outfile, Code *c) {
    // Pseudocode from Eugene's section 11/2
    for (uint32_t i = 0; i < code_size(c); i += 1) {
        bool code_bit = code_get_bit(c, i);
        if (code_bit) {
            buffer_write[write_index / 8] |= (0x1 << write_index % 8);
        } else {
            buffer_write[write_index / 8] &= ~(0x1 << write_index % 8);
        }
        write_index += 1;
        // Check if the buffer is filled
        if (write_index == BLOCK * 8) {
            flush_codes(outfile);
            write_index = 0;
        }
    }
    return;
}

void flush_codes(int outfile) {
    // Pseudocode from Eugene's section 11/2
    //printf("write index is %d\n", write_index);
    if (write_index > 0) {
        //int to_bytes = ceil(write_index / 8);
        for (int i = write_index; i % 8 != 0; i += 1) {
            buffer_write[i / 8] &= ~(0x1 << i % 8);
        }
        int to_bytes = write_index / 8 + (write_index % 8 != 0);
        write_bytes(outfile, buffer_write, to_bytes);
    }
    return;
}
