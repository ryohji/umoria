// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How far the game has got: where the record of it lives

#include "config.h"
#include "constant.h"
#include "types.h"

#include "progress.h"

// No externs.h here, the same as panel.c, stores.c, options.c, stats.c and
// inventory.c: a place to keep the turn counter needs nothing from the rest of
// the game, and externs.h would drag ncurses in for five declarations.
//
// The five are still defined in variable.c at this step, so they are declared
// by hand here and the window reads and writes them through these names. That
// keeps behaviour identical while the callers are moved over one group at a
// time; the definitions come here and turn static once no caller reaches past
// the window. stats.c took the same route for `py`.
extern int32_t turn;
extern uint32_t randes_seed;
extern uint32_t town_seed;
extern bool wizard;
extern bool to_be_wizard;

// --- the turn counter ----------------------------------------------------

int32_t progress_turn(void) {
    return turn;
}

void progress_set_turn(int32_t new_turn) {
    turn = new_turn;
}

void progress_advance_turn(void) {
    turn++;
}

// --- the seeds -----------------------------------------------------------

uint32_t progress_color_seed(void) {
    return randes_seed;
}

void progress_set_color_seed(uint32_t seed) {
    randes_seed = seed;
}

uint32_t progress_town_seed(void) {
    return town_seed;
}

void progress_set_town_seed(uint32_t seed) {
    town_seed = seed;
}

// --- wizard mode ---------------------------------------------------------

bool progress_wizard_mode(void) {
    return wizard;
}

void progress_set_wizard_mode(bool on) {
    wizard = on;
}

bool progress_wizard_requested(void) {
    return to_be_wizard;
}

void progress_set_wizard_requested(bool requested) {
    to_be_wizard = requested;
}
