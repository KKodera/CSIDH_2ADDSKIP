#ifndef UINT_H
#define UINT_H

#include <stdbool.h>
#include <stdint.h>

typedef struct uintbig {
    uint64_t c[(BITS+63)/64];
} uintbig;

extern const uintbig uintbig_1;

void uintbig_set(uintbig *x, uint64_t y);

bool uintbig_bit(uintbig const *x, uint64_t k);

bool uintbig_add3(uintbig *x, uintbig const *y, uintbig const *z); /* returns carry */
bool uintbig_sub3(uintbig *x, uintbig const *y, uintbig const *z); /* returns borrow */

void uintbig_mul3_64(uintbig *x, uintbig const *y, uint64_t z);

#endif
