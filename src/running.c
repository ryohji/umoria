// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Whether the player is running, and how far: where the count lives

#include "config.h"
#include "constant.h"
#include "types.h"

#include "running.h"

// No externs.h here, the same as panel.c, stores.c, options.c, stats.c,
// inventory.c, progress.c, score_death.c, player_pos.c, hp_table.c,
// player_light.c, missile_serial.c, inven_command_state.c, screen_touched.c,
// level_exit.c, pending_teleport.c and input_ended.c: counting steps needs
// nothing from the rest of the game. In particular this module does not know how
// to move anybody, nor which way the run is going (find_direction is a static of
// moria2.c, next to the corridor-following rules that are the only user of it).

// The count is owned here and is static: the only way in is through the six
// windows below. It came over from variable.c (#18-11-6C) with its initial value
// -- the old global was written "int find_flag" with no initializer, and a game
// begins with nobody running. The name follows the windows: it is a number of
// steps, not a flag.
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
