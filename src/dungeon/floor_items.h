// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The things lying on the floor of this level, and how far the table is filled

#ifndef FLOOR_ITEMS_H
#define FLOOR_ITEMS_H

// THE SAME ARRANGEMENT AS THE MONSTERS, FOR THE OTHER HALF OF A SQUARE. One
// table whose rows are kept PACKED, and one mark saying how far the packing
// reaches. Every square of the level carries two indices into two such tables:
// cave[y][x].cptr says which monster stands there and cave[y][x].tptr says
// which thing lies there. This is that second table.
//
// IT IS NOT A LIST OF TREASURE, whatever upstream's name for it said. Doors,
// staircases, rubble, traps, the entrances to the six shops and the mushrooms a
// spell grows are all rows of this table, put there by the same invcopy() call
// that puts a sword there. What the table really holds is EVERYTHING ON A
// SQUARE THAT IS NOT A MONSTER -- the level's furniture as well as its loot --
// which is why 113 references to it are spread over seventeen files: almost
// every file that looks at a square looks in here.
//
// ROW 0 IS THE EMPTY ROW, AND IT IS READ. cave[y][x].tptr == 0 means "nothing
// lies here", so the first row that may be handed out is MIN_TRIX == 1. Only
// one row is reserved, not two: there is no player row here, because the player
// is not a thing lying on the floor. And the reserved row is not merely unused
// -- player_move.c reads it. After carry() picks an object up, delete_object() has
// set that square's tptr back to 0, and the next line still asks
// `t_list[c_ptr->tptr].tval == TV_RUBBLE`. That read lands on row 0 and gets
// TV_NOTHING, which is the answer it needs. So blanking row 0 at the start of a
// level is load-bearing, not tidiness.

// A new level: nothing is lying on it. Blanks every row -- row 0 included, see
// above -- and puts the mark back at the start. The one caller is
// generate_cave() in generate.c, which runs for the town and for every dungeon
// level. (It used to call a static wrapper, tlink(), whose whole body was these
// two statements, exactly as mlink() was for the monsters.)
void floor_items_reset(void);

// The row at INDEX. This is the plain `&t_list[index]` it replaces: no bounds
// check, no null for an empty row, because the callers already know which index
// they hold. Three quarters of the reads arrive here from a square --
// `t_list[c_ptr->tptr]` seventy-one times, plus three through a differently
// named pointer and one straight out of `cave[y][x].tptr` -- and the rest hold
// an index they were given by the window below.
//
// Row 0 works and is what game_state.c hands out as the base of the table.
inven_type *floor_item_at(int index);

// How far the table is filled -- ONE PAST the last occupied row, not a count of
// things on the floor (row 0 is inside the range and never holds one).
//
// This is also the number in the save file: save.c writes it, then writes that
// many rows, and reads the pair back the same way. As with the monsters, the
// restored number is really used -- it is what says how many rows to read.
int16_t floor_items_used(void);
void set_floor_items_used(int16_t used);

// Is there room for one more? Asked by popt() in object_place.c, and only there.
bool floor_items_is_full(void);

// Take the next free row and return its index. The row is NOT blanked -- every
// caller writes it immediately afterwards, most of them through invcopy(). Ask
// floor_items_is_full() first; claiming a slot past the end would run off the
// table, exactly as the old `tcptr++` did.
int floor_items_claim_slot(void);

// Blank the last occupied row and shrink the mark by one. This is the tail of
// pusht() in object_place.c: the caller moves the last row down into the hole and
// fixes up the square that pointed at it, then calls this to give the row back.
void floor_items_drop_last(void);

// WHAT THIS MODULE DOES NOT ANSWER, and three things worth knowing before the
// callers move:
//
//  1. WHICH SQUARE A ROW IS ON. It is not in the row. pusht() has to sweep the
//     whole level looking for the square whose tptr is the row it moved, and
//     upstream wrote the reason down at the struct itself (types.h:136: "extra
//     fields x and y for location in dungeon would simplify pusht()"). That
//     sweep needs cave, which is the last question of this group, so pusht()
//     keeps the move and the sweep and this module only takes the tail.
//
//  2. A ROW NUMBER IS A CURRENCY BETWEEN FUNCTIONS. popt() returns one, and
//     magic_treasure(x, level) in item_enchant.c takes one: its first act is
//     `&t_list[x]`. Three callers hand it a row number they have just claimed.
//     So a window that turns an index into a row is not an internal
//     convenience -- it is the shape the game already speaks in.
//
//  3. TWO CALLERS BORROW A ROW FOR SOMETHING THAT IS NOT ON THE FLOOR. The
//     store's restocking (store_create() in store_stock.c) builds a candidate item
//     in a floor row, decides whether the shop will take it, and gives the row
//     back; the wizard's object-sampling in files.c does the same in a loop.
//     pusht()'s own comment warns about them ("unless the object in question is
//     not in the dungeon, e.g. in store_stock.c and files.c"). The table is a
//     scratch pad as well as a place, and nothing here stops that.
//
// AND ONE HAZARD, CARRIED OVER UNCHANGED. popt() cannot fail:
//
//     int popt(void) {
//         if (tcptr == MAX_TALLOC) { compact_objects(); }
//         return tcptr++;
//     }
//
// compact_objects() is static in object_place.c and returns void, and popt() does not
// ask again whether the table is full -- so unlike popm(), which returns -1
// when compaction cannot free a monster row, this one always hands out
// `tcptr++`, off the end of the table if compaction achieved nothing.
// Compaction is written so that it cannot achieve nothing: it loops until it has
// deleted at least one thing, dropping its distance threshold by 6 each round.
// But staircases and shop doors are given chance 0, so a level whose 174
// handed-out rows were all stairs and shop doors would spin in that loop
// forever rather than overrun. Neither the missing return value nor the loop is this
// question's business; both are written down here and left exactly as they were.

#endif // FLOOR_ITEMS_H
