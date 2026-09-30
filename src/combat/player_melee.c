// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The player's melee attacks on monsters

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

#include "dungeon_map.h"
#include "equipment.h"
#include "inventory.h"
#include "monster_list.h"
#include "player_attack_bonuses.h"
#include "player_base_to_hit.h"
#include "player_body_weight.h"
#include "player_class.h"
#include "player_glowing_hands.h"
#include "player_level.h"
#include "player_status_flags.h"
#include "player_timed_effects.h"

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

    // The to-hit bonus is added as it is; only the character sheet triples it
    // (abilities.c).
    tot_tohit += player_to_hit_bonus();

    // if creature not lit, make it more difficult to hit
    int base_tohit;
    if (m_ptr->ml) {
        base_tohit = player_base_to_hit();
    } else {
        // An unseen target is harder to hit. The halving is this line's rule.
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

            // The damage bonus is added as it is. Stopping at 0 is this
            // attack's own rule (a negative bonus can make it negative).
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

// Make a bash attack on someone. -CJS-
// Used to be part of bash().
void py_bash(int y, int x) {

    int monster = square_at(y, x)->cptr;
    monster_type *m_ptr = monster_list_at(monster);
    creature_type *c_ptr = monster_get_creature(m_ptr->creature);
    m_ptr->csleep = 0;

    // Does the player know what he's fighting?
    const char *cdesc = monster_name_lower((vtype){0}, m_ptr);

    // Read once, used twice: `/ 10` for the to-hit, `/ 60 + 3` for the damage.
    // Nothing in between changes the weight.
    const int body_weight = player_body_weight();

    int base_tohit = py.stats.use_stat[A_STR] + equipment_at(INVEN_ARM)->weight / 2 + body_weight / 10;

    if (!m_ptr->ml) {
        base_tohit = (base_tohit / 2) - (py.stats.use_stat[A_DEX] * (BTH_PLUS_ADJ - 1)) - (player_level() * class_level_adj[player_class()][CLA_BTH] / 2);
    }

    if (test_hit(base_tohit, (int)player_level(), (int)py.stats.use_stat[A_DEX], (int)c_ptr->ac, CLA_BTH)) {
        msg_print(CONCAT("You hit ", cdesc, "."));
        int k = pdamroll(equipment_at(INVEN_ARM)->damage);
        k = critical_blow((equipment_at(INVEN_ARM)->weight / 4 + py.stats.use_stat[A_STR]), 0, k, CLA_BTH);
        k += body_weight / 60 + 3;
        if (k < 0) {
            k = 0;
        }

        // See if we done it in.
        if (mon_take_hit(monster, k)) {
            msg_print(CONCAT("You have slain ", cdesc, "."));
            prt_experience();
        } else {
            char *out_val;

            // Can not stun Balrog
            int avg_max_hp = (c_ptr->cdefense & CD_MAX_HP ? c_ptr->hd[0] * c_ptr->hd[1] : (c_ptr->hd[0] * (c_ptr->hd[1] + 1)) >> 1);
            if ((100 + randint(400) + randint(400)) > (m_ptr->hp + avg_max_hp)) {
                m_ptr->stunned += randint(3) + 1;
                if (m_ptr->stunned > 24) {
                    m_ptr->stunned = 24;
                }
                out_val = CONCAT(cdesc, " appears stunned!");
            } else {
                out_val = CONCAT(cdesc, " ignores your bash!");
            }
            out_val[0] = toupper(out_val[0]); // Capitalize
            msg_print(out_val);
        }
    } else {
        msg_print(CONCAT("You miss ", cdesc, "."));
    }
    if (randint(150) > py.stats.use_stat[A_DEX]) {
        msg_print("You are off balance.");
        player_timed_set(PLAYER_TIMED_PARALYSIS, 1 + randint(2));
    }
}
