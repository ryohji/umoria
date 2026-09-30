// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Moving the player about the map: teleporting, and walking one step and
// dealing with what lies on the square stepped onto

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"
#include "dungeon_map.h"
#include "dungeon_size.h"
#include "floor_items.h"
#include "inventory.h"
#include "monster_list.h"
#include "pending_teleport.h"
#include "player_gold.h"
#include "player_pos.h"
#include "player_search_skill.h"
#include "player_status_flags.h"
#include "player_timed_effects.h"
#include "running.h"

// Teleport the player to a new location -RAK-
void teleport(int dis) {
    int y, x;

    do {
        y = randint(dungeon_height()) - 1;
        x = randint(dungeon_width()) - 1;
        while (distance(y, x, player_row(), player_col()) > dis) {
            y += ((player_row() - y) / 2);
            x += ((player_col() - x) / 2);
        }
    } while ((square_at(y, x)->fval >= MIN_CLOSED_SPACE) || (square_at(y, x)->cptr >= 2));

    move_rec(player_row(), player_col(), y, x);

    for (int i = player_row() - 1; i <= player_row() + 1; i++) {
        for (int j = player_col() - 1; j <= player_col() + 1; j++) {
            square_at(i, j)->tl = false;
            lite_spot(i, j);
        }
    }

    lite_spot(player_row(), player_col());
    player_place(y, x);
    check_view();
    creatures(false);
    teleport_done();
}

// Player is on an object. Many things can happen based -RAK-
// on the TVAL of the object. Traps are set off, money and most objects
// are picked up. Some objects, such as open doors, just sit there.
static void carry(int y, int x, bool pickup) {
    msgtype out_val;
    bigvtype tmp_str;

    cave_type *c_ptr = square_at(y, x);
    inven_type *i_ptr = floor_item_at(c_ptr->tptr);

    int i = floor_item_at(c_ptr->tptr)->tval;
    if (i <= TV_MAX_PICK_UP) {
        end_find();

        // There's GOLD in them thar hills!
        if (i == TV_GOLD) {
            player_gain_gold(i_ptr->cost);
            objdes(tmp_str, i_ptr, true);
            (void)sprintf(out_val, "You have found %d gold pieces worth of %s",
                          i_ptr->cost, tmp_str);
            prt_gold();
            (void)delete_object(y, x);
            msg_print(out_val);
        } else {
            // Too many objects?
            if (inven_check_num(i_ptr)) {
                // Okay,  pick it up
                if (pickup && prompt_carry_flag) {
                    objdes(tmp_str, i_ptr, true);

                    // change the period to a question mark
                    tmp_str[strlen(tmp_str) - 1] = '?';
                    (void)sprintf(out_val, "Pick up %s", tmp_str);
                    pickup = get_check(out_val);
                }

                // Check to see if it will change the players speed.
                if (pickup && !inven_check_weight(i_ptr)) {
                    objdes(tmp_str, i_ptr, true);

                    // change the period to a question mark
                    tmp_str[strlen(tmp_str) - 1] = '?';
                    (void)sprintf(out_val, "Exceed your weight limit to pick up %s", tmp_str);
                    pickup = get_check(out_val);
                }

                // Attempt to pick up an object.
                if (pickup) {
                    int locn = inven_carry(i_ptr);

                    objdes(tmp_str, inventory_at(locn), true);
                    (void)sprintf(out_val, "You have %s (%c)", tmp_str, locn + 'a');
                    msg_print(out_val);
                    (void)delete_object(y, x);
                }
            } else {
                objdes(tmp_str, i_ptr, true);
                (void)snprintf(out_val, sizeof(out_val), "You can't carry %s", tmp_str);
                msg_print(out_val);
            }
        }
    } else if (i == TV_INVIS_TRAP || i == TV_VIS_TRAP || i == TV_STORE_DOOR) {
        // OPPS!

        hit_trap(y, x);
    }
}

// Moves player from one space to another. -RAK-
// Note: This routine has been pre-declared; see that for argument
void move_char(int dir, bool do_pickup) {
    if (player_timed_in_force(PLAYER_TIMED_CONFUSION) && // Confused?
        (randint(4) > 1) &&        // 75% random movement
        (dir != 5))                // Never random if sitting
    {
        dir = randint(9);
        end_find();
    }

    int y = player_row();
    int x = player_col();

    // Legal move?
    if (mmove(dir, &y, &x)) {
        cave_type *c_ptr = square_at(y, x);

        // if there is no creature, or an unlit creature in the walls then...
        // disallow attacks against unlit creatures in walls because moving into
        // a wall is a free turn normally, hence don't give player free turns
        // attacking each wall in an attempt to locate the invisible creature,
        // instead force player to tunnel into walls which always takes a turn
        if ((c_ptr->cptr < 2) ||
            (!monster_list_at(c_ptr->cptr)->ml && c_ptr->fval >= MIN_CLOSED_SPACE)) {
            // Open floor spot
            if (c_ptr->fval <= MAX_OPEN_SPACE) {
                // Make final assignments of char co-ords
                int old_row = player_row();
                int old_col = player_col();
                player_place(y, x);

                // Move character record (-1)
                move_rec(old_row, old_col, player_row(), player_col());

                // Check for new panel
                if (get_panel(player_row(), player_col(), false)) {
                    prt_map();
                }

                // Check to see if he should stop
                if (player_is_running()) {
                    area_affect(dir, player_row(), player_col());
                }

                // Check to see if he notices something.
                //
                // 探索の腕と頻度は窓口へ（#18-12-25B）。**頻度は 1 つの式で
                // 2 度読むので入口の局所に畳んだ**（→ 所見 35 の 1 つめの形）。
                // **1 以下なら毎回見るという規則も、負になりうるのも呼び手の側**
                // —— the frequency may be negative if have good rings of searching.
                const int how_often = player_search_frequency();
                if ((how_often <= 1) || (randint(how_often) == 1) ||
                    player_is_searching()) {
                    search(player_row(), player_col(), player_search_chance());
                }

                // A room of light should be lit.
                if (c_ptr->fval == LIGHT_FLOOR) {
                    if (!c_ptr->pl && !player_timed_in_force(PLAYER_TIMED_BLINDNESS)) {
                        light_room(player_row(), player_col());
                    }
                }

                // In doorway of light-room?
                else if (c_ptr->lr && !player_timed_in_force(PLAYER_TIMED_BLINDNESS)) {
                    for (int i = (player_row() - 1); i <= (player_row() + 1); i++) {
                        for (int j = (player_col() - 1); j <= (player_col() + 1); j++) {
                            cave_type *d_ptr = square_at(i, j);

                            if ((d_ptr->fval == LIGHT_FLOOR) && (!d_ptr->pl)) {
                                light_room(i, j);
                            }
                        }
                    }
                }

                // Move the light source
                move_light(old_row, old_col, player_row(), player_col());

                // An object is beneath him.
                if (c_ptr->tptr != 0) {
                    carry(player_row(), player_col(), do_pickup);

                    // if stepped on falling rock trap, and space contains
                    // rubble, then step back into a clear area
                    if (floor_item_at(c_ptr->tptr)->tval == TV_RUBBLE) {
                        move_rec(player_row(), player_col(), old_row, old_col);
                        move_light(player_row(), player_col(), old_row, old_col);
                        player_place(old_row, old_col);

                        // check to see if we have stepped back onto another
                        // trap, if so, set it off
                        c_ptr = square_at(player_row(), player_col());
                        if (c_ptr->tptr != 0) {
                            int i = floor_item_at(c_ptr->tptr)->tval;
                            if (i == TV_INVIS_TRAP || i == TV_VIS_TRAP ||
                                i == TV_STORE_DOOR) {
                                hit_trap(player_row(), player_col());
                            }
                        }
                    }
                }
            } else {
                // Can't move onto floor space

                if (!player_is_running() && (c_ptr->tptr != 0)) {
                    if (floor_item_at(c_ptr->tptr)->tval == TV_RUBBLE) {
                        msg_print("There is rubble blocking your way.");
                    } else if (floor_item_at(c_ptr->tptr)->tval == TV_CLOSED_DOOR) {
                        msg_print("There is a closed door blocking your way.");
                    }
                } else {
                    end_find();
                }
                free_turn_flag = true;
            }
        } else {
            // Attacking a creature!

            bool was_running = player_is_running();
            end_find();

            // if player can see monster, and was in find mode, then nothing
            if (monster_list_at(c_ptr->cptr)->ml && was_running) {
                // did not do anything this turn
                free_turn_flag = true;
            } else {
                // Coward?
                if (!player_timed_in_force(PLAYER_TIMED_FEAR)) {
                    py_attack(y, x);
                } else { // Coward!
                    msg_print("You are too afraid!");
                }
            }
        }
    }
}
