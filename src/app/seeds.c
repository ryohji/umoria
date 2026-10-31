// Copyright (c) 2025-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The seeds of a new game.

#include "app_internal.h"

#include "game_status.h"
#include "rnd.h"

#include <time.h>

// Not in core/rnd.c with the rest of the seeding, because it also seeds the
// colors of unknown items and the town (progress.c), and reads the clock.
// gets a new random seed for the random number generator
void init_seeds(uint32_t seed) {
    uint32_t clock_var;

    if (seed == 0) {
        clock_var = (uint32_t)time((time_t *)0);
    } else {
        clock_var = seed;
    }
    progress_set_color_seed(clock_var);

    clock_var += 8762;
    progress_set_town_seed(clock_var);

    clock_var += 113452L;
    set_rnd_seed(clock_var);
    // make it a little more random
    for (clock_var = (uint32_t)randint(100); clock_var != 0; clock_var--) {
        (void)rnd();
    }
}
