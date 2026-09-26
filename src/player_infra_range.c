// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
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

// THE NUMBER IS STILL IN py.flags (step A). Every window goes through the
// pointer below, so #18-12-12C has one place to change: the pointer becomes the
// number itself and this comment goes away.
extern player_type py;

static int16_t *squares(void) {
    return &py.flags.see_infra;
}

int player_infra_range(void) {
    return *squares();
}

void player_infra_range_adjust(int num_squares) {
    // Adding, not replacing: an item of infra-vision and the potion can both be
    // in effect at once, and py_bonuses() passes the same amount back with the
    // opposite sign when the item comes off. Nothing clamps the total.
    *squares() = (int16_t)(*squares() + num_squares);
}

void player_infra_range_set(int num_squares) {
    *squares() = (int16_t)num_squares;
}
