// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Whether the player is running, and how far: where the count lives

#include "config.h"
#include "constant.h"
#include "types.h"

#include "running.h"

// No externs.h here: counting steps needs nothing from the rest of the game.
// In particular this module does not know how to move anybody, nor which way the
// run is going (find_direction is a static of run_path.c, next to the
// corridor-following rules that are the only user of it).

// The count is owned here and is static: the only way in is through the six
// windows below. The initial value is zero, which is how a game begins with nobody
// running. The name follows the windows: it is a number of steps, not a flag.
static int run_steps = 0;

// How many steps a run is allowed. Stated once, here, and nowhere else: see
// running.h for why it is not at the call site.
#define RUN_STEPS_ALLOWED 100

void begin_run(void) {
    // The first step is the one find_init() is about to take, so the count starts
    // at one rather than zero -- which is also what makes the value a "yes" to
    // player_is_running().
    run_steps = 1;
}

bool player_is_running(void) {
    return run_steps != 0;
}

void stop_running(void) {
    run_steps = 0;
}

void forget_run(void) {
    run_steps = 0;
}

bool keep_running(void) {
    // The step is counted whether or not the run may go on, exactly as the old
    // "find_flag++ > 100" did: the test reads the count from before the step.
    return run_steps++ <= RUN_STEPS_ALLOWED;
}

int running_steps(void) {
    return run_steps;
}
