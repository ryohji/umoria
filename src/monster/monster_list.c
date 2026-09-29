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

#include "monster_list.h"

// THE STORAGE. It came over from monsters.c in #18-14-4C, where it sat right
// beside the 279-row creature definition table -- two tables that have nothing
// to do with each other. That one says what a kind of monster is; this one says
// which monsters are standing on this level.
//
// The two names were always one container: a table whose rows are PACKED, and
// the mark saying how much of it is in use. Keeping them apart is what let
// fourteen count-down loops and the two delete paths each re-derive the packing
// promise by hand.
//
// It took the three earlier questions in this group to get here. The previous
// three put the storage in the module at step A, which works only when the step
// that moves the callers fits in one commit -- otherwise half the game reads one
// container and half reads the other. This one had 101 references in twelve
// files, so B was split five ways and the storage waited until every caller was
// through a window.
static monster_type the_monsters[MAX_MALLOC];
static int16_t the_mark;

// The value an empty row holds. It stays in monsters.c among the constant
// tables, and this file is now its ONLY reader (both uses are below). It is not
// this question's subject -- it belongs to the read-only-data group -- so it is
// left where it is and noted for that group: a "blank monster" is the monster
// table's notion of an empty row, so its home is here, and moving it would make
// this module need no externs at all.
//
// Declared by hand rather than by including externs.h, which would drag in
// ncurses for the sake of one name (the same choice monster_levels.c and
// object_levels.c made).
extern monster_type blank_monster;

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
