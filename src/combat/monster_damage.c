// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Hurting a monster: taking hit points off it, and the kill and the experience
// when they run out

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

#include "monster_list.h"
#include "monster_turn.h"
#include "player_level.h"
#include "player_timed_effects.h"

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
