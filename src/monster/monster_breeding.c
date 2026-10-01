// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How many monsters have been bred on this level: where the number lives

#include "config.h"
#include "constant.h"
#include "types.h"

#include "monster_breeding.h"

// No externs.h here, the same as panel.c, stores.c, options.c, stats.c,
// inventory.c, progress.c, score_death.c, player_pos.c, hp_table.c,
// player_light.c, missile_serial.c, screen_touched.c and monster_turn.c: one
// number needs nothing from the rest of the game. MAX_MON_MULT comes from
// constant.h.

// The number is owned here and is static: the only way in is through the six
// windows. It starts out as zero, so a new game begins with nothing bred, which
// is also what dungeon.c asks for on every level after the first.
static int16_t the_count;

void monster_breeding_reset(void) {
    the_count = 0;
}

bool monster_breeding_allowed(void) {
    // The comparison as mon_move() wrote it, with the sides left where they
    // were: MAX_MON_MULT on the left, so the count may equal the cap.
    return MAX_MON_MULT >= the_count;
}

void monster_breeding_note_birth(void) {
    the_count++;
}

void monster_breeding_note_death(void) {
    // The floor is the old `if (mon_tot_mult > 0)`. Without it a level that
    // loses monsters without breeding any would go negative, and a negative
    // count would hand the breeders their budget back twice over.
    if (the_count > 0) {
        the_count -= 1;
    }
}

int16_t monster_breeding_count(void) {
    return the_count;
}

void set_monster_breeding_count(int16_t count) {
    the_count = count;
}
