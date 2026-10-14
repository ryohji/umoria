// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#ifndef ITEM_KINDS_H
#define ITEM_KINDS_H

#include "config.h"
#include "constant.h"
#include "types.h"

#include <stdint.h>

// Every kind of object that can be generated, laid out in order of the depth it
// first appears at. Positions run from 0 to MAX_DUNGEON_OBJ - 1, and the ones
// belonging to a single level sit together in a band.
//
// Two arrays make the one table, and neither means anything without the other:
// t_level is the index and sorted_objects the body (object_levels.c).
//
// The table is built once at startup from object_list and never changes, so
// nothing saves it -- loading a game rebuilds it.

// Build the table. Call once, before anything asks for a position.
void object_levels_init(void);

// Which kind of object sits at a position -- an index into object_list.
int16_t object_at_level_position(int position);

// How many kinds appear at `level` or shallower. This is the count the random
// choice is bounded by: rolling within it can turn up anything down to `level`.
int objects_up_to_level(int level);

// How many kinds appear at exactly `level`, and where that level's band starts.
// Both take a level of 1 or more; level 0's band starts at position 0 and its
// width is objects_up_to_level(0).
int objects_at_level(int level);
int first_position_at_level(int level);

// inven_type comes from types.h, which has to be included before this header.

// Copy fields from object_list[from_index] into the inven_type pointed to
// by `to`. Used to initialize inventory slots, floor items, and store items
// from the definition table. Sets index to from_index, clears name2 and
// inscrip, copies all other fields from the template, and sets ident to 0.
void invcopy(inven_type *to, int from_index);

#endif // ITEM_KINDS_H
