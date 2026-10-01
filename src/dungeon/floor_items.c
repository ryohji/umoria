// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The things lying on the floor of this level, and how far the table is filled

#include "config.h"
#include "constant.h"
#include "types.h"

#include "floor_items.h"

// THE STORAGE. The storage
// can move at step A only when that step fits in a single commit, and this
// question had 127 references in seventeen files, so B was split five ways;
// while it was split, two containers would have meant half the game reading one
// table and half reading the other.
//
// Two names, one container. The table's rows are kept PACKED, and the mark says
// how far the packing reaches; neither means anything without the other, so
// nothing outside this file may hold either one.
static inven_type t_list[MAX_TALLOC];
static int16_t tcptr;

// WHAT AN EMPTY ROW HOLDS, and the one dependency this module cannot shed.
//
// For the monsters an empty row was a named constant, `blank_monster`. Here it
// is not a constant at all: an empty floor row is a COPY OF A ROW OF THE
// DEFINITION TABLE, object_list[OBJ_NOTHING], the item called "nothing", whose
// tval is TV_NOTHING and whose character is a space. So blanking a row
// means calling invcopy(), which lives in desc.c and reads object_list, and
// this module needs it for both of the places that give a row back.
//
// Leaving the blanking outside (a window that only moved the mark) was the
// alternative, and it was not taken: "give this row back" and "start a new
// level" both mean a blank row, and a caller that moved the mark without
// blanking would leave the last level's sword lying in the table for the next
// claim to inherit. The dependency is on the definition table, which is
// read-only data, so it points the same way the module does.
//
// Declared by hand rather than by including externs.h, which would drag in
// ncurses for the sake of one name (the same choice monster_list.c made).
void invcopy(inven_type *to, int from_index);

// Blank every row and put the mark back at the start of a new level. Row 0 is
// blanked with the rest: it is the row an unchecked `t_list[c_ptr->tptr]` lands
// on, and it has to answer TV_NOTHING (see floor_items.h).
void floor_items_reset(void) {
    for (int i = 0; i < MAX_TALLOC; i++) {
        invcopy(&t_list[i], OBJ_NOTHING);
    }
    tcptr = MIN_TRIX;
}

inven_type *floor_item_at(int index) { return &t_list[index]; }

int16_t floor_items_used(void) { return tcptr; }

void set_floor_items_used(int16_t used) { tcptr = used; }

bool floor_items_is_full(void) { return tcptr == MAX_TALLOC; }

// The old popt() returned `tcptr++` -- hand out the row the mark points at,
// then move the mark past it.
int floor_items_claim_slot(void) { return tcptr++; }

// The tail of pusht(): step the mark back, then blank the row it now points at.
// Upstream's two statements in upstream's order.
void floor_items_drop_last(void) {
    tcptr--;
    invcopy(&t_list[tcptr], OBJ_NOTHING);
}
