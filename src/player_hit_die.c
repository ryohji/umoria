// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How many faces this character's hit die has

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_hit_die.h"

// No externs.h here, the same as the fifteen questions before this one.

// THE NUMBER IS STILL IN py.misc (step A). Every window goes through the pointer
// below, so #18-12-17C has one place to change: the pointer becomes the number
// itself and this comment goes away.
extern player_type py;

static uint8_t *faces_of_the_die(void) {
    return &py.misc.hitdie;
}

int player_hit_die(void) {
    return *faces_of_the_die();
}

void player_hit_die_set(int faces) {
    // The race's base, and the saved file's byte put back. One sentence for
    // both, because both are a plain replacement (player_hit_die.h says why
    // player_max_depth needed two doors here and this question does not).
    *faces_of_the_die() = (uint8_t)faces;
}

void player_hit_die_adjust(int faces) {
    // The class's adj_hd, added to whatever the race left here. This was
    // `m_ptr->hitdie += c_ptr->adj_hd;` in create.c: one statement then, one
    // statement now, so the store is still touched once.
    *faces_of_the_die() = (uint8_t)(*faces_of_the_die() + faces);
}
