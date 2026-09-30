// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Taking a row of the floor-item table for a new object, and giving one back

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "dungeon_map.h"
#include "dungeon_size.h"
#include "externs.h"
#include "floor_items.h"
#include "player_pos.h"

// Moved out of misc1.c unchanged (#54); the prototypes of popt() and pusht()
// stay in externs.h. compact_objects() is static and has only popt() to serve,
// so it came along.

// If too many objects on floor level, delete some of them-RAK-
static void compact_objects(void) {
    msg_print("Compacting objects...");

    int ctr = 0;
    int cur_dis = 66;

    do {
        for (int i = 0; i < dungeon_height(); i++) {
            for (int j = 0; j < dungeon_width(); j++) {
                int chance;

                cave_type *cave_ptr = square_at(i, j);
                if ((cave_ptr->tptr != 0) &&
                    (distance(i, j, player_row(), player_col()) > cur_dis)) {
                    switch (floor_item_at(cave_ptr->tptr)->tval) {
                    case TV_VIS_TRAP:
                        chance = 15;
                        break;
                    case TV_INVIS_TRAP:
                    case TV_RUBBLE:
                    case TV_OPEN_DOOR:
                    case TV_CLOSED_DOOR:
                        chance = 5;
                        break;
                    case TV_UP_STAIR:
                    case TV_DOWN_STAIR:
                    case TV_STORE_DOOR:
                        // Stairs, don't delete them.
                        // Shop doors, don't delete them.
                        chance = 0;
                        break;
                    case TV_SECRET_DOOR: // secret doors
                        chance = 3;
                        break;
                    default:
                        chance = 10;
                    }
                    if (randint(100) <= chance) {
                        (void)delete_object(i, j);
                        ctr++;
                    }
                }
            }
        }
        if (ctr == 0) {
            cur_dis -= 6;
        }
    } while (ctr <= 0);

    if (cur_dis < 66) {
        prt_map();
    }
}

// Gives pointer to next free space -RAK-
int popt(void) {
    if (floor_items_is_full()) {
        compact_objects();
    }
    return floor_items_claim_slot();
}

// Pushs a record back onto free space list -RAK-
// Delete_object() should always be called instead, unless the object
// in question is not in the dungeon, e.g. in store_stock.c and files.c
void pusht(uint8_t x) {
    const int last = floor_items_used() - 1;

    if (x != last) {
        *floor_item_at(x) = *floor_item_at(last);

        // must change the tptr in the cave of the object just moved
        for (int i = 0; i < dungeon_height(); i++) {
            for (int j = 0; j < dungeon_width(); j++) {
                if (square_at(i, j)->tptr == last) {
                    square_at(i, j)->tptr = x;
                }
            }
        }
    }
    // Blank the row that was just copied away and take the mark back one
    floor_items_drop_last();
}
