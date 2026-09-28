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

// THE STORAGE IS STILL monsters.c's FOR NOW. The previous three questions in
// this group put the storage in the module at step A, which works only when the
// step that moves the callers fits in one commit -- otherwise half the game
// reads one container and half reads the other. This question has 101 references
// in twelve files, so B is split by file and the storage cannot move until every
// caller is through a window. It comes over in #18-14-4C, and these three lines
// go away with it.
//
// Declared by hand rather than by including externs.h, which would drag in
// ncurses for the sake of two names (the same choice monster_levels.c and
// object_levels.c made).
extern monster_type m_list[MAX_MALLOC];
extern int16_t mfptr;

// The value an empty row holds. It lives in monsters.c among the constant
// tables, and after #18-14-4B its only two readers are both in this file -- so
// it is a name to look at again once this group is finished, not part of this
// question.
extern monster_type blank_monster;

// Link all free space in monster list together
void monster_list_reset(void) {
    for (int i = 0; i < MAX_MALLOC; i++) {
        m_list[i] = blank_monster;
    }
    mfptr = MIN_MONIX;
}

monster_type *monster_list_at(int index) { return &m_list[index]; }

int16_t monster_list_used(void) { return mfptr; }

void set_monster_list_used(int16_t used) { mfptr = used; }

bool monster_list_is_full(void) { return mfptr == MAX_MALLOC; }

int monster_list_free_slots(void) { return MAX_MALLOC - mfptr; }

// The old popm() returned `mfptr++` -- hand out the row the mark points at, then
// move the mark past it.
int monster_list_claim_slot(void) { return mfptr++; }

void monster_list_drop_last(void) {
    m_list[mfptr - 1] = blank_monster;
    mfptr -= 1;
}
