// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The monsters standing on this level, and how far the table is filled

#include "config.h"
#include "constant.h"
#include "types.h"
#include "externs.h"

#include "monster_list.h"

// THE STORAGE. Two tables that have nothing to do with each other: the creature
// definition table says what a kind of monster is; this one says which monsters
// are standing on this level.
//
// The two names are one container: a table whose rows are PACKED, and the mark
// saying how much of it is in use.
static monster_type the_monsters[MAX_MALLOC];
static int16_t the_mark;

// The value an empty row holds. It stays in monsters.c among the constant
// tables, and this file is now its ONLY reader (both uses are below). It is not
// this question's subject -- it belongs to the read-only-data group -- so it is
// left where it is and noted for that group: a "blank monster" is the monster
// table's notion of an empty row, so its home is here, and moving it would make
// this module need no externs at all.

// Link all free space in monster list together
void monster_list_reset(void) {
    for (int i = 0; i < MAX_MALLOC; i++) {
        the_monsters[i] = blank_monster;
    }
    the_mark = MIN_MONIX;
}

monster_type *monster_list_at(int index) { return &the_monsters[index]; }

int16_t monster_list_used(void) { return the_mark; }

void set_monster_list_used(int16_t used) { the_mark = used; }

bool monster_list_is_full(void) { return the_mark == MAX_MALLOC; }

int monster_list_free_slots(void) { return MAX_MALLOC - the_mark; }

// The old popm() returned `mfptr++` -- hand out the row the mark points at,
// then move the mark past it.
int monster_list_claim_slot(void) { return the_mark++; }

void monster_list_drop_last(void) {
    the_monsters[the_mark - 1] = blank_monster;
    the_mark -= 1;
}
