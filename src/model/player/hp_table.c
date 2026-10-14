// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The hit points rolled for every level: where the record of them lives

#include "config.h"
#include "constant.h"
#include "types.h"

#include "hp_table.h"

// The table is owned here and is static: the only way in is through the three
// windows below. It has no initial value: character creation fills every entry
// before anything reads one.
//
// Why the numbers are kept at all: drain life and restore life must not change
// what the character rolled, so the rolls are recorded once and read back,
// never re-rolled.
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
