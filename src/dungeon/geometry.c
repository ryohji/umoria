// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Questions about the squares around a point of the map

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "dungeon_map.h"
#include "externs.h"
#include "floor_items.h"

// Moved out of misc1.c unchanged (#54); their prototypes stay in externs.h.
// This is where the rest of the geometry in misc1.c (in_bounds(), distance()
// and los()) is meant to come (docs/refactoring/layout.md).

// Checks points north, south, east, and west for a wall -RAK-
// note that y,x is always in_bounds(), i.e. inside the boundary ring:
// 0 < y < height-1 and 0 < x < width-1 (see dungeon_size.h)
int next_to_walls(int y, int x) {
    int i = 0;
    cave_type *c_ptr = square_at(y - 1, x);

    if (c_ptr->fval >= MIN_CAVE_WALL) {
        i++;
    }
    c_ptr = square_at(y + 1, x);
    if (c_ptr->fval >= MIN_CAVE_WALL) {
        i++;
    }
    c_ptr = square_at(y, x - 1);
    if (c_ptr->fval >= MIN_CAVE_WALL) {
        i++;
    }
    c_ptr = square_at(y, x + 1);
    if (c_ptr->fval >= MIN_CAVE_WALL) {
        i++;
    }

    return i;
}

// Checks all adjacent spots for corridors -RAK-
// note that y, x is always in_bounds(), hence no need to check that
// j, k are in_bounds(), even if they are 0 or cur_x-1 is still works
int next_to_corr(int y, int x) {
    int i = 0;

    for (int j = y - 1; j <= (y + 1); j++) {
        for (int k = x - 1; k <= (x + 1); k++) {
            cave_type *c_ptr = square_at(j, k);

            // should fail if there is already a door present
            if (c_ptr->fval == CORR_FLOOR &&
                (c_ptr->tptr == 0 || floor_item_at(c_ptr->tptr)->tval < TV_MIN_DOORS)) {
                i++;
            }
        }
    }

    return i;
}
