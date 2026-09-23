// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The hit points rolled for every level: where the record of them lives

#include "config.h"
#include "constant.h"
#include "types.h"

#include "hp_table.h"

// No externs.h here, the same as panel.c, stores.c, options.c, stats.c,
// inventory.c, progress.c, score_death.c and player_pos.c: a table of numbers
// needs nothing from the rest of the game.
//
// The table is owned here and is static: the only way in is through the three
// windows below. It came over from player.c unchanged (#18-7-2C1), with its
// comment, and it had no initial value there either -- character creation fills
// every entry before anything reads one.
//
// The original comment said why the numbers are kept at all: drain life and
// restore life must not change what the character rolled, so the rolls are
// recorded once and read back, never re-rolled.
static uint16_t player_hp[MAX_PLAYER_LEVEL];

uint16_t hp_total_at_level(int level) {
    return player_hp[level - 1];
}

void set_hp_total_at_level(int level, uint16_t total) {
    player_hp[level - 1] = total;
}

uint16_t *hp_table_slots(void) {
    return player_hp;
}
