// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Every square of the level the player is walking around

#include <string.h>

#include "config.h"
#include "constant.h"
#include "types.h"

#include "dungeon_map.h"

// THE STORAGE IS STILL IN variable.c, and it stays there until step C. The
// storage may move at step A only when moving every caller fits in one commit,
// and this question has 258 references in fifteen files; while the callers are
// being moved a few files at a time, two containers would mean half the game
// walking around one map and half around another. So this module reaches the
// one container by hand for now, and the declaration below is the only place in
// the tree that will still spell the old name once B is done.
//
// Declared here rather than by including externs.h, which would drag in ncurses
// for the sake of one name (the same choice monster_list.c and floor_items.c
// made).
extern cave_type cave[MAX_HEIGHT][MAX_WIDTH];

// Blank the whole table -- upstream's blank_cave(), one memset, moved here
// unchanged. sizeof(cave) is the whole 66 x 198, not the part the level in play
// uses; see dungeon_map.h for why that matters.
void dungeon_map_reset(void) { memset(cave, 0, sizeof(cave)); }

cave_type *square_at(int y, int x) { return &cave[y][x]; }
