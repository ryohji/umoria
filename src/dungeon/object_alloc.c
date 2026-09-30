// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Putting things on the level: traps, rubble, gold and random objects, and
// choosing which object a level gets
//
// These take their rows from the free list of object_place.c (popt()), but
// are kept out of that file: object_levels_test links the real get_obj_num()
// together with a stand-in for popt() (tests/object_levels_fixture.c), and
// the two would meet in one object file.

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"
#include "dungeon_level.h"
#include "dungeon_map.h"
#include "dungeon_size.h"
#include "floor_items.h"
#include "object_levels.h"
#include "player_pos.h"

// Places a particular trap at location y, x -RAK-
void place_trap(int y, int x, int subval) {
    int cur_pos = popt();
    square_at(y, x)->tptr = cur_pos;
    invcopy(floor_item_at(cur_pos), OBJ_TRAP_LIST + subval);
}

// Places rubble at location y, x -RAK-
void place_rubble(int y, int x) {
    int cur_pos = popt();
    cave_type *cave_ptr = square_at(y, x);
    cave_ptr->tptr = cur_pos;
    cave_ptr->fval = BLOCKED_FLOOR;
    invcopy(floor_item_at(cur_pos), OBJ_RUBBLE);
}

// Places a treasure (Gold or Gems) at given row, column -RAK-
void place_gold(int y, int x) {
    int cur_pos = popt();
    int i = ((randint(dungeon_level() + 2) + 2) / 2) - 1;
    if (randint(OBJ_GREAT) == 1) {
        i += randint(dungeon_level() + 1);
    }
    if (i >= MAX_GOLD) {
        i = MAX_GOLD - 1;
    }
    square_at(y, x)->tptr = cur_pos;
    invcopy(floor_item_at(cur_pos), OBJ_GOLD_LIST + i);

    inven_type *t_ptr = floor_item_at(cur_pos);
    t_ptr->cost += (8L * (int32_t)randint((int)t_ptr->cost)) + randint(8);

    if (square_at(y, x)->cptr == 1) {
        msg_print("You feel something roll beneath your feet.");
    }
}

// Returns the array number of a random object -RAK-
int get_obj_num(int level, bool must_be_small) {
    int i;

    if (level == 0) {
        i = randint(objects_up_to_level(0)) - 1;
    } else {
        if (level >= MAX_OBJ_LEVEL) {
            level = MAX_OBJ_LEVEL;
        } else if (randint(OBJ_GREAT) == 1) {
            level = level * MAX_OBJ_LEVEL / randint(MAX_OBJ_LEVEL) + 1;
            if (level > MAX_OBJ_LEVEL) {
                level = MAX_OBJ_LEVEL;
            }
        }

        // This code has been added to make it slightly more likely to get the
        // higher level objects.  Originally a uniform distribution over all
        // objects less than or equal to the dungeon level. This distribution
        // makes a level n objects occur approx 2/n% of the time on level n,
        // and 1/2n are 0th level.
        do {
            if (randint(2) == 1) {
                i = randint(objects_up_to_level(level)) - 1;
            } else {
                // Choose three objects, pick the highest level.

                i = randint(objects_up_to_level(level)) - 1;
                int j = randint(objects_up_to_level(level)) - 1;
                if (i < j) {
                    i = j;
                }
                j = randint(objects_up_to_level(level)) - 1;
                if (i < j) {
                    i = j;
                }
                j = object_list[object_at_level_position(i)].level;
                if (j == 0) {
                    i = randint(objects_up_to_level(0)) - 1;
                } else {
                    // re-roll inside that level's band alone
                    i = randint(objects_at_level(j)) - 1 + first_position_at_level(j);
                }
            }
        } while ((must_be_small) && (set_large(&object_list[object_at_level_position(i)])));
    }
    return i;
}

// Places an object at given row, column co-ordinate -RAK-
void place_object(int y, int x, bool must_be_small) {
    int cur_pos = popt();
    square_at(y, x)->tptr = cur_pos;

    // split this line up to avoid a reported compiler bug
    int tmp = get_obj_num(dungeon_level(), must_be_small);
    invcopy(floor_item_at(cur_pos), object_at_level_position(tmp));
    magic_treasure(cur_pos, dungeon_level());
    if (square_at(y, x)->cptr == 1) {
        msg_print("You feel something roll beneath your feet."); // -CJS-
    }
}

// Allocates an object for tunnels and rooms -RAK-
void alloc_object(bool (*alloc_set)(int), int typ, int num) {
    for (int k = 0; k < num; k++) {
        int i, j;

        do {
            i = randint(dungeon_height()) - 1;
            j = randint(dungeon_width()) - 1;
        }

        // don't put an object beneath the player, this could cause
        // problems if player is standing under rubble, or on a trap.
        while ((!(*alloc_set)(square_at(i, j)->fval)) || (square_at(i, j)->tptr != 0) || (i == player_row() && j == player_col()));

        // NOTE: typ == 2 is not used - used to be visible traps.
        if (typ < 4) {
            if (typ == 1) {
                // typ == 1
                place_trap(i, j, randint(MAX_TRAP) - 1);
            } else {
                // typ == 3
                place_rubble(i, j);
            }
        } else {
            if (typ == 4) {
                // typ == 4
                place_gold(i, j);
            } else {
                // typ == 5
                place_object(i, j, false);
            }
        }
    }
}

// Creates objects nearby the coordinates given -RAK-
void random_object(int y, int x, int num) {
    do {
        int i = 0;

        do {
            int j = y - 3 + randint(5);
            int k = x - 4 + randint(7);

            cave_type *cave_ptr = square_at(j, k);

            if (in_bounds(j, k) && (cave_ptr->fval <= MAX_CAVE_FLOOR) && (cave_ptr->tptr == 0)) {
                if (randint(100) < 75) {
                    place_object(j, k, false);
                } else {
                    place_gold(j, k);
                }
                i = 9;
            }
            i++;
        } while (i <= 10);

        num--;
    } while (num != 0);
}
