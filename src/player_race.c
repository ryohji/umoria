// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Which of the eight races this character is

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_race.h"

// No externs.h here, the same as the twenty-one questions before this one.

// THE ROW NUMBER ITSELF. It was py.misc.prace until #18-12-22C; now this one byte
// is the only place the answer lives, and the windows below are the only way to
// reach it. (`struct misc` is down to thirteen fields.)
//
// ZERO IS WHERE A CHARACTER STARTS AND ALSO A REAL ANSWER: nothing is chosen yet
// until the race menu is answered, and row zero happens to be Human. Unlike every
// question before this one the byte is NOT A QUANTITY -- it is a row, so there is
// nothing here to add to and no `_adjust` window to go with the setter.
static uint8_t the_row;

// THE TABLE STAYS WHERE IT IS. This is the one line that reaches out, the same
// arrangement player_level.c has for the price list -- race[] is one of the twenty
// read-only constant tables in externs.h and that group is out of scope for #18.
// Nothing in the game writes it, so bringing it in here would mean a setter with
// no callers (player_race.h says the rest).
extern race_type race[MAX_RACES];

int player_race(void) {
    return the_row;
}

void player_race_set(int row) {
    // The race menu and the saved file's byte put back. One sentence for both,
    // because both are a plain replacement -- and there is no `_adjust` to go with
    // it, because a character does not become more of a Dwarf.
    //
    // The cast is the field's own width, not a rule this window adds: `prace` was
    // a uint8_t and `p_ptr->misc.prace = j;` truncated exactly like this.
    the_row = (uint8_t)row;
}

const char *player_race_name(void) {
    // Not bounds-checked, deliberately. All three callers used to write
    // `race[py.misc.prace].trace` with no check of their own, and a window that
    // started checking would be a different game (ledger observation 24). The menu
    // cannot produce a bad row and nothing else writes this but a saved file.
    return race[the_row].trace;
}
