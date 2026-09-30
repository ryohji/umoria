// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The monsters standing on this level, and how far the table is filled

#ifndef MONSTER_LIST_H
#define MONSTER_LIST_H

// ONE TABLE AND ONE NUMBER, AND THE NUMBER ONLY MEANS ANYTHING BECAUSE OF HOW
// THE TABLE IS KEPT. Every monster on the level has a row in the table, the rows
// are kept PACKED -- no gaps -- and the number says how far the packing reaches.
// So the occupied rows are exactly MIN_MONIX up to monster_list_used() - 1, and
// that is why fourteen loops in the game walk the list by counting down from it.
//
// Rows 0 and 1 are never handed out: cave[y][x].cptr uses 0 for "no monster" and
// 1 for "the player", so MIN_MONIX is 2 and the table starts filling there.
// (cave[y][x].cptr is the index into this table -- the two are one structure
// seen from two sides, which is why the last of this group's questions cannot be
// settled before this one.)
//
// PACKED IS A PROMISE, AND KEEPING IT IS WHY REMOVING A MONSTER IS AWKWARD.
// A row in the middle cannot simply be emptied; the last row is moved down into
// the hole and the table shrinks by one (moria3.c's fix2_delete_monster, which
// also rewrites the moved monster's square in the cave). Any code holding a row
// index or a row pointer across a removal is therefore holding something that
// may now mean a different monster -- the hazard #18-14-1 named, and the reason
// that question had to come first.
//
// This was `monster_type m_list[MAX_MALLOC];` and `int16_t mfptr;` in monsters.c
// -- the table of monsters standing on the level, and the mark of how far it is
// filled, sitting next to the table of monster DEFINITIONS, which is a different
// thing entirely (the definitions are the 279 kinds; this is the crowd on one
// level). Two names for one arrangement, so one module. #18-14-4.

// A new level: nobody is on it. Blanks every row and puts the mark back at the
// start. The one caller is generate_cave() in generate.c, which runs for the
// town and for every dungeon level. (It used to call a static wrapper, mlink(),
// whose whole body was these two loops; #18-14-4B dropped the wrapper.)
void monster_list_reset(void);

// The row at INDEX. This is the plain `&m_list[index]` it replaces: no bounds
// check, no null for an empty row, because the callers already know which index
// they hold (from cave[y][x].cptr, from a countdown, or from claiming a slot).
// Row 0 works and is what game_state.c hands out as the base of the table.
monster_type *monster_list_at(int index);

// How far the table is filled -- ONE PAST the last occupied row, not a count of
// monsters (rows 0 and 1 are inside the range and never hold one). The reverse
// walk is `for (int i = monster_list_used() - 1; i >= MIN_MONIX; i--)`, spelled
// out fourteen times -- thirteen of them character for character (spells.c 11,
// monster_place.c 1, player_bonuses.c 1) and one with an extra condition (creature.c stops early
// if the player has died). The same shape the definition table got an iterator
// for in #17; here the body needs the index itself, for removals and for hits,
// so the loops stay written out and only the bound comes through a window.
//
// This is also the number in the save file: save.c writes it, then writes that
// many rows, and reads the pair back the same way. Unlike #18-14-3's counter,
// THIS ONE IS REALLY USED after a restore -- the restored level's monsters are
// the rows that were written, and the mark is what says how many to read.
int16_t monster_list_used(void);
void set_monster_list_used(int16_t used);

// Is there room for one more? Asked by popm() in monster_place.c, which calls
// compact_monsters() and tries again when the answer is yes.
bool monster_list_is_full(void);

// How many rows are still free. dungeon.c asks once a turn and compacts early
// when fewer than ten are left, because compacting from there is much more likely
// to succeed than compacting from inside creatures().
int monster_list_free_slots(void);

// Take the next free row and return its index. The row is NOT blanked -- the
// caller (place_monster() in monster_place.c) writes every field. Ask
// monster_list_is_full() first; claiming a slot past the end would run off the
// table, exactly as the old `mfptr++` did.
int monster_list_claim_slot(void);

// Blank the last occupied row and shrink the mark by one. This is the second half
// of fix2_delete_monster(): the caller moves the last row down into the hole and
// fixes the cave, then calls this to give the row back.
void monster_list_drop_last(void);

#endif // MONSTER_LIST_H
