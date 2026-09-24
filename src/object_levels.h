// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The kinds of object the dungeon can produce, ordered by depth

#ifndef OBJECT_LEVELS_H
#define OBJECT_LEVELS_H

#include <stdint.h>

// Every kind of object that can be generated, laid out in order of the depth it
// first appears at. Positions run from 0 to MAX_DUNGEON_OBJ - 1, and the ones
// belonging to a single level sit together in a band.
//
// This used to be two globals, sorted_objects and t_level, and neither meant
// anything without the other: t_level was the index and sorted_objects the body
// of one table. Five places read them, all of them working out a band.
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

#endif // OBJECT_LEVELS_H
