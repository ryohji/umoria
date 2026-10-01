// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Where each level's monsters sit in the definition table
//
// The building of the index came from init_m_level() in main.c, where it was
// static. Moving it here made it testable.

#include "config.h"
#include "constant.h"
#include "types.h"
#include "externs.h"

#include "monster_levels.h"

// m_level[L] counts the monsters at level L or shallower, so level L's band runs
// from m_level[L - 1] to m_level[L] - 1. It starts out as zeroes: until
// monster_levels_init() runs, every band is empty, which is why main() calls it
// before the first level is generated.
static int16_t m_level[MAX_MONS_LEVEL + 1];

// Initializes M_LEVEL array for use with PLACE_MONSTER -RAK-
void monster_levels_init(void) {
    for (int i = 0; i <= MAX_MONS_LEVEL; i++) {
        m_level[i] = 0;
    }

    const creature_rev_iterator end = monster_creature_rend();
    for (creature_rev_iterator it = monster_creature_rbegin(); !monster_creature_rsame(it, end); it = monster_creature_rnext(it)) {
        const uint8_t level = monster_creature_rget(it)->level;
        if (level <= MAX_MONS_LEVEL) {
            m_level[level] += 1;
        }
    }

    for (int i = 1; i <= MAX_MONS_LEVEL; i++) {
        m_level[i] += m_level[i - 1];
    }
}

int monsters_up_to_level(int level) {
    return m_level[level];
}

int monsters_at_level(int level) {
    return m_level[level] - m_level[level - 1];
}

int first_monster_at_level(int level) {
    return m_level[level - 1];
}
