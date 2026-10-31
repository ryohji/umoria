// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Which monsters the player can see. Called by the monsters' turns and by the
// spells, bolts and moves that change what is in sight.

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"
#include "dungeon_state.h"
#include "monster_state.h"
#include "game_status.h"

#include "world_dungeon.h"
#include "world_monster.h"
#include "world_player.h"
#include "world_view.h"
#include "world_internal.h"
#include "distance.h"
#include "geometry.h"
#include "monsters.h"
#include "ui_screen.h"
#include "player_stats.h"
#include "player_skills.h"
#include "player_condition.h"

// Updates screen when monsters move about -RAK-
void update_mon(int monptr) {
    cave_type *c_ptr;
    creature_type *r_ptr;

    bool flag = false;
    monster_type *m_ptr = monster_list_at(monptr);

    // Asked once and held in one place: the condition below read the same
    // number twice.
    const int infra_range = player_infra_range();

    if ((m_ptr->cdis <= MAX_SIGHT) && !player_effect_in_force(PLAYER_EFFECT_BLIND) &&
        (panel_contains((int)m_ptr->fy, (int)m_ptr->fx))) {
        if (progress_wizard_mode()) {
            // Wizard sight.
            flag = true;
        } else if (los(player_row(), player_col(), (int)m_ptr->fy, (int)m_ptr->fx)) {
            // Normal sight.
            c_ptr = square_at(m_ptr->fy, m_ptr->fx);
            r_ptr = monster_creature_get(m_ptr->creature);
            if (c_ptr->pl || c_ptr->tl || (player_is_running() && m_ptr->cdis < 2 && player_has_light())) {
                if ((CM_INVISIBLE & r_ptr->cmove) == 0) {
                    flag = true;
                } else if (player_can_see_invisible()) {
                    flag = true;
                    recall_update_move(m_ptr->creature, CM_INVISIBLE);
                }
            } else if (infra_range > 0 && m_ptr->cdis <= infra_range && (CD_INFRA & r_ptr->cdefense)) {
                // Infra vision. Two halves, and only the first one is about the
                // character: how far the warm blood carries (this module) and
                // whether THIS monster has any (CD_INFRA, which belongs to the
                // monster). The `> 0` guard is redundant in practice -- a
                // monster is never on the player's own square, so cdis is at
                // least one -- but removing it would change behaviour.

                flag = true;
                recall_update_characteristics(m_ptr->creature, CD_INFRA);
            }
        }
    }

    if (flag) {
        // Light it up.

        if (!m_ptr->ml) {
            disturb(1, 0);
            m_ptr->ml = true;
            lite_spot((int)m_ptr->fy, (int)m_ptr->fx);

            // notify inven_command
            note_screen_flushed();
        }
    } else if (m_ptr->ml) {
        // Turn it off.

        m_ptr->ml = false;
        lite_spot((int)m_ptr->fy, (int)m_ptr->fx);

        // notify inven_command
        note_screen_flushed();
    }
}

void update_monsters(void) {
    for (int i = monster_list_used() - 1; i >= MIN_MONIX && !player_is_dead(); i--) {
        monster_type *m_ptr = monster_list_at(i);
        // Get rid of an eaten/breathed on monster.  Note: Be sure not to
        // process this monster. This is necessary because we can't delete
        // monsters while scanning the monster list here.
        if (m_ptr->hp < 0) {
            fix2_delete_monster(i);
            continue;
        }

        m_ptr->cdis = distance(player_row(), player_col(), (int)m_ptr->fy, (int)m_ptr->fx);
        update_mon(i);

        // Get rid of an eaten/breathed on monster. This is necessary because
        // we can't delete monsters while scanning the monster list here.
        if (m_ptr->hp < 0) {
            fix2_delete_monster(i);
        }
    }
}
