// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How far the character can see warm-blooded creatures

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_infra_range.h"

// No externs.h here, the same as the eleven questions before this one. And no
// player_timed_effects.h either: the potion's clock counts turns, this number
// counts squares, and dungeon.c holds both ends (player_infra_range.h says why
// they are not two halves of one fact).

// THE ANSWER ITSELF. It was py.flags.see_infra until #18-12-12C; now this one
// short is the only place it lives, and the windows below are the only way to
// reach it.
//
// No reset window: zero means "no infra-vision", which is where a Human starts.
// The race writes its own distance through player_infra_range_set() when the
// character is made (create.c), and so does loading a saved game (save.c).
static int16_t the_squares;

int player_infra_range(void) {
    return the_squares;
}

void player_infra_range_adjust(int num_squares) {
    // Adding, not replacing: an item of infra-vision and the potion can both be
    // in effect at once, and py_bonuses() passes the same amount back with the
    // opposite sign when the item comes off. Nothing clamps the total.
    the_squares = (int16_t)(the_squares + num_squares);
}

void player_infra_range_set(int num_squares) {
    the_squares = (int16_t)num_squares;
}
