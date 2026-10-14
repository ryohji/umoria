// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Searching for hidden traps and secret doors, and revealing them

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

#include "dungeon_map.h"
#include "floor_items.h"
#include "player_timed_effects.h"

// Change a trap from invisible to visible -RAK-
// Note: Secret doors are handled here
void change_trap(int y, int x) {
    cave_type *c_ptr = square_at(y, x);
    inven_type *t_ptr = floor_item_at(c_ptr->tptr);

    if (t_ptr->tval == TV_INVIS_TRAP) {
        t_ptr->tval = TV_VIS_TRAP;
        lite_spot(y, x);
    } else if (t_ptr->tval == TV_SECRET_DOOR) {
        // change secret door to closed door
        t_ptr->index = OBJ_CLOSED_DOOR;
        t_ptr->tval = object_list[OBJ_CLOSED_DOOR].tval;
        t_ptr->tchar = object_list[OBJ_CLOSED_DOOR].tchar;
        lite_spot(y, x);
    }
}

// Searches for hidden things. -RAK-
void search(int y, int x, int chance) {
    if (player_timed_in_force(PLAYER_TIMED_CONFUSION)) {
        chance = chance / 10;
    }
    if (player_timed_in_force(PLAYER_TIMED_BLINDNESS) || no_light()) {
        chance = chance / 10;
    }
    if (player_timed_in_force(PLAYER_TIMED_HALLUCINATION)) {
        chance = chance / 10;
    }
    for (int i = (y - 1); i <= (y + 1); i++) {
        for (int j = (x - 1); j <= (x + 1); j++) {
            // always in_bounds here
            if (randint(100) < chance) {
                cave_type *c_ptr = square_at(i, j);

                // Search for hidden objects
                if (c_ptr->tptr != 0) {
                    inven_type *t_ptr = floor_item_at(c_ptr->tptr);

                    // Trap on floor?
                    if (t_ptr->tval == TV_INVIS_TRAP) {
                        msgtype tmp_str;
                        bigvtype tmp_str2;

                        objdes(tmp_str2, t_ptr, true);
                        (void)snprintf(tmp_str, sizeof(tmp_str), "You have found %s", tmp_str2);
                        msg_print(tmp_str);
                        change_trap(i, j);
                        end_find();
                    } else if (t_ptr->tval == TV_SECRET_DOOR) {
                        // Secret door?
                        msg_print("You have found a secret door.");
                        change_trap(i, j);
                        end_find();
                    } else if (t_ptr->tval == TV_CHEST) {
                        // Chest is trapped?

                        // mask out the treasure bits
                        if ((t_ptr->flags & CH_TRAPPED) > 1) {
                            if (!known2_p(t_ptr)) {
                                known2(t_ptr);
                                msg_print("You have discovered a trap on the chest!");
                            } else {
                                msg_print("The chest is trapped!");
                            }
                        }
                    }
                }
            }
        }
    }
}
