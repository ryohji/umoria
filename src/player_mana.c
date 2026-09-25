// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The player's mana

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_mana.h"

// No externs.h here, the same as player_food.c and the rest. This module knows
// nothing about spells, classes, stats, hunger or resting -- the caller works
// out what the maximum should be and how fast the store refills, and hands over
// a number. It prints nothing and raises no status flag.

// PLAYER_REGEN_MNBASE and the factor the caller passes are both in 65536ths,
// which is why everything below shifts by 16.
#define MANA_FRACTION_FULL 0x10000L

// The three numbers themselves (#18-12-4C). They used to be py.misc.mana,
// py.misc.cmana and py.misc.cmana_frac, and nothing outside this file can reach
// them now. All three start at nothing, exactly as they did inside the
// uninitialised struct: the real values arrive when calc_mana() sees the first
// spell learned, or when the save file is read.
static int16_t max_mana = 0;
static int16_t current_mana = 0;
static uint16_t mana_fraction = 0;

int16_t player_mana(void) {
    return current_mana;
}

int16_t player_max_mana(void) {
    return max_mana;
}

uint16_t player_mana_fraction(void) {
    return mana_fraction;
}

int player_spend_mana(int cost) {
    if (cost > current_mana) {
        // Not enough: the store empties, and so does the fraction. The caller
        // is told how little it got.
        int spent = current_mana;
        current_mana = 0;
        mana_fraction = 0;
        return spent;
    }

    // Enough: the fraction is left alone, so progress towards the next point
    // survives being spent.
    current_mana = (int16_t)(current_mana - cost);
    return cost;
}

void player_regenerate_mana(int percent) {
    int16_t before = current_mana;
    int32_t gain = (int32_t)max_mana * percent + PLAYER_REGEN_MNBASE;

    current_mana = (int16_t)(current_mana + (gain >> 16));

    // A maximum large enough to hand back more than a short can hold comes
    // round negative; the old code pinned it at the top instead.
    if (current_mana < 0 && before > 0) {
        current_mana = MAX_SHORT;
    }

    int32_t carried = (gain & 0xFFFF) + mana_fraction;
    if (carried >= MANA_FRACTION_FULL) {
        mana_fraction = (uint16_t)(carried - MANA_FRACTION_FULL);
        current_mana = (int16_t)(current_mana + 1);
    } else {
        mana_fraction = (uint16_t)carried;
    }

    if (current_mana >= max_mana) {
        current_mana = max_mana;

        // A full store has nothing on its way back.
        mana_fraction = 0;
    }
}

bool player_restore_mana(void) {
    if (current_mana >= max_mana) {
        return false;
    }

    // The fraction is deliberately left where it is: the potion says nothing
    // about it, and neither did the old code.
    current_mana = max_mana;
    return true;
}

bool player_change_max_mana(int16_t new_max) {
    if (max_mana == new_max) {
        return false;
    }

    if (max_mana != 0) {
        // What is left keeps its share of the new maximum. Divide before
        // multiplying to stay inside the long, at the cost of a little
        // accuracy -- the old code's words.
        int32_t value = (((int32_t)current_mana << 16) + mana_fraction) / max_mana * new_max;
        current_mana = (int16_t)(value >> 16);
        mana_fraction = (uint16_t)(value & 0xFFFF);
    } else {
        // Nothing to carry across: a character who had no mana starts full.
        current_mana = new_max;
        mana_fraction = 0;
    }

    max_mana = new_max;
    return true;
}

bool player_lose_all_mana(void) {
    if (max_mana == 0) {
        return false;
    }

    max_mana = 0;
    current_mana = 0;

    // The fraction is left alone, as it was before: nothing reads it while the
    // maximum is zero, because regeneration is not even attempted then, and the
    // next maximum to arrive comes through the branch above that clears it.
    return true;
}

void player_set_mana(int16_t value) {
    current_mana = value;
}

void player_set_max_mana(int16_t value) {
    max_mana = value;
}

void player_set_mana_fraction(uint16_t value) {
    mana_fraction = value;
}
