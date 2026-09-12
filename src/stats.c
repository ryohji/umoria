// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The tables that turn a player stat into a bonus

#include <stddef.h>

#include "config.h"
#include "constant.h"
#include "types.h"

#include "stats.h"

// No externs.h here, the same as panel.c, stores.c and options.c: a table
// lookup needs nothing from the rest of the game. That leaves one symbol to
// declare, so it is declared here rather than dragging in the global header
// (which pulls ncurses along with it) for a single line. The declaration
// therefore appears twice, here and in externs.h; the definition is still the
// one in player.c, so the two can not drift into different objects. Whoever
// makes the player a parameter instead of a global deletes this line.
extern player_type py;

// A step in a stat-to-bonus table: `bonus` applies from `min_stat` upward,
// until the next entry's `min_stat`. Entries must be in ascending order of
// `min_stat`, and the first entry must be 0 so that every stat value matches.
typedef struct {
    uint8_t min_stat;
    int bonus;
} stat_bonus_step;

// Look up the bonus for `stat` in an ascending table.
// Callers pass `count` via STAT_BONUS_STEPS() so it always matches the table.
static int lookup_stat_bonus(uint8_t stat, const stat_bonus_step *table, size_t count) {
    int bonus = table[0].bonus;

    for (size_t i = 1; i < count && stat >= table[i].min_stat; i++) {
        bonus = table[i].bonus;
    }

    return bonus;
}

// Derive the element count from the table itself, so the two can not drift.
#define STAT_BONUS_STEPS(table) ((sizeof(table)) / (sizeof((table)[0])))

// Adjustment for wisdom/intelligence -JWT-
int stat_adj(int stat) {
    // The original chain tested `value > N` in descending order, so each
    // bonus started at N + 1: `> 117` becomes a lower bound of 118, and
    // so on. Note this is the only adjustment that never returns a
    // penalty -- its floor is 0, not a negative value.
    static const stat_bonus_step by_stat[] = {
        {0, 0}, {8, 1}, {15, 2}, {18, 3}, {68, 4}, {88, 5}, {108, 6}, {118, 7},
    };

    return lookup_stat_bonus(py.stats.use_stat[stat], by_stat, STAT_BONUS_STEPS(by_stat));
}

// Adjustment for charisma -RAK-
// Percent decrease or increase in price of goods
int chr_adj(void) {
    // Percent of the base price. Below 3 the switch fell through to its
    // default of 100, which is why the table starts there. Steps are wide
    // at the low end (130 -> 125 -> 122) and narrow to one point around
    // 15..18, then widen again above 19.
    static const stat_bonus_step by_charisma[] = {
        {0, 100},  {3, 130},  {4, 125},  {5, 122},  {6, 120},  {7, 118},
        {8, 116},  {9, 114},  {10, 112}, {11, 110}, {12, 108}, {13, 106},
        {14, 104}, {15, 103}, {16, 102}, {17, 101}, {18, 100}, {19, 98},
        {68, 96},  {88, 94},  {108, 92}, {118, 90},
    };

    return lookup_stat_bonus(py.stats.use_stat[A_CHR], by_charisma, STAT_BONUS_STEPS(by_charisma));
}

// Returns a character's adjustment to hit points -JWT-
int con_adj(void) {
    int con = py.stats.use_stat[A_CON];

    if (con < 7) {
        return (con - 7);
    } else if (con < 17) {
        return 0;
    } else if (con == 17) {
        return 1;
    } else if (con < 94) {
        return 2;
    } else if (con < 117) {
        return 3;
    } else {
        return 4;
    }
}

// Returns a character's adjustment to hit. -JWT-
int tohit_adj(void) {
    // Dexterity and strength contribute independently; the two are summed.
    // Note the top steps do not agree: dexterity's runs from 118, strength's
    // from 117.
    static const stat_bonus_step by_dexterity[] = {
        {0, -3}, {4, -2}, {6, -1}, {8, 0}, {16, 1}, {17, 2}, {18, 3}, {69, 4}, {118, 5},
    };
    static const stat_bonus_step by_strength[] = {
        {0, -3}, {4, -2}, {5, -1}, {7, 0}, {18, 1}, {94, 2}, {109, 3}, {117, 4},
    };

    return lookup_stat_bonus(py.stats.use_stat[A_DEX], by_dexterity, STAT_BONUS_STEPS(by_dexterity)) +
           lookup_stat_bonus(py.stats.use_stat[A_STR], by_strength, STAT_BONUS_STEPS(by_strength));
}

// Returns a character's adjustment to armor class -JWT-
int toac_adj(void) {
    static const stat_bonus_step by_dexterity[] = {
        {0, -4}, {4, -3}, {5, -2}, {6, -1}, {7, 0}, {15, 1}, {18, 2}, {59, 3}, {94, 4}, {117, 5},
    };

    return lookup_stat_bonus(py.stats.use_stat[A_DEX], by_dexterity, STAT_BONUS_STEPS(by_dexterity));
}

// Returns a character's adjustment to disarm -RAK-
int todis_adj(void) {
    // Note the gaps: 2 is followed by 4, and 6 by 8. Unlike the other
    // adjustments this one does not step by 1, so 3 and 7 never occur.
    static const stat_bonus_step by_dexterity[] = {
        {0, -8}, {4, -6}, {5, -4}, {6, -2}, {7, -1}, {8, 0},
        {13, 1}, {16, 2}, {18, 4}, {59, 5}, {94, 6}, {117, 8},
    };

    return lookup_stat_bonus(py.stats.use_stat[A_DEX], by_dexterity, STAT_BONUS_STEPS(by_dexterity));
}

// Returns a character's adjustment to damage -JWT-
int todam_adj(void) {
    static const stat_bonus_step by_strength[] = {
        {0, -2}, {4, -1}, {5, 0}, {16, 1}, {17, 2}, {18, 3}, {94, 4}, {109, 5}, {117, 6},
    };

    return lookup_stat_bonus(py.stats.use_stat[A_STR], by_strength, STAT_BONUS_STEPS(by_strength));
}
