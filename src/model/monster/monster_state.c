// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Monster state: levels index, the list of monsters on the current level, and recall

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"
#include "monsters.h"

#include "monster_state.h"

// --- From monster_levels.c: level index ---

// m_level[L] counts the monsters at level L or shallower, so level L's band runs
// from m_level[L - 1] to m_level[L] - 1. It starts out as zeroes: until
// monster_levels_init() runs, every band is empty, which is why main() calls it
// before the first level is generated.
static int16_t m_level[MAX_MONS_LEVEL + 1];

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

// --- From monster_list.c: the list on this level ---

// THE STORAGE. Two tables that have nothing to do with each other: the creature
// definition table says what a kind of monster is; this one says which monsters
// are standing on this level.
//
// The two names are one container: a table whose rows are PACKED, and the mark
// saying how much of it is in use.
static monster_type the_monsters[MAX_MALLOC];
static int16_t the_mark;

// The value an empty row holds. It stays in monsters.c among the constant
// tables, and this file is now its ONLY reader (both uses are below). It is not
// this question's subject -- it belongs to the read-only-data group -- so it is
// left where it is and noted for that group: a "blank monster" is the monster
// table's notion of an empty row, so its home is here, and moving it would make
// this module need no externs at all.

void monster_list_reset(void) {
    for (int i = 0; i < MAX_MALLOC; i++) {
        the_monsters[i] = blank_monster;
    }
    the_mark = MIN_MONIX;
}

monster_type *monster_list_at(int index) { return &the_monsters[index]; }

int16_t monster_list_used(void) { return the_mark; }

void set_monster_list_used(int16_t used) { the_mark = used; }

bool monster_list_is_full(void) { return the_mark == MAX_MALLOC; }

int monster_list_free_slots(void) { return MAX_MALLOC - the_mark; }

int monster_list_claim_slot(void) { return the_mark++; }

void monster_list_drop_last(void) {
    the_monsters[the_mark - 1] = blank_monster;
    the_mark -= 1;
}

// --- From monster_recall.c: what the player has learned ---

static recall_type c_recall[MAX_CREATURES]; // Monster memories

recall_type *recall_get(creature_iterator h) {
    return c_recall + monster_creature_index(h);
}

void recall_update_characteristics(creature_iterator h, int defence) {
    recall_get(h)->r_cdefense |= defence;
}

void recall_update_move(creature_iterator h, int move) {
    recall_get(h)->r_cmove |= move;
}

static inline uint8_t get_carry(uint32_t cmove) {
    return (cmove & CM_TREASURE) >> CM_TR_SHIFT;
}

void recall_update_carry(creature_iterator h, uint8_t number) {
    recall_type *const recall = recall_get(h);
    uint32_t current = get_carry(recall->r_cmove);
    recall->r_cmove &= ~CM_TREASURE;
    recall->r_cmove |= MAX(current, number) << CM_TR_SHIFT;
}

void recall_update_spell(creature_iterator h, uint32_t type) {
    recall_type *const recall = recall_get(h);
    recall->r_spells |= type;
}

void recall_increment_spell_chance(creature_iterator h) {
    recall_type *const recall = recall_get(h);
    if ((recall->r_spells & CS_FREQ) != CS_FREQ) {
        recall->r_spells += 1;
    }
}

void recall_increment_kill(creature_iterator h) {
    recall_type *const recall = recall_get(h);
    if (recall->r_kills < MAX_SHORT) {
        recall->r_kills += 1;
    }
}

void recall_increment_death(creature_iterator h) {
    recall_type *const recall = recall_get(h);
    if (recall->r_deaths < MAX_SHORT) {
        recall->r_deaths += 1;
    }
}
