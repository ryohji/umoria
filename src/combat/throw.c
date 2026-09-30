// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Throwing (or firing) an object from the inventory at a monster, and where it
// lands

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

#include "dungeon_map.h"
#include "equipment.h"
#include "floor_items.h"
#include "inventory.h"
#include "monster_list.h"
#include "panel.h"
#include "player_attack_bonuses.h"
#include "player_base_to_hit.h"
#include "player_class.h"
#include "player_level.h"
#include "player_pos.h"
#include "player_status_flags.h"
#include "player_timed_effects.h"

static void inven_throw(int item_val, inven_type *t_ptr) {
    inven_type *i_ptr = inventory_at(item_val);

    *t_ptr = *i_ptr;
    if (i_ptr->number > 1) {
        t_ptr->number = 1;
        i_ptr->number--;
        inventory_set_weight(inventory_weight() - i_ptr->weight);
        player_request_strength_check();
    } else {
        inven_destroy(item_val);
    }
}

// Obtain the hit and damage bonuses and the maximum distance for a thrown missile.
static void facts(inven_type *i_ptr, int *tbth, int *tpth, int *tdam, int *tdis) {
    int tmp_weight;
    if (i_ptr->weight < 1) {
        tmp_weight = 1;
    } else {
        tmp_weight = i_ptr->weight;
    }

    // Throwing objects
    *tdam = pdamroll(i_ptr->damage) + i_ptr->todam;
    // 投げるだけなら 75 パーセント。**この割りかたはこの 1 行のもの**で、
    // 窓口には入れない（#18-12-19B）。
    *tbth = player_base_to_hit_with_bows() * 75 / 100;
    // 命中の下駄は窓口へ（#18-12-24B）。**そのまま足す**（3 倍は人物画面だけ）。
    // すぐ下の `-=` は投げ道具でない武器の分を引きもどす計算で、**局所の
    // *tpth を直すだけ**なので下駄そのものは動かない。
    *tpth = player_to_hit_bonus() + i_ptr->tohit;

    // Add this back later if the correct throwing device. -CJS-
    if (equipment_at(INVEN_WIELD)->tval != TV_NOTHING) {
        *tpth -= equipment_at(INVEN_WIELD)->tohit;
    }

    *tdis = (((py.stats.use_stat[A_STR] + 20) * 10) / tmp_weight);
    if (*tdis > 10) {
        *tdis = 10;
    }

    // multiply damage bonuses instead of adding, when have proper
    // missile/weapon combo, this makes them much more useful

    // Using Bows,  slings,  or crossbows
    if (equipment_at(INVEN_WIELD)->tval == TV_BOW) {
        switch (equipment_at(INVEN_WIELD)->p1) {
        case 1:
            if (i_ptr->tval == TV_SLING_AMMO) { // Sling and ammo
                *tbth = player_base_to_hit_with_bows();
                *tpth += 2 * equipment_at(INVEN_WIELD)->tohit;
                *tdam += equipment_at(INVEN_WIELD)->todam;
                *tdam = *tdam * 2;
                *tdis = 20;
            }
            break;
        case 2:
            if (i_ptr->tval == TV_ARROW) { // Short Bow and Arrow
                *tbth = player_base_to_hit_with_bows();
                *tpth += 2 * equipment_at(INVEN_WIELD)->tohit;
                *tdam += equipment_at(INVEN_WIELD)->todam;
                *tdam = *tdam * 2;
                *tdis = 25;
            }
            break;
        case 3:
            if (i_ptr->tval == TV_ARROW) { // Long Bow and Arrow
                *tbth = player_base_to_hit_with_bows();
                *tpth += 2 * equipment_at(INVEN_WIELD)->tohit;
                *tdam += equipment_at(INVEN_WIELD)->todam;
                *tdam = *tdam * 3;
                *tdis = 30;
            }
            break;
        case 4:
            if (i_ptr->tval == TV_ARROW) { // Composite Bow and Arrow
                *tbth = player_base_to_hit_with_bows();
                *tpth += 2 * equipment_at(INVEN_WIELD)->tohit;
                *tdam += equipment_at(INVEN_WIELD)->todam;
                *tdam = *tdam * 4;
                *tdis = 35;
            }
            break;
        case 5:
            if (i_ptr->tval == TV_BOLT) { // Light Crossbow and Bolt
                *tbth = player_base_to_hit_with_bows();
                *tpth += 2 * equipment_at(INVEN_WIELD)->tohit;
                *tdam += equipment_at(INVEN_WIELD)->todam;
                *tdam = *tdam * 3;
                *tdis = 25;
            }
            break;
        case 6:
            if (i_ptr->tval == TV_BOLT) { // Heavy Crossbow and Bolt
                *tbth = player_base_to_hit_with_bows();
                *tpth += 2 * equipment_at(INVEN_WIELD)->tohit;
                *tdam += equipment_at(INVEN_WIELD)->todam;
                *tdam = *tdam * 4;
                *tdis = 35;
            }
            break;
        }
    }
}

static void drop_throw(int y, int x, inven_type *t_ptr) {
    int i = y;
    int j = x;
    int k = 0;

    bool flag = false;
    if (randint(10) > 1) {
        do {
            if (in_bounds(i, j)) {
                cave_type *c_ptr = square_at(i, j);

                if (c_ptr->fval <= MAX_OPEN_SPACE && c_ptr->tptr == 0) {
                    flag = true;
                }
            }
            if (!flag) {
                i = y + randint(3) - 2;
                j = x + randint(3) - 2;
                k++;
            }
        } while ((!flag) && (k <= 9));
    }

    if (flag) {
        int cur_pos = popt();
        square_at(i, j)->tptr = cur_pos;
        *floor_item_at(cur_pos) = *t_ptr;
        lite_spot(i, j);
    } else {
        msgtype out_val;
        bigvtype tmp_str;
        objdes(tmp_str, t_ptr, false);
        (void)snprintf(out_val, sizeof(out_val), "The %s disappears.", tmp_str);
        msg_print(out_val);
    }
}

// Throw an object across the dungeon. -RAK-
// Note: Flasks of oil do fire damage
// Note: Extra damage and chance of hitting when missiles are used
// with correct weapon.  I.E.  wield bow and throw arrow.
void throw_object(void) {
    int item_val;

    if (inventory_count() == 0) {
        msg_print("But you are not carrying anything.");
        free_turn_flag = true;
    } else if (get_item(&item_val, "Fire/Throw which one?", 0, inventory_count() - 1, CNIL, CNIL)) {
        int dir;
        if (get_dir(CNIL, &dir)) {
            desc_remain(item_val);
            if (player_timed_in_force(PLAYER_TIMED_CONFUSION)) {
                msg_print("You are confused.");
                do {
                    dir = randint(9);
                } while (dir == 5);
            }

            inven_type throw_obj;
            inven_throw(item_val, &throw_obj);

            int tbth, tpth, tdam, tdis;
            facts(&throw_obj, &tbth, &tpth, &tdam, &tdis);

            char tchar = throw_obj.tchar;
            bool flag = false;
            bool visible;
            int y = player_row();
            int x = player_col();
            int oldy = player_row();
            int oldx = player_col();
            int cur_dis = 0;

            do {
                (void)mmove(dir, &y, &x);
                cur_dis++;
                lite_spot(oldy, oldx);
                if (cur_dis > tdis) {
                    flag = true;
                }

                cave_type *c_ptr = square_at(y, x);
                if ((c_ptr->fval <= MAX_OPEN_SPACE) && (!flag)) {
                    if (c_ptr->cptr > 1) {
                        flag = true;
                        monster_type *m_ptr = monster_list_at(c_ptr->cptr);
                        const creature_type *const r_ptr = monster_get_creature(m_ptr->creature);
                        tbth = tbth - cur_dis;

                        // if monster not lit, make it much more difficult to hit, subtract
                        // off most bonuses, and reduce bthb depending on distance.
                        if (!m_ptr->ml) {
                            tbth = (tbth / (cur_dis + 2)) - (player_level() * class_level_adj[player_class()][CLA_BTHB] / 2) - (tpth * (BTH_PLUS_ADJ - 1));
                        }

                        if (test_hit(tbth, (int)player_level(), tpth, (int)r_ptr->ac, CLA_BTHB)) {
                            bigvtype tmp_str;
                            objdes(tmp_str, &throw_obj, false);

                            msgtype out_val;

                            // Does the player know what he's fighting?
                            if (!m_ptr->ml) {
                                (void)snprintf(out_val, sizeof(out_val), "You hear a cry as the %s finds a mark.", tmp_str);
                                visible = false;
                            } else {
                                (void)snprintf(out_val, sizeof(out_val), "The %s hits the %s.", tmp_str, r_ptr->name);
                                visible = true;
                            }
                            msg_print(out_val);
                            tdam = tot_dam(&throw_obj, tdam, m_ptr->creature);
                            tdam = critical_blow((int)throw_obj.weight, tpth, tdam, CLA_BTHB);
                            if (tdam < 0) {
                                tdam = 0;
                            }

                            if (mon_take_hit((int)c_ptr->cptr, tdam)) {
                                if (!visible) {
                                    msg_print("You have killed something!");
                                } else {
                                    (void)sprintf(out_val, "You have killed the %s.", r_ptr->name);
                                    msg_print(out_val);
                                }
                                prt_experience();
                            }
                        } else {
                            drop_throw(oldy, oldx, &throw_obj);
                        }
                    } else {
                        // do not test c_ptr->fm here

                        if (panel_contains(y, x) && !player_timed_in_force(PLAYER_TIMED_BLINDNESS) && (c_ptr->tl || c_ptr->pl)) {
                            print(tchar, y, x);
                            put_qio(); // show object moving
                        }
                    }
                } else {
                    flag = true;
                    drop_throw(oldy, oldx, &throw_obj);
                }
                oldy = y;
                oldx = x;
            } while (!flag);
        }
    }
}
