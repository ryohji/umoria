// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How far the character has come: the level and the experience

#include "config.h"
#include "constant.h"
#include "types.h"
#include "externs.h"

#include "player_level.h"

// This module knows nothing about what a level is worth, about messages, about
// the hit points, the spells or the mana, and it draws nothing. It works out what
// the experience has paid for; the caller decides what that buys.
//
// The five numbers are owned here and are static (#18-12-6C): they came out of
// py.misc, which is one field shorter for each of them, and the only way in is
// through the windows in player_level.h. Their initial value is nothing, the same
// as it was inside the uninitialised struct -- a level of zero means no character
// exists yet, and neither the price (player_exp[lev - 1]) nor the share of a
// kill (divided by lev) means anything until creation writes a level of one.
static uint16_t player_level_reached;
static int32_t player_experience_held;
static int32_t player_experience_high_water_mark;
static uint16_t player_experience_remainder;
static uint8_t player_experience_percentage;

// The price list stays where it is. It is one of the twenty read-only constant
// tables in externs.h (with player_title, race, class and class_level_adj), and
// those are out of scope for #18 -- the inventory's own line for them says
// "const-ification only" (the per-group table in
// docs/refactoring/globals_inventory.md). Bringing it in here would take the
// ledger from fifty-three to fifty-two, but it would also need a setter that no
// caller in the game would ever use, because unlike the hit-point table in
// hp_table.c nothing ever writes this one.

// The remainder is kept in 65536ths, so this is a whole point of experience.
#define EXPERIENCE_FRACTION_FULL 0x10000L

uint16_t player_level(void) {
    return player_level_reached;
}

int32_t player_experience(void) {
    return player_experience_held;
}

int32_t player_max_experience(void) {
    return player_experience_high_water_mark;
}

uint16_t player_experience_fraction(void) {
    return player_experience_remainder;
}

uint8_t player_experience_factor(void) {
    return player_experience_percentage;
}

// The table entry is unsigned and so is the factor, so the multiplication and
// the division both happen in unsigned arithmetic and the result is only then
// read as signed -- which is what the `(signed)` casts at the five old call
// sites were doing. There is room: the dearest character pays a hundred and
// sixty-five per cent (a race base of 100, 110, 120 or 125 plus a class share of
// 0, 20, 30, 35 or 40), and 10000000 * 165 is 1.65e9, still inside a signed
// thirty-two-bit number. A factor above about 214 would turn the last few levels
// negative and the character would climb for ever.
int32_t player_experience_to_advance_from(uint16_t level) {
    return (int32_t)(player_exp[level - 1] * player_experience_percentage / 100);
}

int32_t player_experience_needed_to_advance(void) {
    return player_experience_to_advance_from(player_level_reached);
}

void player_gain_experience(int32_t amount) {
    player_experience_held += amount;
}

// A dead monster is worth mexp * its level, shared out by how far the character
// has already come. The part that does not divide evenly is kept, so that a
// high-level character killing something small still creeps towards the next
// point instead of collecting nothing at all, for ever.
//
// The divisor is the level, so this must not be called with a level of nothing.
// The one caller (mon_take_hit) cannot be reached before a character exists.
void player_gain_shared_experience(int32_t total) {
    int32_t whole = total / player_level_reached;
    int32_t fraction = (total % player_level_reached) * EXPERIENCE_FRACTION_FULL / player_level_reached + player_experience_remainder;

    if (fraction >= EXPERIENCE_FRACTION_FULL) {
        whole++;
        player_experience_remainder = (uint16_t)(fraction - EXPERIENCE_FRACTION_FULL);
    } else {
        player_experience_remainder = (uint16_t)fraction;
    }

    player_experience_held += whole;
}

// Nothing is the floor, and the fraction is left alone: losing experience does
// not throw away the part of a point that has been collected.
void player_lose_experience(int32_t amount) {
    if (amount > player_experience_held) {
        player_experience_held = 0;
    } else {
        player_experience_held -= amount;
    }
}

bool player_cap_experience(void) {
    if (player_experience_held > MAX_EXP) {
        player_experience_held = MAX_EXP;
        return true;
    }

    return false;
}

bool player_deserves_next_level(void) {
    if (player_level_reached >= MAX_PLAYER_LEVEL) {
        return false;
    }

    return player_experience_to_advance_from(player_level_reached) <= player_experience_held;
}

void player_advance_level(void) {
    player_level_reached++;
}

// Arriving at a level with more experience than it costs means several levels
// were paid for at once (or the level was handed over by something other than
// the experience). Half of the surplus goes.
void player_trim_surplus_experience(void) {
    int32_t need_exp = player_experience_to_advance_from(player_level_reached);

    if (player_experience_held > need_exp) {
        int32_t dif_exp = player_experience_held - need_exp;
        player_experience_held = need_exp + (dif_exp / 2);
    }
}

bool player_record_max_experience(void) {
    if (player_experience_held > player_experience_high_water_mark) {
        player_experience_high_water_mark = player_experience_held;
        return true;
    }

    return false;
}

bool player_restore_experience(void) {
    if (player_experience_high_water_mark > player_experience_held) {
        player_experience_held = player_experience_high_water_mark;
        return true;
    }

    return false;
}

// Counting from the bottom of the price list, the way lose_exp() has always done
// it. The walk stops because the last entry in the table (ten million) is dearer
// than the ceiling on experience (MAX_EXP, one less than ten million) for every
// factor of a hundred or more -- THAT ONE DIFFERENCE IS THE WHOLE REASON THIS
// DOES NOT READ PAST THE END OF THE TABLE. It is not checked anywhere, here or
// in the original. Nothing is changed about that; it is written down instead.
uint16_t player_level_deserved_by_experience(void) {
    int level = 1;

    while (player_experience_to_advance_from((uint16_t)level) <= player_experience_held) {
        level++;
    }

    return (uint16_t)level;
}

bool player_recompute_level(void) {
    uint16_t deserved = player_level_deserved_by_experience();

    if (player_level_reached == deserved) {
        return false;
    }

    player_level_reached = deserved;

    return true;
}

void player_set_level(uint16_t value) {
    player_level_reached = value;
}

void player_set_experience(int32_t value) {
    player_experience_held = value;
}

void player_set_max_experience(int32_t value) {
    player_experience_high_water_mark = value;
}

void player_set_experience_fraction(uint16_t value) {
    player_experience_remainder = value;
}

void player_set_experience_factor(uint8_t value) {
    player_experience_percentage = value;
}
