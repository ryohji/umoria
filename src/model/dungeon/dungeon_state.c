// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Dungeon state: level number, map, size, and floor items

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "config.h"
#include "constant.h"
#include "types.h"

#include "dungeon_state.h"
#include "item_kinds.h"

// --- Which level ---

// THE STORAGE. A new game starts in the town.
//
// A short, because that is what the save file holds. The window hands out a
// plain int: every caller widens it anyway, and the deepest level the game can
// name is 99.
//
// Zero is the town, so the initial value is not just an empty container -- it is
// where the game begins. A test pins it, because nothing else does: the first
// level is built from this number rather than the number being set first.
static int16_t the_level = 0;

int dungeon_level(void) { return the_level; }

bool player_is_in_town(void) { return the_level == 0; }

void set_dungeon_level(int level) { the_level = (int16_t)level; }

// --- The map ---

// THE STORAGE. Nothing outside this file can spell it.
//
// The table is one contiguous row-major block, and that is this file's promise
// alone. See dungeon_state.h.
static cave_type cave[MAX_HEIGHT][MAX_WIDTH];

void dungeon_map_reset(void) { memset(cave, 0, sizeof(cave)); }

cave_type *square_at(int y, int x) { return &cave[y][x]; }

// --- Dimensions ---

// THE STORAGE.
// Two shorts, kept as shorts because that is what the save file holds and what
// every caller's arithmetic was written against. The windows hand out plain
// ints: every reader widens them anyway, and nothing here is close to a short's
// limit (the biggest level is 66 by 198).
//
// Zero before the first level is made. Nothing reads them in that window -- the
// game makes a level or restores one before the first turn -- but the value is
// pinned by a test, because it is the one thing about this pair that no caller
// establishes.
static int16_t the_height;
static int16_t the_width;

int dungeon_height(void) { return the_height; }

int dungeon_width(void) { return the_width; }

void set_dungeon_size(int height, int width) {
    the_height = (int16_t)height;
    the_width = (int16_t)width;
}

// --- Items on the floor ---

// THE STORAGE.
//
// Two names, one container. The table's rows are kept PACKED, and the mark says
// how far the packing reaches; neither means anything without the other, so
// nothing outside this file may hold either one.
static inven_type t_list[MAX_TALLOC];
static int16_t tcptr;

// WHAT AN EMPTY ROW HOLDS.
//
// For the monsters an empty row was a named constant, `blank_monster`. Here it
// is not a constant at all: an empty floor row is a COPY OF A ROW OF THE
// DEFINITION TABLE, object_list[OBJ_NOTHING], the item called "nothing", whose
// tval is TV_NOTHING and whose character is a space. So blanking a row
// means calling invcopy(), which now lives in model/item/object_copy.c and
// reads object_list, and this module needs it for both of the places that give
// a row back.
//
// Leaving the blanking outside (a window that only moved the mark) was the
// alternative, and it was not taken: "give this row back" and "start a new
// level" both mean a blank row, and a caller that moved the mark without
// blanking would leave the last level's sword lying in the table for the next
// claim to inherit. The dependency is on the definition table, which is
// read-only data, so it points the same way the module does.

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

int floor_items_claim_slot(void) { return tcptr++; }

void floor_items_drop_last(void) {
    tcptr--;
    invcopy(&t_list[tcptr], OBJ_NOTHING);
}
