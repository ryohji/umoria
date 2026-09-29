// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How many more spells (or prayers) the character may still learn

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_spells_to_learn.h"

// No externs.h here, the same as the thirteen questions before this one. And no
// spells_known.h either: calc_spells() subtracts the number of spells already
// known to make this count, but it does that once and remembers the answer
// (player_spells_to_learn.h says why the two are not halves of one fact).

// THE ANSWER ITSELF. It was py.flags.new_spells until #18-12-14C; now this one
// byte is the only place it lives, and the windows below are the only way to
// reach it.
//
// No reset window: zero means "nothing may be learned right now", which is where
// every character starts, where every fighter stays, and where gain_spells()
// leaves a character who has studied everything the level allows.
//
// With this byte moved, py.flags holds only three fields, and all three are
// dead (types.h says so).
static uint8_t the_count;

int player_spells_to_learn(void) {
    return the_count;
}

void player_spells_to_learn_set(int spells_to_learn) {
    // Replacing, not adding. calc_spells() runs again on every level gain and
    // may lower this number as well as raise it (a drained stat allows fewer
    // spells, and the character forgets some), so adding here would make the
    // count climb without bound.
    the_count = (uint8_t)spells_to_learn;
}
