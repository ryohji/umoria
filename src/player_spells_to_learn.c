// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
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

// THE COUNT IS STILL IN py.flags (step A). Every window goes through the pointer
// below, so #18-12-14C has one place to change: the pointer becomes the byte
// itself and this comment goes away.
extern player_type py;

static uint8_t *count(void) {
    return &py.flags.new_spells;
}

int player_spells_to_learn(void) {
    return *count();
}

void player_spells_to_learn_set(int spells_to_learn) {
    // Replacing, not adding. calc_spells() runs again on every level gain and
    // may lower this number as well as raise it (a drained stat allows fewer
    // spells, and the character forgets some), so adding here would make the
    // count climb without bound.
    *count() = (uint8_t)spells_to_learn;
}
