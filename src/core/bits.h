// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Picking the set bits out of a flag word one at a time.

#ifndef BITS_H
#define BITS_H

#include <stdint.h>

// Returns the position (0 for the lowest) of the lowest set bit of `*test` and
// clears that bit, or -1 when no bit is set. The callers loop on it to visit
// each flag of a spell or effect mask in turn. The same prototype is in
// externs.h, which is where every caller reads it.
int bit_pos(uint32_t *test);

#endif // BITS_H
