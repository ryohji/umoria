// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Dungeon state: level number, map, size, and floor items

#ifndef DUNGEON_STATE_H
#define DUNGEON_STATE_H

#include <stdbool.h>
#include <stdint.h>

// cave_type, inven_type come from types.h which must be included first.

// --- From dungeon_level.h: which level ---

// Current dungeon level (0 = town, 1+ = dungeon). Difficulty scales with this.
int dungeon_level(void);

// True when in town (level 0).
bool player_is_in_town(void);

// Sets the current level.
void set_dungeon_level(int level);

// --- From dungeon_map.h: the map ---

// Resets the entire map to empty (blanks all MAX_HEIGHT x MAX_WIDTH squares).
void dungeon_map_reset(void);

// Returns pointer to the square at (y, x).
cave_type *square_at(int y, int x);

// --- From dungeon_size.h: dimensions ---

// Height of the current level.
int dungeon_height(void);

// Width of the current level.
int dungeon_width(void);

// Sets both dimensions (always set together: town is 22x66, dungeon is 66x198).
void set_dungeon_size(int height, int width);

// --- From floor_items.h: items on the floor ---

// Resets floor items table to empty (blanks all rows, resets to MIN_TRIX).
void floor_items_reset(void);

// Returns pointer to floor item at given index.
inven_type *floor_item_at(int index);

// How far the packed table is filled (next free slot).
int16_t floor_items_used(void);
void set_floor_items_used(int16_t used);

// True when the table is full.
bool floor_items_is_full(void);

// Claims the next slot and advances the mark. Returns the claimed index.
int floor_items_claim_slot(void);

// Drops the last claimed slot, moves mark back, and blanks the row.
void floor_items_drop_last(void);

#endif // DUNGEON_STATE_H
