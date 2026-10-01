// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Which monster creatures() is acting for: where the number lives

#include "config.h"
#include "constant.h"
#include "types.h"

#include "monster_turn.h"

// No externs.h here: one number needs nothing from the rest of the game.

// The number is owned here and is static: the only way in is through the four
// windows. It starts at -1, so a new game starts at "nobody's turn", and so
// does a restored one (save.c never wrote it).
//
// -1 IS NOT A MONSTER AND CANNOT BE ONE. Monster numbers start at MIN_MONIX (2),
// which is what lets one comparison serve as both "the walk has not got here
// yet" and "there is no walk".
static int the_turn = -1;

void monster_turn_begin(int index) {
    the_turn = index;
}

void monster_turn_end(void) {
    the_turn = -1;
}

bool monster_delete_may_shift(int index) {
    // `hack_monptr < i`: strictly less than, so a monster asking about its own
    // entry gets a no.
    return the_turn < index;
}

int monster_turn_index(void) {
    return the_turn;
}
