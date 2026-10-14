// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The game's one random number generator.

#ifndef RND_H
#define RND_H

#include <stdbool.h>
#include <stdint.h>

// The same prototypes are in externs.h, which is where every caller reads
// them.

// Starts the stream again from `seedval`.
void set_rnd_seed(uint32_t seedval);
// The next number of the stream, from 1 to 2^31 - 2.
int32_t rnd(void);
// Switches to the reproducible stream of `seed` (town layout, item colours)
// and remembers where the normal stream was; reset_seed() goes back to it.
void set_seed(uint32_t seed);
void reset_seed(void);
// From 1 to `maxval`.
int randint(int maxval);
// Roughly normal around `mean` with deviation `stand` (`stand` must be > 0).
int randnor(int mean, int stand);
// True with a chance of `chance` in 100.
bool magik(int chance);

#endif // RND_H
