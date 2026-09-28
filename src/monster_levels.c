// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Where each level's monsters sit in the definition table
//
// The building of the index came from init_m_level() in main.c, where it was
// static -- so the counting had never been reachable from a test. Moving it here
// is what made it testable (the same story as init_t_level() in #18-10).

#include "config.h"
#include "constant.h"
#include "types.h"

#include "monster_levels.h"

// This file needs one thing from outside: a walk over the monster definitions.
// The table itself is a static of monsters.c and has been since #17, so there is
// no array to declare here -- only the five calls of the walk. They are declared
// here rather than by including externs.h, which drags in ncurses for the sake
// of one loop (the same choice object_levels.c and stats.c made).
creature_rev_iterator monster_creature_rbegin(void);
creature_rev_iterator monster_creature_rend(void);
bool monster_creature_rsame(creature_rev_iterator a, creature_rev_iterator b);
creature_rev_iterator monster_creature_rnext(creature_rev_iterator it);
creature_type *monster_creature_rget(creature_rev_iterator it);

// m_level[L] counts the monsters at level L or shallower, so level L's band runs
// from m_level[L - 1] to m_level[L] - 1. The name came over from monsters.c
// (#18-14-2) with the type -- `int16_t m_level[MAX_MONS_LEVEL + 1];` -- and so
// did the fact that it starts out as zeroes: until monster_levels_init() runs,
// every band is empty, which is why main() calls it before the first cave is
// generated.
//
// FOR ONE STEP THERE WERE TWO OF THESE, as with #18-14-1: the row went in here
// at #18-14-2A while the game still built and read monsters.c's m_level and
// nothing called these windows, safe only because the unit test was the only
// reader. #18-14-2B pointed main.c, misc1.c and spells.c at the windows, which
// moved the storage as well as the call sites, and #18-14-2C deleted the old
// array from monsters.c and externs.h.
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
