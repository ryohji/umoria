// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Traps: the player disarming them

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

#include "dungeon_map.h"
#include "floor_items.h"
#include "monster_list.h"
#include "player_class.h"
#include "player_disarm.h"
#include "player_level.h"
#include "player_pos.h"
#include "player_timed_effects.h"
#include "stats.h"

// Disarms a trap -RAK-
void disarm_trap(void) {
    int y = player_row();
    int x = player_col();

    int dir;
    if (get_dir(CNIL, &dir)) {
        (void)mmove(dir, &y, &x);

        cave_type *c_ptr = square_at(y, x);

        bool no_disarm = false;

        if (c_ptr->cptr > 1 && c_ptr->tptr != 0 && (floor_item_at(c_ptr->tptr)->tval == TV_VIS_TRAP || floor_item_at(c_ptr->tptr)->tval == TV_CHEST)) {
            monster_type *m_ptr = monster_list_at(c_ptr->cptr);
            msg_print(CONCAT(monster_name_or_something((vtype){0}, m_ptr), " is in your way!"));
        } else if (c_ptr->tptr != 0) {
            int tot = player_disarm() + 2 * todis_adj() + stat_adj(A_INT) + (class_level_adj[player_class()][CLA_DISARM] * player_level() / 3);

            if (player_timed_in_force(PLAYER_TIMED_BLINDNESS) || (no_light())) {
                tot = tot / 10;
            }
            if (player_timed_in_force(PLAYER_TIMED_CONFUSION)) {
                tot = tot / 10;
            }
            if (player_timed_in_force(PLAYER_TIMED_HALLUCINATION)) {
                tot = tot / 10;
            }

            inven_type *i_ptr = floor_item_at(c_ptr->tptr);
            int i = i_ptr->tval;
            int level = i_ptr->level;

            if (i == TV_VIS_TRAP) { // Floor trap
                if ((tot + 100 - level) > randint(100)) {
                    msg_print("You have disarmed the trap.");
                    player_gain_experience(i_ptr->p1);
                    (void)delete_object(y, x);

                    // make sure we move onto the trap even if confused
                    int tmp = player_timed_turns(PLAYER_TIMED_CONFUSION);
                    player_timed_clear(PLAYER_TIMED_CONFUSION);
                    move_char(dir, false);
                    player_timed_set(PLAYER_TIMED_CONFUSION, tmp);
                    prt_experience();
                } else if ((tot > 5) && (randint(tot) > 5)) {
                    // avoid randint(0) call
                    count_msg_print("You failed to disarm the trap.");
                } else {
                    msg_print("You set the trap off!");

                    // make sure we move onto the trap even if confused. This
                    // one ADDS the parked turns back where the one above puts
                    // them back, so confusion the trap itself caused is kept;
                    // that difference is how it has always been.
                    int tmp = player_timed_turns(PLAYER_TIMED_CONFUSION);
                    player_timed_clear(PLAYER_TIMED_CONFUSION);
                    move_char(dir, false);
                    player_timed_add(PLAYER_TIMED_CONFUSION, tmp);
                }
            } else if (i == TV_CHEST) {
                if (!known2_p(i_ptr)) {
                    msg_print("I don't see a trap.");
                    free_turn_flag = true;
                } else if (CH_TRAPPED & i_ptr->flags) {
                    if ((tot - level) > randint(100)) {
                        i_ptr->flags &= ~CH_TRAPPED;
                        if (CH_LOCKED & i_ptr->flags) {
                            i_ptr->name2 = SN_LOCKED;
                        } else {
                            i_ptr->name2 = SN_DISARMED;
                        }
                        msg_print("You have disarmed the chest.");
                        known2(i_ptr);
                        player_gain_experience(level);
                        prt_experience();
                    } else if ((tot > 5) && (randint(tot) > 5)) {
                        count_msg_print("You failed to disarm the chest.");
                    } else {
                        msg_print("You set a trap off!");
                        known2(i_ptr);
                        chest_trap(y, x);
                    }
                } else {
                    msg_print("The chest was not trapped.");
                    free_turn_flag = true;
                }
            } else {
                no_disarm = true;
            }
        } else {
            no_disarm = true;
        }

        if (no_disarm) {
            msg_print("I do not see anything to disarm there.");
            free_turn_flag = true;
        }
    }
}
