// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Player commands that act on the dungeon: tunnelling through rock and rubble,
// and bashing doors, chests and monsters

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

#include "burden.h"
#include "command_state.h"
#include "dungeon_map.h"
#include "equipment.h"
#include "floor_items.h"
#include "monster_list.h"
#include "player_body_weight.h"
#include "player_pos.h"
#include "player_search_skill.h"
#include "player_timed_effects.h"

// Tunnels through rubble and walls -RAK-
// Must take into account: secret doors, special tools
void tunnel(int dir) {
    // Confused?                    75% random movement
    if (player_timed_in_force(PLAYER_TIMED_CONFUSION) && (randint(4) > 1)) {
        dir = randint(9);
    }

    int y = player_row();
    int x = player_col();
    (void)mmove(dir, &y, &x);

    cave_type *c_ptr = square_at(y, x);

    // Compute the digging ability of player; based on
    // strength, and type of tool used
    int tabil = py.stats.use_stat[A_STR];

    inven_type *i_ptr = equipment_at(INVEN_WIELD);

    // Don't let the player tunnel somewhere illegal, this is necessary to
    // prevent the player from getting a free attack by trying to tunnel
    // somewhere where it has no effect.
    if (c_ptr->fval < MIN_CAVE_WALL &&
        (c_ptr->tptr == 0 || (floor_item_at(c_ptr->tptr)->tval != TV_RUBBLE && floor_item_at(c_ptr->tptr)->tval != TV_SECRET_DOOR))) {
        if (c_ptr->tptr == 0) {
            msg_print("Tunnel through what?  Empty air?!?");
            free_turn_flag = true;
        } else {
            msg_print("You can't tunnel through that.");
            free_turn_flag = true;
        }
        return;
    }

    if (c_ptr->cptr > 1) {
        monster_type *m_ptr = monster_list_at(c_ptr->cptr);
        msg_print(CONCAT(monster_name_or_something((vtype){0}, m_ptr), " is in your way!"));

        // let the player attack the creature
        if (!player_timed_in_force(PLAYER_TIMED_FEAR)) {
            py_attack(y, x);
        } else {
            msg_print("You are too afraid!");
        }
    } else if (i_ptr->tval != TV_NOTHING) {

        if (TR_TUNNEL & i_ptr->flags) {
            tabil += 25 + i_ptr->p1 * 50;
        } else {
            tabil += (i_ptr->damage[0] * i_ptr->damage[1]) + i_ptr->tohit + i_ptr->todam;
            // divide by two so that digging without shovel isn't too easy
            tabil >>= 1;
        }

        // If this weapon is too heavy for the player to wield properly, then
        // also make it harder to dig with it.

        if (weapon_is_too_heavy()) {
            tabil += (py.stats.use_stat[A_STR] * 15) - i_ptr->weight;
            if (tabil < 0) {
                tabil = 0;
            }
        }

        int i;

        // Regular walls; Granite, magma intrusion, quartz vein
        // Don't forget the boundary walls, made of titanium (255)
        switch (c_ptr->fval) {
        case GRANITE_WALL:
            i = randint(1200) + 80;
            if (twall(y, x, tabil, i)) {
                msg_print("You have finished the tunnel.");
            } else {
                count_msg_print("You tunnel into the granite wall.");
            }
            break;
        case MAGMA_WALL:
            i = randint(600) + 10;
            if (twall(y, x, tabil, i)) {
                msg_print("You have finished the tunnel.");
            } else {
                count_msg_print("You tunnel into the magma intrusion.");
            }
            break;
        case QUARTZ_WALL:
            i = randint(400) + 10;
            if (twall(y, x, tabil, i)) {
                msg_print("You have finished the tunnel.");
            } else {
                count_msg_print("You tunnel into the quartz vein.");
            }
            break;
        case BOUNDARY_WALL:
            msg_print("This seems to be permanent rock.");
            break;
        default:
            // Is there an object in the way?  (Rubble and secret doors)
            if (c_ptr->tptr != 0) {
                if (floor_item_at(c_ptr->tptr)->tval == TV_RUBBLE) {
                    // Rubble.

                    if (tabil > randint(180)) {
                        (void)delete_object(y, x);
                        msg_print("You have removed the rubble.");
                        if (randint(10) == 1) {
                            place_object(y, x, false);
                            if (test_light(y, x)) {
                                msg_print("You have found something!");
                            }
                        }
                        lite_spot(y, x);
                    } else {
                        count_msg_print("You dig in the rubble.");
                    }
                } else if (floor_item_at(c_ptr->tptr)->tval == TV_SECRET_DOOR) {
                    // Secret doors.

                    count_msg_print("You tunnel into the granite wall.");
                    // 探索の腕は窓口へ（#18-12-25B）。
                    search(player_row(), player_col(), player_search_chance());
                } else {
                    abort();
                }
            } else {
                abort();
            }
            break;
        }
    } else {
        msg_print("You dig with your hands, making no progress.");
    }
}

// Bash open a door or chest -RAK-
// Note: Affected by strength and weight of character
//
// For a closed door, p1 is positive if locked; negative if stuck. A disarm spell
// unlocks and unjams doors!
//
// For an open door, p1 is positive for a broken door.
//
// A closed door can be opened - harder if locked. Any door might be bashed open
// (and thereby broken). Bashing a door is (potentially) faster! You move into the
// door way. To open a stuck door, it must be bashed. A closed door can be jammed
// (which makes it stuck if previously locked).
//
// Creatures can also open doors. A creature with open door ability will (if not
// in the line of sight) move though a closed or secret door with no changes. If
// in the line of sight, closed door are openned, & secret door revealed. Whether
// in the line of sight or not, such a creature may unlock or unstick a door.
//
// A creature with no such ability will attempt to bash a non-secret door.
void bash(void) {
    int y = player_row();
    int x = player_col();

    int dir;
    if (get_dir(CNIL, &dir)) {
        if (player_timed_in_force(PLAYER_TIMED_CONFUSION)) {
            msg_print("You are confused.");
            do {
                dir = randint(9);
            } while (dir == 5);
        }
        (void)mmove(dir, &y, &x);

        cave_type *c_ptr = square_at(y, x);
        if (c_ptr->cptr > 1) {
            if (player_timed_in_force(PLAYER_TIMED_FEAR)) {
                msg_print("You are afraid!");
            } else {
                py_bash(y, x);
            }
        } else if (c_ptr->tptr != 0) {
            inven_type *t_ptr = floor_item_at(c_ptr->tptr);

            if (t_ptr->tval == TV_CLOSED_DOOR) {
                count_msg_print("You smash into the door!");
                // 扉への体当たりも窓口へ（#18-12-23B）。盾での打ちかかりとは
                // 別の関数で、割る数も違う（`/ 2`）。
                int tmp = py.stats.use_stat[A_STR] + player_body_weight() / 2;

                // Use (roughly) similar method as for monsters.
                if (randint(tmp * (20 + abs(t_ptr->p1))) < 10 * (tmp - abs(t_ptr->p1))) {
                    msg_print("The door crashes open!");
                    invcopy(floor_item_at(c_ptr->tptr), OBJ_OPEN_DOOR);
                    t_ptr->p1 = 1 - randint(2); // 50% chance of breaking door
                    c_ptr->fval = CORR_FLOOR;
                    if (!player_timed_in_force(PLAYER_TIMED_CONFUSION)) {
                        move_char(dir, false);
                    } else {
                        lite_spot(y, x);
                    }
                } else if (randint(150) > py.stats.use_stat[A_DEX]) {
                    msg_print("You are off-balance.");
                    player_timed_set(PLAYER_TIMED_PARALYSIS, 1 + randint(2));
                } else if (!command_is_repeating()) {
                    msg_print("The door holds firm.");
                }
            } else if (t_ptr->tval == TV_CHEST) {
                if (randint(10) == 1) {
                    msg_print("You have destroyed the chest.");
                    msg_print("and its contents!");
                    t_ptr->index = OBJ_RUINED_CHEST;
                    t_ptr->flags = 0;
                } else if ((CH_LOCKED & t_ptr->flags) && (randint(10) == 1)) {
                    msg_print("The lock breaks open!");
                    t_ptr->flags &= ~CH_LOCKED;
                } else {
                    count_msg_print("The chest holds firm.");
                }
            } else {
                // Can't give free turn, or else player could try directions
                // until he found invisible creature
                msg_print("You bash it, but nothing interesting happens.");
            }
        } else {
            if (c_ptr->fval < MIN_CAVE_WALL) {
                msg_print("You bash at empty space.");
            } else {
                // same message for wall as for secret door
                msg_print("You bash it, but nothing interesting happens.");
            }
        }
    }
}
