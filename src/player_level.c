// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How far the character has come: the level and the experience

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_level.h"

// No externs.h here, the same as player_hp.c and the rest. This module knows
// nothing about what a level is worth, about messages, about the hit points, the
// spells or the mana, and it draws nothing. It works out what the experience has
// paid for; the caller decides what that buys.
extern player_type py;

// The price list. It belongs to another global for now (src/player.c) and is one
// of the fifty-three left in externs.h -- the five readers of it are all inside
// this question, so it could come in here later, the way the hit-point table
// came into hp_table.c. Until then this declaration is the one line that reaches
// out, the same arrangement stats.c and level_exit.c have.
extern uint32_t player_exp[MAX_PLAYER_LEVEL];

// The remainder is kept in 65536ths, so this is a whole point of experience.
#define EXPERIENCE_FRACTION_FULL 0x10000L

uint16_t player_level(void) {
    return py.misc.lev;
}

int32_t player_experience(void) {
    return py.misc.exp;
}

int32_t player_max_experience(void) {
    return py.misc.max_exp;
}

uint16_t player_experience_fraction(void) {
    return py.misc.exp_frac;
}

uint8_t player_experience_factor(void) {
    return py.misc.expfact;
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
    return (int32_t)(player_exp[level - 1] * py.misc.expfact / 100);
}

int32_t player_experience_needed_to_advance(void) {
    return player_experience_to_advance_from(py.misc.lev);
}

void player_gain_experience(int32_t amount) {
    py.misc.exp += amount;
}

// A dead monster is worth mexp * its level, shared out by how far the character
// has already come. The part that does not divide evenly is kept, so that a
// high-level character killing something small still creeps towards the next
// point instead of collecting nothing at all, for ever.
//
// The divisor is the level, so this must not be called with a level of nothing.
// The one caller (mon_take_hit) cannot be reached before a character exists.
void player_gain_shared_experience(int32_t total) {
    int32_t whole = total / py.misc.lev;
    int32_t fraction = (total % py.misc.lev) * EXPERIENCE_FRACTION_FULL / py.misc.lev + py.misc.exp_frac;

    if (fraction >= EXPERIENCE_FRACTION_FULL) {
        whole++;
        py.misc.exp_frac = (uint16_t)(fraction - EXPERIENCE_FRACTION_FULL);
    } else {
        py.misc.exp_frac = (uint16_t)fraction;
    }

    py.misc.exp += whole;
}

// Nothing is the floor, and the fraction is left alone: losing experience does
// not throw away the part of a point that has been collected.
void player_lose_experience(int32_t amount) {
    if (amount > py.misc.exp) {
        py.misc.exp = 0;
    } else {
        py.misc.exp -= amount;
    }
}

bool player_cap_experience(void) {
    if (py.misc.exp > MAX_EXP) {
        py.misc.exp = MAX_EXP;
        return true;
    }

    return false;
}

bool player_deserves_next_level(void) {
    if (py.misc.lev >= MAX_PLAYER_LEVEL) {
        return false;
    }

    return player_experience_to_advance_from(py.misc.lev) <= py.misc.exp;
}

void player_advance_level(void) {
    py.misc.lev++;
}

// Arriving at a level with more experience than it costs means several levels
// were paid for at once (or the level was handed over by something other than
// the experience). Half of the surplus goes.
void player_trim_surplus_experience(void) {
    int32_t need_exp = player_experience_to_advance_from(py.misc.lev);

    if (py.misc.exp > need_exp) {
        int32_t dif_exp = py.misc.exp - need_exp;
        py.misc.exp = need_exp + (dif_exp / 2);
    }
}

bool player_record_max_experience(void) {
    if (py.misc.exp > py.misc.max_exp) {
        py.misc.max_exp = py.misc.exp;
        return true;
    }

    return false;
}

bool player_restore_experience(void) {
    if (py.misc.max_exp > py.misc.exp) {
        py.misc.exp = py.misc.max_exp;
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

    while (player_experience_to_advance_from((uint16_t)level) <= py.misc.exp) {
        level++;
    }

    return (uint16_t)level;
}

bool player_recompute_level(void) {
    uint16_t deserved = player_level_deserved_by_experience();

    if (py.misc.lev == deserved) {
        return false;
    }

    py.misc.lev = deserved;

    return true;
}

void player_set_level(uint16_t value) {
    py.misc.lev = value;
}

void player_set_experience(int32_t value) {
    py.misc.exp = value;
}

void player_set_max_experience(int32_t value) {
    py.misc.max_exp = value;
}

void player_set_experience_fraction(uint16_t value) {
    py.misc.exp_frac = value;
}

void player_set_experience_factor(uint8_t value) {
    py.misc.expfact = value;
}
