// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Picking the set bits out of a flag word one at a time

#include <stdint.h>

#include "bits.h"

// This module includes no externs.h: it calls nothing outside itself.

// Returns position of first set bit -RAK-
// and clears that bit
int bit_pos(uint32_t *test) {
    uint32_t mask = 0x1;

    // Cast to int to avoid sign conversion in comparison.
    for (int i = 0; i < (int)(sizeof(*test) * 8); i++) {
        if (*test & mask) {
            *test &= ~mask;
            return i;
        }
        mask <<= 1;
    }

    // no one bits found
    return -1;
}
