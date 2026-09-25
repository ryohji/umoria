// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The player's hit points

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_hp.h"

// No externs.h here, the same as player_mana.c and the rest. This module knows
// nothing about levels, the constitution, poison, resting or what dying does --
// the caller works out what the maximum should be and how fast the store
// refills, and hands over a number. It prints nothing and raises no status flag.

// The place is still py.misc for now (#18-12-5A); the three fields become
// statics here in #18-12-5C.
extern player_type py;

// PLAYER_REGEN_HPBASE and the factor the caller passes are both in 65536ths,
// which is why everything below shifts by 16.
#define HP_FRACTION_FULL 0x10000L

int16_t player_hp(void) {
    return py.misc.chp;
}

int16_t player_max_hp(void) {
    return py.misc.mhp;
}

uint16_t player_hp_fraction(void) {
    return py.misc.chp_frac;
}

bool player_hp_marks_death(void) {
    return py.misc.chp < 0;
}

bool player_take_hp_damage(int damage) {
    // Nothing is clamped on purpose. A fatal wound has to leave the number
    // negative, because that is the only record anyone keeps of the death --
    // including the save file.
    py.misc.chp = (int16_t)(py.misc.chp - damage);
    return player_hp_marks_death();
}

bool player_heal_hp(int amount) {
    if (py.misc.chp >= py.misc.mhp) {
        // Already at the top: not healed at all, and the caller stays quiet.
        return false;
    }

    py.misc.chp = (int16_t)(py.misc.chp + amount);

    if (py.misc.chp > py.misc.mhp) {
        py.misc.chp = py.misc.mhp;

        // A full store has nothing on its way back.
        py.misc.chp_frac = 0;
    }

    return true;
}

void player_regenerate_hp(int percent) {
    int16_t before = py.misc.chp;
    int32_t gain = (int32_t)py.misc.mhp * percent + PLAYER_REGEN_HPBASE;

    py.misc.chp = (int16_t)(py.misc.chp + (gain >> 16));

    // A maximum large enough to hand back more than a short can hold comes
    // round negative; the old code pinned it at the top instead.
    if (py.misc.chp < 0 && before > 0) {
        py.misc.chp = MAX_SHORT;
    }

    int32_t carried = (gain & 0xFFFF) + py.misc.chp_frac;
    if (carried >= HP_FRACTION_FULL) {
        py.misc.chp_frac = (uint16_t)(carried - HP_FRACTION_FULL);
        py.misc.chp = (int16_t)(py.misc.chp + 1);
    } else {
        py.misc.chp_frac = (uint16_t)carried;
    }

    if (py.misc.chp >= py.misc.mhp) {
        py.misc.chp = py.misc.mhp;

        // "must set frac to zero even if equal" -- the old code's words.
        py.misc.chp_frac = 0;
    }
}

bool player_change_max_hp(int16_t new_max) {
    if (py.misc.mhp == new_max) {
        return false;
    }

    if (py.misc.mhp == 0) {
        // Still being made. The old code did not even store the new maximum
        // here -- create.c fills both numbers in a moment later. This is where
        // the mana's window and this one part company: an empty mana store
        // starts full, an empty hit point store is not touched at all.
        return false;
    }

    // What is left keeps its share of the new maximum. Divide before
    // multiplying to stay inside the long, at the cost of a little accuracy --
    // the old code's words.
    int32_t value = (((int32_t)py.misc.chp << 16) + py.misc.chp_frac) / py.misc.mhp * new_max;
    py.misc.chp = (int16_t)(value >> 16);
    py.misc.chp_frac = (uint16_t)(value & 0xFFFF);

    py.misc.mhp = new_max;
    return true;
}

void player_gain_temporary_max_hp(int16_t bonus) {
    // Both numbers move together, so how far from the top the character is does
    // not change, and the fraction is left where it is.
    py.misc.mhp = (int16_t)(py.misc.mhp + bonus);
    py.misc.chp = (int16_t)(py.misc.chp + bonus);
}

bool player_lose_temporary_max_hp(int16_t bonus) {
    py.misc.mhp = (int16_t)(py.misc.mhp - bonus);

    if (py.misc.chp > py.misc.mhp) {
        py.misc.chp = py.misc.mhp;
        py.misc.chp_frac = 0;
        return true;
    }

    return false;
}

void player_reset_hp(int16_t max) {
    py.misc.mhp = max;
    py.misc.chp = max;
    py.misc.chp_frac = 0;
}

bool player_resurrect_hp(void) {
    if (!player_hp_marks_death()) {
        return false;
    }

    py.misc.chp = 0;
    py.misc.chp_frac = 0;
    return true;
}

void player_set_hp(int16_t value) {
    py.misc.chp = value;
}

void player_set_max_hp(int16_t value) {
    py.misc.mhp = value;
}

void player_set_hp_fraction(uint16_t value) {
    py.misc.chp_frac = value;
}
