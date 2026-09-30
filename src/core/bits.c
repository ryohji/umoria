// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Picking the set bits out of a flag word one at a time

#include <stdint.h>

#include "bits.h"

// No externs.h here, the same as str_insert.c: nothing outside this file is
// called, so the linker reports no unresolved symbol at all.

// Returns position of first set bit -RAK-
// and clears that bit
int bit_pos(uint32_t *test) {
    uint32_t mask = 0x1;

    // i は int、sizeof(*test) * 8 は size_t（符号なし）。そのまま比べると
    // i が符号なしに変換される。ここは i >= 0 しか通らないので値は変わらない
    // が、変換が起きていることを明示しておく。
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
