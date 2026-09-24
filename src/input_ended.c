// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Whether the input has run out: where the count lives

#include "config.h"
#include "constant.h"
#include "types.h"

#include "input_ended.h"

// No externs.h here, the same as panel.c, stores.c, options.c, stats.c,
// inventory.c, progress.c, score_death.c, player_pos.c, hp_table.c,
// player_light.c, missile_serial.c, inven_command_state.c, screen_touched.c,
// level_exit.c and pending_teleport.c: counting EOFs needs nothing from the rest
// of the game. In particular this module does not read the terminal -- it only
// keeps the tally io.c hands it.

// The count still lives in variable.c at this step (#18-11-5A); it moves in here
// as a static in #18-11-5C. Declared rather than included, so that this module
// does not pull in the global header -- the same choice as stats.c and
// object_levels.c.
extern int eof_flag;

// The number of EOFs io.c has put up with before it panic-saves and dies. Stated
// once, here, and nowhere else: see input_ended.h for why it is not at the call
// site.
#define EOF_TRIES_ALLOWED 100

void note_input_ended(void) {
    eof_flag++;
}

bool input_has_ended(void) {
    return eof_flag != 0;
}

bool input_end_is_hopeless(void) {
    return eof_flag > EOF_TRIES_ALLOWED;
}

int input_end_count(void) {
    return eof_flag;
}
