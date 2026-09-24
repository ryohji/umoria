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

// The count still lives in variable.c at this step (#18-11-6A); it moves in here
// as a static in #18-11-6C. Declared rather than included, so that this module
// does not pull in the global header -- the same choice as stats.c and
// object_levels.c.
extern int find_flag;

// How many steps a run is allowed. Stated once, here, and nowhere else: see
// running.h for why it is not at the call site.
#define RUN_STEPS_ALLOWED 100

void begin_run(void) {
    // The first step is the one find_init() is about to take, so the count starts
    // at one rather than zero -- which is also what makes the value a "yes" to
    // player_is_running().
    find_flag = 1;
}

bool player_is_running(void) {
    return find_flag != 0;
}

void stop_running(void) {
    find_flag = 0;
}

void forget_run(void) {
    find_flag = 0;
}

bool keep_running(void) {
    // The step is counted whether or not the run may go on, exactly as the old
    // "find_flag++ > 100" did: the test reads the count from before the step.
    return find_flag++ <= RUN_STEPS_ALLOWED;
}

int running_steps(void) {
    return find_flag;
}
