// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// What the character can do and resist because of what is being worn

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_abilities.h"

// No externs.h here, the same as the seven questions before this one. This module
// does not know what any of the seventeen answers leads to -- no messages, no
// rolls, no drawing.

// The seventeen, IN THE ORDER THE SAVE FILE KEEPS THEM. save.c writes them as
// seventeen bytes in one run, so this order is the file format and cannot move;
// player_abilities_saved_byte() turns a position in the file into one of these.
typedef enum {
    ABILITY_SEE_INVISIBLE,
    ABILITY_TELEPORT_AT_RANDOM,
    ABILITY_NEVER_PARALYZED,
    ABILITY_SLOW_DIGESTION,
    ABILITY_AGGRAVATE,
    ABILITY_RESIST_FIRE,
    ABILITY_RESIST_COLD,
    ABILITY_RESIST_ACID,
    ABILITY_REGENERATE,
    ABILITY_RESIST_LIGHT,
    ABILITY_NO_FALLING_DAMAGE,
    ABILITY_SUSTAIN_STR,
    ABILITY_SUSTAIN_INT,
    ABILITY_SUSTAIN_WIS,
    ABILITY_SUSTAIN_CON,
    ABILITY_SUSTAIN_DEX,
    ABILITY_SUSTAIN_CHR,
    ABILITY_COUNT
} player_ability;

// The count and the save file's run of bytes are the same thing said twice, so
// say so once and for all: adding a member without widening the run would write
// a byte nobody reads back.
_Static_assert(ABILITY_COUNT == PLAYER_ABILITIES_SAVED_BYTES, "the seventeen are seventeen bytes");

// THE SEVENTEEN ANSWERS THEMSELVES. They were py.flags.see_inv through
// py.flags.sustain_chr until #18-12-8C; now this array is the only place they
// live, and nothing outside this file can name one of them.
//
// All zero at the start, which is the same start the seventeen fields had: A
// CHARACTER CAN DO NOTHING AND RESIST NOTHING until calc_bonuses() works the
// answers out from the equipment, or the save file's seventeen bytes are read
// back in.
//
// A byte, not a bool: a byte read out of a save file is written back unchanged
// (a 5 stays a 5), and anything not zero counts as having the ability.
static uint8_t ability_held[ABILITY_COUNT];

static bool has(player_ability ability) {
    return ability_held[ability] != 0;
}

static void grant(player_ability ability) {
    ability_held[ability] = true;
}

// The eleven that a worn item can grant, each with the flag that grants it. THIS
// TABLE IS THE ONLY PLACE A TR_* CONSTANT IS NAMED, which is what lets
// calc_bonuses() hand over one word and say nothing about what is in it. The
// order is the order player_bonuses.c used to test them in; nothing depends on it,
// because granting one ability never affects another.
static const struct {
    uint32_t item_flag;
    player_ability ability;
} granted_by_item[] = {
    {TR_SLOW_DIGEST, ABILITY_SLOW_DIGESTION},
    {TR_AGGRAVATE, ABILITY_AGGRAVATE},
    {TR_TELEPORT, ABILITY_TELEPORT_AT_RANDOM},
    {TR_REGEN, ABILITY_REGENERATE},
    {TR_RES_FIRE, ABILITY_RESIST_FIRE},
    {TR_RES_ACID, ABILITY_RESIST_ACID},
    {TR_RES_COLD, ABILITY_RESIST_COLD},
    {TR_FREE_ACT, ABILITY_NEVER_PARALYZED},
    {TR_SEE_INVIS, ABILITY_SEE_INVISIBLE},
    {TR_RES_LIGHT, ABILITY_RESIST_LIGHT},
    {TR_FFALL, ABILITY_NO_FALLING_DAMAGE},
};

#define GRANTED_BY_ITEM_COUNT ((int)(sizeof granted_by_item / sizeof granted_by_item[0]))

// THE TWO STAT ORDERS. The rest of the game numbers the stats A_STR, A_INT,
// A_WIS, A_DEX, A_CON, A_CHR (constant.h); an item that sustains one stat says
// which in its p1, numbered 1 to 6 as strength, intelligence, wisdom,
// CONSTITUTION, DEXTERITY, charisma. THE FOURTH AND FIFTH ARE SWAPPED between
// the two, so one table cannot serve both and indexing either order by the other
// silently sustains the wrong stat.
#define PLAYER_STAT_COUNT 6

static const player_ability sustain_of_stat[PLAYER_STAT_COUNT] = {
    ABILITY_SUSTAIN_STR, // A_STR
    ABILITY_SUSTAIN_INT, // A_INT
    ABILITY_SUSTAIN_WIS, // A_WIS
    ABILITY_SUSTAIN_DEX, // A_DEX
    ABILITY_SUSTAIN_CON, // A_CON
    ABILITY_SUSTAIN_CHR, // A_CHR
};

static const player_ability sustain_of_item_p1[PLAYER_STAT_COUNT] = {
    ABILITY_SUSTAIN_STR, // p1 == 1
    ABILITY_SUSTAIN_INT, // p1 == 2
    ABILITY_SUSTAIN_WIS, // p1 == 3
    ABILITY_SUSTAIN_CON, // p1 == 4
    ABILITY_SUSTAIN_DEX, // p1 == 5
    ABILITY_SUSTAIN_CHR, // p1 == 6
};

bool player_can_see_invisible(void) {
    return has(ABILITY_SEE_INVISIBLE);
}

bool player_never_paralyzed(void) {
    return has(ABILITY_NEVER_PARALYZED);
}

bool player_resists_fire(void) {
    return has(ABILITY_RESIST_FIRE);
}

bool player_resists_cold(void) {
    return has(ABILITY_RESIST_COLD);
}

bool player_resists_acid(void) {
    return has(ABILITY_RESIST_ACID);
}

bool player_resists_light(void) {
    return has(ABILITY_RESIST_LIGHT);
}

bool player_takes_no_falling_damage(void) {
    return has(ABILITY_NO_FALLING_DAMAGE);
}

bool player_regenerates(void) {
    return has(ABILITY_REGENERATE);
}

bool player_has_slow_digestion(void) {
    return has(ABILITY_SLOW_DIGESTION);
}

bool player_teleports_randomly(void) {
    return has(ABILITY_TELEPORT_AT_RANDOM);
}

bool player_aggravates_monsters(void) {
    return has(ABILITY_AGGRAVATE);
}

bool player_stat_sustained(int stat) {
    if (stat < 0 || stat >= PLAYER_STAT_COUNT) {
        return false;
    }

    return has(sustain_of_stat[stat]);
}

void player_abilities_forget_all(void) {
    for (int i = 0; i < ABILITY_COUNT; i++) {
        ability_held[i] = false;
    }
}

void player_abilities_note_item_flags(uint32_t item_flags) {
    for (int i = 0; i < GRANTED_BY_ITEM_COUNT; i++) {
        if (granted_by_item[i].item_flag & item_flags) {
            grant(granted_by_item[i].ability);
        }
    }
}

void player_abilities_note_sustain(int item_p1) {
    if (item_p1 < 1 || item_p1 > PLAYER_STAT_COUNT) {
        return;
    }

    grant(sustain_of_item_p1[item_p1 - 1]);
}

void player_grant_see_invisible(void) {
    grant(ABILITY_SEE_INVISIBLE);
}

uint8_t player_abilities_saved_byte(int position) {
    if (position < 0 || position >= ABILITY_COUNT) {
        return 0;
    }

    return ability_held[position];
}

void player_abilities_restore_byte(int position, uint8_t value) {
    if (position < 0 || position >= ABILITY_COUNT) {
        return;
    }

    ability_held[position] = value;
}
