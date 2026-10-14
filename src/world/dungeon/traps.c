// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Traps on the floor and on chests: setting them off and disarming them

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

#include "dungeon_level.h"
#include "dungeon_map.h"
#include "floor_items.h"
#include "level_exit.h"
#include "monster_list.h"
#include "pending_teleport.h"
#include "player_abilities.h"
#include "player_armour_class.h"
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

// Player hit a trap.  (Chuckle) -RAK-
void hit_trap(int y, int x) {
    end_find();
    change_trap(y, x);

    cave_type *c_ptr = square_at(y, x);
    inven_type *t_ptr = floor_item_at(c_ptr->tptr);

    int dam = pdamroll(t_ptr->damage);

    bigvtype tmp;
    switch (t_ptr->subval) {
    case 1: // Open pit
        msg_print("You fell into a pit!");
        if (player_takes_no_falling_damage()) {
            msg_print("You gently float down.");
        } else {
            objdes(tmp, t_ptr, true);
            take_hit(dam, tmp);
        }
        break;
    case 2: // Arrow trap
        if (test_hit(125, 0, 0, player_armour_class(), CLA_MISC_HIT)) {
            objdes(tmp, t_ptr, true);
            take_hit(dam, tmp);
            msg_print("An arrow hits you.");
        } else {
            msg_print("An arrow barely misses you.");
        }
        break;
    case 3: // Covered pit
        msg_print("You fell into a covered pit.");
        if (player_takes_no_falling_damage()) {
            msg_print("You gently float down.");
        } else {
            objdes(tmp, t_ptr, true);
            take_hit(dam, tmp);
        }
        place_trap(y, x, 0);
        break;
    case 4: // Trap door
        msg_print("You fell through a trap door!");
        leave_for_level(dungeon_level() + 1);
        if (player_takes_no_falling_damage()) {
            msg_print("You gently float down.");
        } else {
            objdes(tmp, t_ptr, true);
            take_hit(dam, tmp);
        }
        // Force the messages to display before starting to generate the next level.
        msg_print(CNIL);
        break;
    case 5: // Sleep gas
        if (!player_timed_in_force(PLAYER_TIMED_PARALYSIS)) {
            msg_print("A strange white mist surrounds you!");
            if (player_never_paralyzed()) {
                msg_print("You are unaffected.");
            } else {
                msg_print("You fall asleep.");
                player_timed_add(PLAYER_TIMED_PARALYSIS, randint(10) + 4);
            }
        }
        break;
    case 6: // Hid Obj
        (void)delete_object(y, x);
        place_object(y, x, false);
        msg_print("Hmmm, there was something under this rock.");
        break;
    case 7: // STR Dart
        if (test_hit(125, 0, 0, player_armour_class(), CLA_MISC_HIT)) {
            if (!player_stat_sustained(A_STR)) {
                (void)dec_stat(A_STR);
                objdes(tmp, t_ptr, true);
                take_hit(dam, tmp);
                msg_print("A small dart weakens you!");
            } else {
                msg_print("A small dart hits you.");
            }
        } else {
            msg_print("A small dart barely misses you.");
        }
        break;
    case 8: // Teleport
        schedule_teleport();
        msg_print("You hit a teleport trap!");

        // Light up the teleport trap, before we teleport away.
        move_light(y, x, y, x);
        break;
    case 9: // Rockfall
        take_hit(dam, "a falling rock");
        (void)delete_object(y, x);
        place_rubble(y, x);
        msg_print("You are hit by falling rock.");
        break;
    case 10: // Corrode gas
        // Makes more sense to print the message first, then damage an object.
        msg_print("A strange red gas surrounds you.");
        corrode_gas("corrosion gas");
        break;
    case 11:                       // Summon mon
        (void)delete_object(y, x); // Rune disappears.

        int num = 2 + randint(3);
        for (int i = 0; i < num; i++) {
            int ty = y;
            int tx = x;
            (void)summon_monster(&ty, &tx, false);
        }
        break;
    case 12: // Fire trap
        msg_print("You are enveloped in flames!");
        fire_dam(dam, "a fire trap");
        break;
    case 13: // Acid trap
        msg_print("You are splashed with acid!");
        acid_dam(dam, "an acid trap");
        break;
    case 14: // Poison gas
        msg_print("A pungent green gas surrounds you!");
        poison_gas(dam, "a poison gas trap");
        break;
    case 15: // Blind Gas
        msg_print("A black gas surrounds you!");
        player_timed_add(PLAYER_TIMED_BLINDNESS, randint(50) + 50);
        break;
    case 16: // Confuse Gas
        msg_print("A gas of scintillating colors surrounds you!");
        player_timed_add(PLAYER_TIMED_CONFUSION, randint(15) + 15);
        break;
    case 17: // Slow Dart
        if (test_hit(125, 0, 0, player_armour_class(), CLA_MISC_HIT)) {
            objdes(tmp, t_ptr, true);
            take_hit(dam, tmp);
            msg_print("A small dart hits you!");
            if (player_never_paralyzed()) {
                msg_print("You are unaffected.");
            } else {
                player_timed_add(PLAYER_TIMED_SLOWNESS, randint(20) + 10);
            }
        } else {
            msg_print("A small dart barely misses you.");
        }
        break;
    case 18: // CON Dart
        if (test_hit(125, 0, 0, player_armour_class(), CLA_MISC_HIT)) {
            if (!player_stat_sustained(A_CON)) {
                (void)dec_stat(A_CON);
                objdes(tmp, t_ptr, true);
                take_hit(dam, tmp);
                msg_print("A small dart saps your health!");
            } else {
                msg_print("A small dart hits you.");
            }
        } else {
            msg_print("A small dart barely misses you.");
        }
        break;
    case 19: // Secret Door
        break;
    case 99: // Scare Mon
        break;

    // Town level traps are special, the stores.
    case 101: // General
        enter_store(0);
        break;
    case 102: // Armory
        enter_store(1);
        break;
    case 103: // Weaponsmith
        enter_store(2);
        break;
    case 104: // Temple
        enter_store(3);
        break;
    case 105: // Alchemy
        enter_store(4);
        break;
    case 106: // Magic-User
        enter_store(5);
        break;

    default:
        msg_print("Unknown trap value.");
        break;
    }
}

// Chests have traps too. -RAK-
// Note: Chest traps are based on the FLAGS value
void chest_trap(int y, int x) {
    inven_type *t_ptr = floor_item_at(square_at(y, x)->tptr);

    if (CH_LOSE_STR & t_ptr->flags) {
        msg_print("A small needle has pricked you!");
        if (!player_stat_sustained(A_STR)) {
            (void)dec_stat(A_STR);
            take_hit(damroll(1, 4), "a poison needle");
            msg_print("You feel weakened!");
        } else {
            msg_print("You are unaffected.");
        }
    }
    if (CH_POISON & t_ptr->flags) {
        msg_print("A small needle has pricked you!");
        take_hit(damroll(1, 6), "a poison needle");
        player_timed_add(PLAYER_TIMED_POISON, 10 + randint(20));
    }
    if (CH_PARALYSED & t_ptr->flags) {
        msg_print("A puff of yellow gas surrounds you!");
        if (player_never_paralyzed()) {
            msg_print("You are unaffected.");
        } else {
            msg_print("You choke and pass out.");
            player_timed_set(PLAYER_TIMED_PARALYSIS, 10 + randint(20));
        }
    }
    if (CH_SUMMON & t_ptr->flags) {
        for (int i = 0; i < 3; i++) {
            int j = y;
            int k = x;
            (void)summon_monster(&j, &k, false);
        }
    }
    if (CH_EXPLODE & t_ptr->flags) {
        msg_print("There is a sudden explosion!");
        (void)delete_object(y, x);
        take_hit(damroll(5, 8), "an exploding chest");
    }
}
