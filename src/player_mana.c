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

// The place is still py.misc for now (#18-12-4A); the three fields become
// statics here in #18-12-4C.
extern player_type py;

// PLAYER_REGEN_MNBASE and the factor the caller passes are both in 65536ths,
// which is why everything below shifts by 16.
#define MANA_FRACTION_FULL 0x10000L

int16_t player_mana(void) {
    return py.misc.cmana;
}

int16_t player_max_mana(void) {
    return py.misc.mana;
}

uint16_t player_mana_fraction(void) {
    return py.misc.cmana_frac;
}

int player_spend_mana(int cost) {
    if (cost > py.misc.cmana) {
        // Not enough: the store empties, and so does the fraction. The caller
        // is told how little it got.
        int spent = py.misc.cmana;
        py.misc.cmana = 0;
        py.misc.cmana_frac = 0;
        return spent;
    }

    // Enough: the fraction is left alone, so progress towards the next point
    // survives being spent.
    py.misc.cmana = (int16_t)(py.misc.cmana - cost);
    return cost;
}

void player_regenerate_mana(int percent) {
    int16_t before = py.misc.cmana;
    int32_t gain = (int32_t)py.misc.mana * percent + PLAYER_REGEN_MNBASE;

    py.misc.cmana = (int16_t)(py.misc.cmana + (gain >> 16));

    // A maximum large enough to hand back more than a short can hold comes
    // round negative; the old code pinned it at the top instead.
    if (py.misc.cmana < 0 && before > 0) {
        py.misc.cmana = MAX_SHORT;
    }

    int32_t carried = (gain & 0xFFFF) + py.misc.cmana_frac;
    if (carried >= MANA_FRACTION_FULL) {
        py.misc.cmana_frac = (uint16_t)(carried - MANA_FRACTION_FULL);
        py.misc.cmana = (int16_t)(py.misc.cmana + 1);
    } else {
        py.misc.cmana_frac = (uint16_t)carried;
    }

    if (py.misc.cmana >= py.misc.mana) {
        py.misc.cmana = py.misc.mana;

        // A full store has nothing on its way back.
        py.misc.cmana_frac = 0;
    }
}

bool player_restore_mana(void) {
    if (py.misc.cmana >= py.misc.mana) {
        return false;
    }

    // The fraction is deliberately left where it is: the potion says nothing
    // about it, and neither did the old code.
    py.misc.cmana = py.misc.mana;
    return true;
}

bool player_change_max_mana(int16_t new_max) {
    if (py.misc.mana == new_max) {
        return false;
    }

    if (py.misc.mana != 0) {
        // What is left keeps its share of the new maximum. Divide before
        // multiplying to stay inside the long, at the cost of a little
        // accuracy -- the old code's words.
        int32_t value = (((int32_t)py.misc.cmana << 16) + py.misc.cmana_frac) / py.misc.mana * new_max;
        py.misc.cmana = (int16_t)(value >> 16);
        py.misc.cmana_frac = (uint16_t)(value & 0xFFFF);
    } else {
        // Nothing to carry across: a character who had no mana starts full.
        py.misc.cmana = new_max;
        py.misc.cmana_frac = 0;
    }

    py.misc.mana = new_max;
    return true;
}

bool player_lose_all_mana(void) {
    if (py.misc.mana == 0) {
        return false;
    }

    py.misc.mana = 0;
    py.misc.cmana = 0;

    // The fraction is left alone, as it was before: nothing reads it while the
    // maximum is zero, because regeneration is not even attempted then, and the
    // next maximum to arrive comes through the branch above that clears it.
    return true;
}

void player_set_mana(int16_t value) {
    py.misc.cmana = value;
}

void player_set_max_mana(int16_t value) {
    py.misc.mana = value;
}

void player_set_mana_fraction(uint16_t value) {
    py.misc.cmana_frac = value;
}
