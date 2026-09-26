// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How fast the character is moving right now

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_speed.h"

// No externs.h here, the same as the ten questions before this one. And no
// player_status_flags.h either: search mode's one step is a different question
// and the state line keeps it (player_speed.h says why this one does not fold
// the pair the way player_timed_effects.c does).

// THE ANSWER ITSELF. It was py.flags.speed until #18-12-11C; now this one short
// is the only place it lives, and the windows below are the only way to reach it.
//
// No reset window: zero means "normal speed", which is where a new character
// starts, the same as the status word, the eighteen clocks and the rest. Loading
// a saved game writes it through player_speed_set() (save.c).
static int16_t the_steps;

static int16_t *steps(void) {
    return &the_steps;
}

int player_speed(void) {
    return *steps();
}

void player_speed_adjust(int num_steps) {
    // Adding, not replacing: change_speed() is called once per potion, item and
    // trap, and the effects stack. Nothing clamps the total.
    *steps() = (int16_t)(*steps() + num_steps);
}

void player_speed_set(int num_steps) {
    *steps() = (int16_t)num_steps;
}
