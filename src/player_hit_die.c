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

// THE NUMBER ITSELF. It was py.misc.hitdie until #18-12-17C; now this one byte
// is the only place it lives, and the windows below are the only way to reach
// it.
//
// Zero is not a die any character plays with -- create.c puts a 6 to a 12 here
// as soon as a race is chosen. Zero only means "no race yet", which is where
// the game starts. (`struct misc` is down to twenty fields.)
static uint8_t the_faces;

int player_hit_die(void) {
    return the_faces;
}

void player_hit_die_set(int faces) {
    // The race's base, and the saved file's byte put back. One sentence for
    // both, because both are a plain replacement (player_hit_die.h says why
    // player_max_depth needed two doors here and this question does not).
    the_faces = (uint8_t)faces;
}

void player_hit_die_adjust(int faces) {
    // The class's adj_hd, added to whatever the race left here. This was
    // `m_ptr->hitdie += c_ptr->adj_hd;` in create.c: one statement then, one
    // statement now, so the store is still touched once.
    the_faces = (uint8_t)(the_faces + faces);
}
