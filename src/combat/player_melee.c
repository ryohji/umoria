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
#include "monster_list.h"
#include "player_body_weight.h"
#include "player_class.h"
#include "player_level.h"
#include "player_timed_effects.h"

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
