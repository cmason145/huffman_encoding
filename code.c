#include <stdio.h>
#include "code.h"
#include "defines.h"
#include <inttypes.h>
#include <stdbool.h>

#define BITS_PER_UNIT 8

Code code_init(void) {
    Code c;
    for (int i = 0; i < MAX_CODE_SIZE; i += 1) {
        c.bits[i] = 0;
    }
    c.top = 0;
    return c;
}

uint32_t code_size(Code *c) {
    return c->top;
}

bool code_empty(Code *c) {
    if (c->top == 0) {
        return true;
    }
    return false;
}

bool code_full(Code *c) {
    if (c->top == ALPHABET) {
        return true;
    }
    return false;
}

bool code_set_bit(Code *c, uint32_t i) {
    // Code from bv8.h comments repo
    if (i > ALPHABET) {
        return false;
    }
    c->bits[i / BITS_PER_UNIT] |= (0x1 << i % BITS_PER_UNIT);
    return true;
}

bool code_clr_bit(Code *c, uint32_t i) {
    // Code from bv8.h comments repo
    if (i > ALPHABET) {
        return false;
    }
    c->bits[i / BITS_PER_UNIT] &= ~(0x1 << i % BITS_PER_UNIT);
    return true;
}

bool code_get_bit(Code *c, uint32_t i) {
    if (i > ALPHABET) {
        return false;
    }
    return ((c->bits[i / BITS_PER_UNIT] >> i % BITS_PER_UNIT) & 0x1);
}

bool code_push_bit(Code *c, uint8_t bit) {
    if (code_full(c)) {
        return false;
    }
    if (bit == 0) {
        code_clr_bit(c, c->top);
        c->top += 1;
        return true;
    }
    code_set_bit(c, c->top);
    c->top += 1;
    return true;
}

bool code_pop_bit(Code *c, uint8_t *bit) {
    if (code_empty(c)) {
        return false;
    }
    c->top -= 1;
    *bit = code_get_bit(c, c->top);
    code_clr_bit(c, c->top);
    return true;
}

void code_print(Code *c) {
    printf("Code top: ");
    printf("%" PRIu32, c->top);
    printf("\n");
    return;
}
