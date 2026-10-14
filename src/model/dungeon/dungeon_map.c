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

// THE STORAGE. Nothing outside this file can spell it.
//
// The table is one contiguous row-major block, and that is this file's promise
// alone. See src/dungeon/dungeon_map.h.
static cave_type cave[MAX_HEIGHT][MAX_WIDTH];

// Blank the whole table -- one memset. sizeof(cave) is the whole 66 x 198, not
// the part the level in play uses; see dungeon_map.h for why that matters.
void dungeon_map_reset(void) { memset(cave, 0, sizeof(cave)); }

cave_type *square_at(int y, int x) { return &cave[y][x]; }
