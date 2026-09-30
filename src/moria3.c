// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Misc code, mainly to handle player commands

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"
#include "floor_items.h"
#include "command_state.h"
#include "dungeon_level.h"
#include "dungeon_map.h"
#include "score_death.h"
#include "equipment.h"
#include "inventory.h"
#include "level_exit.h"
#include "monster_breeding.h"
#include "monster_list.h"
#include "monster_turn.h"
#include "panel.h"
#include "pending_teleport.h"
#include "player_abilities.h"
#include "player_armour_class.h"
#include "player_attack_bonuses.h"
#include "player_base_to_hit.h"
#include "player_class.h"
#include "player_disarm.h"
#include "player_glowing_hands.h"
#include "player_gold.h"
#include "player_level.h"
#include "player_mana.h"
#include "player_pos.h"
#include "player_search_skill.h"
#include "player_status_flags.h"
#include "player_timed_effects.h"
#include "running.h"
#include "spells_known.h"
#include "stats.h"

// Deletes a monster entry from the level -RAK-
void delete_monster(int j) {
    fix1_delete_monster(j);
    fix2_delete_monster(j);
}

// The following two procedures implement the same function as delete monster.
// However, they are used within creatures(), because deleting a monster
// while scanning the monster list causes two problems, monsters might get two
// turns, and m_ptr/monptr might be invalid after the delete_monster.
// Hence the delete is done in two steps.
//
// fix1_delete_monster does everything delete_monster does except delete
// the monster record and take the mark back, this is called in breathe, and
// a couple of places in creatures.c
void fix1_delete_monster(int j) {
    monster_type *const m_ptr = monster_list_at(j);

    // force the hp negative to ensure that the monster is dead, for example,
    // if the monster was just eaten by another, it will still have positive
    // hit points
    m_ptr->hp = -1;
    square_at(m_ptr->fy, m_ptr->fx)->cptr = 0;
    if (m_ptr->ml) {
        lite_spot(m_ptr->fy, m_ptr->fx);
    }
    monster_breeding_note_death();
}

// fix2_delete_monster does everything in delete_monster that wasn't done
// by fix1_monster_delete above, this is only called in creatures()
void fix2_delete_monster(int j) {
    monster_type *const m_ptr = monster_list_at(j);
    monster_type *const the_last = monster_list_at(monster_list_used() - 1);

    if (m_ptr != the_last) {
        square_at(the_last->fy, the_last->fx)->cptr = j;
        *m_ptr = *the_last;
    }
    // Blank the row that was just copied away and take the mark back one
    monster_list_drop_last();
}

// Creates objects nearby the coordinates given -RAK-
static int summon_object(int y, int x, int num, int typ) {
    int real_typ;
    if ((typ == 1) || (typ == 5)) {
        real_typ = 1; // typ == 1 -> objects
    } else {
        real_typ = 256; // typ == 2 -> gold
    }

    int res = 0;

    do {
        int i = 0;

        do {
            int j = y - 3 + randint(5);
            int k = x - 3 + randint(5);

            if (in_bounds(j, k) && los(y, x, j, k)) {
                cave_type *c_ptr = square_at(j, k);

                if (c_ptr->fval <= MAX_OPEN_SPACE && (c_ptr->tptr == 0)) {
                    // typ == 3 -> 50% objects, 50% gold
                    if ((typ == 3) || (typ == 7)) {
                        if (randint(100) < 50) {
                            real_typ = 1;
                        } else {
                            real_typ = 256;
                        }
                    }
                    if (real_typ == 1) {
                        place_object(j, k, (typ >= 4));
                    } else {
                        place_gold(j, k);
                    }
                    lite_spot(j, k);
                    if (test_light(j, k)) {
                        res += real_typ;
                    }
                    i = 20;
                }
            }
            i++;
        } while (i <= 20);

        num--;
    } while (num != 0);

    return res;
}

// Deletes object from given location -RAK-
int delete_object(int y, int x) {
    cave_type *c_ptr = square_at(y, x);

    if (c_ptr->fval == BLOCKED_FLOOR) {
        c_ptr->fval = CORR_FLOOR;
    }
    pusht(c_ptr->tptr);
    c_ptr->tptr = 0;
    c_ptr->fm = false;
    lite_spot(y, x);

    bool delete;

    if (test_light(y, x)) {
        delete = true;
    } else {
        delete = false;
    }

    return delete;
}

// Allocates objects upon a creatures death -RAK-
// Oh well,  another creature bites the dust. Reward the
// victor based on flags set in the main creature record.
//
// Returns a mask of bits from the given flags which indicates what the
// monster is seen to have dropped.  This may be added to monster memory.
uint32_t monster_death(int y, int x, uint32_t flags) {
    int i;
    if (flags & CM_CARRY_OBJ) {
        i = 1;
    } else {
        i = 0;
    }
    if (flags & CM_CARRY_GOLD) {
        i += 2;
    }
    if (flags & CM_SMALL_OBJ) {
        i += 4;
    }

    int number = 0;
    if ((flags & CM_60_RANDOM) && (randint(100) < 60)) {
        number++;
    }
    if ((flags & CM_90_RANDOM) && (randint(100) < 90)) {
        number++;
    }
    if (flags & CM_1D2_OBJ) {
        number += randint(2);
    }
    if (flags & CM_2D2_OBJ) {
        number += damroll(2, 2);
    }
    if (flags & CM_4D2_OBJ) {
        number += damroll(4, 2);
    }

    uint32_t dump;
    if (number > 0) {
        dump = (uint32_t)summon_object(y, x, number, i);
    } else {
        dump = 0;
    }

    if (flags & CM_WIN) {
        // maybe the player died in mid-turn
        if (!player_is_dead()) {
            set_player_has_won(true);
            prt_winner();
            msg_print("*** CONGRATULATIONS *** You have won the game.");
            msg_print("You cannot save this game, but you may retire when ready.");
        }
    }

    uint32_t res;
    if (dump) {
        res = 0;
        if (dump & 255) {
            res |= CM_CARRY_OBJ;
            if (i & 0x04) {
                res |= CM_SMALL_OBJ;
            }
        }

        if (dump >= 256) {
            res |= CM_CARRY_GOLD;
        }

        dump = (dump % 256) + (dump / 256); // number of items
        res |= dump << CM_TR_SHIFT;
    } else {
        res = 0;
    }

    return res;
}

// Decreases monsters hit points and deletes monster if needed.
// (Picking on my babies.) -RAK-
int mon_take_hit(int monptr, int dam) {
    monster_type *m_ptr = monster_list_at(monptr);
    creature_type *r_ptr = monster_get_creature(m_ptr->creature);
    m_ptr->hp -= dam;
    m_ptr->csleep = 0;

    const int m_dead = m_ptr->hp < 0;

    if (m_dead) {
        uint32_t i = monster_death(m_ptr->fy, m_ptr->fx, r_ptr->cmove);

        if ((!player_timed_in_force(PLAYER_TIMED_BLINDNESS) && m_ptr->ml) || (r_ptr->cmove & CM_WIN)) {
            recall_update_move(m_ptr->creature, i & ~CM_TREASURE);
            recall_update_carry(m_ptr->creature, (i & CM_TREASURE) >> CM_TR_SHIFT);
            recall_increment_kill(m_ptr->creature);
        }

        // the monster is worth its experience times its level, shared out by how
        // far the character has already come; what does not divide evenly is kept
        // in 65536ths (player_level.c).
        //
        // can't call prt_experience() here, as that would result in "new level"
        // message appearing before "monster dies" message.
        player_gain_shared_experience((int32_t)r_ptr->mexp * r_ptr->level);

        // in case this is called from within creatures(), this is a horrible
        // hack, the monster-list/creatures() code needs to be rewritten.
        if (monster_delete_may_shift(monptr)) {
            delete_monster(monptr);
        } else {
            fix1_delete_monster(monptr);
        }
    }

    return m_dead;
}

// Player attacks a (poor, defenseless) creature -RAK-
void py_attack(int y, int x) {
    const int crptr = square_at(y, x)->cptr;
    monster_type *const m_ptr = monster_list_at(crptr);
    const creature_type *const r_ptr = monster_get_creature(m_ptr->creature);
    m_ptr->csleep = 0;
    inven_type *i_ptr = equipment_at(INVEN_WIELD);

    // Does the player know what he's fighting?
    const char *cdesc = monster_name_lower((vtype){0}, m_ptr);

    int blows, tot_tohit;
    if (i_ptr->tval != TV_NOTHING) {
        // Proper weapon
        blows = attack_blows((int)i_ptr->weight, &tot_tohit);
    } else {
        // Bare hands?
        blows = 2;
        tot_tohit = -3;
    }

    if ((i_ptr->tval >= TV_SLING_AMMO) && (i_ptr->tval <= TV_SPIKE)) {
        // Fix for arrows
        blows = 1;
    }

    // 命中の下駄は窓口へ（#18-12-24B）。**そのまま足す** —— 3 を掛けるのは
    // 人物画面だけの規則（abilities.c）。
    tot_tohit += player_to_hit_bonus();

    // if creature not lit, make it more difficult to hit
    int base_tohit;
    if (m_ptr->ml) {
        base_tohit = player_base_to_hit();
    } else {
        // 見えない相手は当てにくい。**半分にする式はこの 1 行のもの**で、
        // 窓口には入れない（#18-12-19B）。
        base_tohit = (player_base_to_hit() / 2) - (tot_tohit * (BTH_PLUS_ADJ - 1)) - (player_level() * class_level_adj[player_class()][CLA_BTH] / 2);
    }

    int k;

    // Loop for number of blows,  trying to hit the critter.
    do {
        if (test_hit(base_tohit, (int)player_level(), tot_tohit, (int)r_ptr->ac, CLA_BTH)) {
            msg_print(CONCAT("You hit ", cdesc, "."));
            if (i_ptr->tval != TV_NOTHING) {
                k = pdamroll(i_ptr->damage);
                k = tot_dam(i_ptr, k, m_ptr->creature);
                k = critical_blow((int)i_ptr->weight, tot_tohit, k, CLA_BTH);
            } else {
                // Bare hands!?
                k = damroll(1, 1);
                k = critical_blow(1, 0, k, CLA_BTH);
            }

            // 打撃の下駄も窓口へ（#18-12-24B）。**0 で止めるのはこの行いの
            // 規則**なので呼び手に残す（負の下駄で damage が負になりうる）。
            k += player_to_damage_bonus();
            if (k < 0) {
                k = 0;
            }

            if (player_glowing_hands()) {
                player_glowing_hands_spend();
                msg_print("Your hands stop glowing.");
                char *out_val;
                if ((r_ptr->cdefense & CD_NO_SLEEP) || (randint(MAX_MONS_LEVEL) < r_ptr->level)) {
                    out_val = CONCAT(cdesc, " is unaffected.");
                } else {
                    out_val = CONCAT(cdesc, " appears confused.");
                    if (m_ptr->confused) {
                        m_ptr->confused += 3;
                    } else {
                        m_ptr->confused = 2 + randint(16);
                    }
                }
                out_val[0] = toupper(out_val[0]); // Capitalize
                msg_print(out_val);
                if (m_ptr->ml && randint(4) == 1) {
                    recall_update_characteristics(m_ptr->creature, CD_NO_SLEEP);
                }
            }

            // See if we done it in.
            if (mon_take_hit(crptr, k)) {
                msg_print(CONCAT("You have slain ", cdesc, "."));
                prt_experience();
                blows = 0;
            }

            if ((i_ptr->tval >= TV_SLING_AMMO) &&
                (i_ptr->tval <= TV_SPIKE)) // Use missiles up
            {
                i_ptr->number--;
                inventory_set_weight(inventory_weight() - i_ptr->weight);
                player_request_strength_check();

                if (i_ptr->number == 0) {
                    equipment_set_count(equipment_count() - 1);
                    py_bonuses(i_ptr, -1);
                    invcopy(i_ptr, OBJ_NOTHING);
                    calc_bonuses();
                }
            }
        } else {
            msg_print(CONCAT("You miss ", cdesc, "."));
        }
        blows--;
    } while (blows >= 1);
}

// Opens a closed door or closed chest. -RAK-
void openobject(void) {
    int y = player_row();
    int x = player_col();

    int dir;
    if (get_dir(CNIL, &dir)) {
        (void)mmove(dir, &y, &x);

        bool no_object = false;
        cave_type *c_ptr = square_at(y, x);

        if (c_ptr->cptr > 1 && c_ptr->tptr != 0 && (floor_item_at(c_ptr->tptr)->tval == TV_CLOSED_DOOR || floor_item_at(c_ptr->tptr)->tval == TV_CHEST)) {
            monster_type *m_ptr = monster_list_at(c_ptr->cptr);
            msg_print(CONCAT(monster_name_or_something((vtype){0}, m_ptr), " is in your way!"));
        } else if (c_ptr->tptr != 0) {
            // Closed door
            if (floor_item_at(c_ptr->tptr)->tval == TV_CLOSED_DOOR) {
                inven_type *t_ptr = floor_item_at(c_ptr->tptr);

                // It's locked.
                if (t_ptr->p1 > 0) {
                    int i = player_disarm() + 2 * todis_adj() + stat_adj(A_INT) + (class_level_adj[player_class()][CLA_DISARM] * player_level() / 3);

                    if (player_timed_in_force(PLAYER_TIMED_CONFUSION)) {
                        msg_print("You are too confused to pick the lock.");
                    } else if ((i - t_ptr->p1) > randint(100)) {
                        msg_print("You have picked the lock.");
                        player_gain_experience(1);
                        prt_experience();
                        t_ptr->p1 = 0;
                    } else {
                        count_msg_print("You failed to pick the lock.");
                    }
                } else if (t_ptr->p1 < 0) { // It's stuck
                    msg_print("It appears to be stuck.");
                }
                if (t_ptr->p1 == 0) {
                    invcopy(floor_item_at(c_ptr->tptr), OBJ_OPEN_DOOR);
                    c_ptr->fval = CORR_FLOOR;
                    lite_spot(y, x);
                    cancel_command_count();
                }
            } else if (floor_item_at(c_ptr->tptr)->tval == TV_CHEST) {
                // Open a closed chest.

                int i = player_disarm() + 2 * todis_adj() + stat_adj(A_INT) + (class_level_adj[player_class()][CLA_DISARM] * player_level() / 3);

                inven_type *t_ptr = floor_item_at(c_ptr->tptr);

                bool flag = false;

                if (CH_LOCKED & t_ptr->flags) {
                    if (player_timed_in_force(PLAYER_TIMED_CONFUSION)) {
                        msg_print("You are too confused to pick the lock.");
                    } else if ((i - (int)t_ptr->level) > randint(100)) {
                        msg_print("You have picked the lock.");
                        flag = true;
                        player_gain_experience(t_ptr->level);
                        prt_experience();
                    } else {
                        count_msg_print("You failed to pick the lock.");
                    }
                } else {
                    flag = true;
                }
                if (flag) {
                    t_ptr->flags &= ~CH_LOCKED;
                    t_ptr->name2 = SN_EMPTY;
                    known2(t_ptr);
                    t_ptr->cost = 0;
                }
                flag = false;

                // Was chest still trapped?   (Snicker)
                if ((CH_LOCKED & t_ptr->flags) == 0) {
                    chest_trap(y, x);
                    if (c_ptr->tptr != 0) {
                        flag = true;
                    }
                }

                // Chest treasure is allocated as if a creature
                // had been killed.
                if (flag) {
                    // clear the cursed chest/monster win flag, so that people
                    // can not win by opening a cursed chest
                    floor_item_at(c_ptr->tptr)->flags &= ~TR_CURSED;
                    (void)monster_death(y, x, floor_item_at(c_ptr->tptr)->flags);
                    floor_item_at(c_ptr->tptr)->flags = 0;
                }
            } else {
                no_object = true;
            }
        } else {
            no_object = true;
        }

        if (no_object) {
            msg_print("I do not see anything you can open there.");
            free_turn_flag = true;
        }
    }
}

// Closes an open door. -RAK-
void closeobject(void) {
    int y = player_row();
    int x = player_col();

    int dir;
    if (get_dir(CNIL, &dir)) {
        (void)mmove(dir, &y, &x);

        cave_type *c_ptr = square_at(y, x);

        bool no_object = false;

        if (c_ptr->tptr != 0) {
            if (floor_item_at(c_ptr->tptr)->tval == TV_OPEN_DOOR) {
                if (c_ptr->cptr == 0) {
                    if (floor_item_at(c_ptr->tptr)->p1 == 0) {
                        invcopy(floor_item_at(c_ptr->tptr), OBJ_CLOSED_DOOR);
                        c_ptr->fval = BLOCKED_FLOOR;
                        lite_spot(y, x);
                    } else {
                        msg_print("The door appears to be broken.");
                    }
                } else {
                    monster_type *m_ptr = monster_list_at(c_ptr->cptr);
                    msg_print(CONCAT(monster_name_or_something((vtype){0}, m_ptr), " is in your way!"));
                }
            } else {
                no_object = true;
            }
        } else {
            no_object = true;
        }

        if (no_object) {
            msg_print("I do not see anything you can close there.");
            free_turn_flag = true;
        }
    }
}

// Tunneling through real wall: 10, 11, 12 -RAK-
// Used by TUNNEL and WALL_TO_MUD
int twall(int y, int x, int t1, int t2) {
    bool found;
    bool res = false;

    if (t1 > t2) {
        cave_type *c_ptr = square_at(y, x);

        if (c_ptr->lr) {
            // Should become a room space, check to see whether
            // it should be LIGHT_FLOOR or DARK_FLOOR.
            found = false;

            for (int i = y - 1; i <= y + 1; i++) {
                for (int j = x - 1; j <= x + 1; j++) {
                    if (square_at(i, j)->fval <= MAX_CAVE_ROOM) {
                        c_ptr->fval = square_at(i, j)->fval;
                        c_ptr->pl = square_at(i, j)->pl;
                        found = true;
                        break;
                    }
                }
            }

            if (!found) {
                c_ptr->fval = CORR_FLOOR;
                c_ptr->pl = false;
            }
        } else {
            // should become a corridor space
            c_ptr->fval = CORR_FLOOR;
            c_ptr->pl = false;
        }
        c_ptr->fm = false;
        if (panel_contains(y, x)) {
            if ((c_ptr->tl || c_ptr->pl) && c_ptr->tptr != 0) {
                msg_print("You have found something!");
            }
        }
        lite_spot(y, x);
        res = true;
    }
    return res;
}
