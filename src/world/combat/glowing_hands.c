// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The glowing hands of a scroll of Monster Confusion, from either side of a fight.

#include "config.h"
#include "constant.h"
#include "types.h"
#include "game_status.h"
#include "monster_state.h"
#include "monsters.h"
#include "rnd.h"
#include "str_insert.h"
#include "ui_screen.h"
#include "player_condition.h"
#include "world_internal.h"

// The glowing hands (the scroll of Monster Confusion) are spent on the first
// monster they touch, whether the player hits it or it hits the player. A
// monster that never sleeps, or one that wins a roll against its level, is not
// confused. visible is whether the player saw the monster, for its memory.
void glowing_hands_touch(monster_type *m_ptr, bool visible) {
    const creature_type *r_ptr = monster_creature_get(m_ptr->creature);
    const char *cdesc = monster_name((vtype){0}, m_ptr);

    msg_print("Your hands stop glowing.");
    player_glowing_hands_spend();
    const char *verb;
    if ((CD_NO_SLEEP & r_ptr->cdefense) || (randint(MAX_MONS_LEVEL) < r_ptr->level)) {
        verb = " is unaffected.";
    } else {
        if (m_ptr->confused) {
            m_ptr->confused += 3;
        } else {
            m_ptr->confused = 2 + randint(16);
        }
        verb = " appears confused.";
    }
    msg_print(CONCAT(cdesc, verb));
    if (visible && !player_is_dead() && randint(4) == 1) {
        recall_update_characteristics(m_ptr->creature, CD_NO_SLEEP);
    }
}
