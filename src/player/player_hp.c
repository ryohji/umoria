// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The player's hit points

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_hp.h"

// This module knows nothing about levels, the constitution, poison, resting or
// what dying does -- the caller works out what the maximum should be and how
// fast the store refills, and hands over a number. It prints nothing and raises
// no status flag.

// PLAYER_REGEN_HPBASE and the factor the caller passes are both in 65536ths,
// which is why everything below shifts by 16.
#define HP_FRACTION_FULL 0x10000L

// The three numbers themselves. Nothing outside this file can reach them. All
// three start at nothing: the real values arrive when create.c makes a character,
// or when the save file is read. A START OF ZERO IS NOT THE MARK OF DEATH -- the
// mark is a negative number, so a character who has not been made yet is alive.
static int16_t max_hp = 0;
static int16_t current_hp = 0;
static uint16_t hp_fraction = 0;

int16_t player_hp(void) {
    return current_hp;
}

int16_t player_max_hp(void) {
    return max_hp;
}

uint16_t player_hp_fraction(void) {
    return hp_fraction;
}

bool player_hp_marks_death(void) {
    return current_hp < 0;
}

bool player_take_hp_damage(int damage) {
    // Nothing is clamped on purpose. A fatal wound has to leave the number
    // negative, because that is the only record anyone keeps of the death --
    // including the save file.
    current_hp = (int16_t)(current_hp - damage);
    return player_hp_marks_death();
}

bool player_heal_hp(int amount) {
    if (current_hp >= max_hp) {
        // Already at the top: not healed at all, and the caller stays quiet.
        return false;
    }

    current_hp = (int16_t)(current_hp + amount);

    if (current_hp > max_hp) {
        current_hp = max_hp;

        // A full store has nothing on its way back.
        hp_fraction = 0;
    }

    return true;
}

void player_regenerate_hp(int percent) {
    int16_t before = current_hp;
    int32_t gain = (int32_t)max_hp * percent + PLAYER_REGEN_HPBASE;

    current_hp = (int16_t)(current_hp + (gain >> 16));

    // A maximum large enough to hand back more than a short can hold comes
    // round negative; the old code pinned it at the top instead.
    if (current_hp < 0 && before > 0) {
        current_hp = MAX_SHORT;
    }

    int32_t carried = (gain & 0xFFFF) + hp_fraction;
    if (carried >= HP_FRACTION_FULL) {
        hp_fraction = (uint16_t)(carried - HP_FRACTION_FULL);
        current_hp = (int16_t)(current_hp + 1);
    } else {
        hp_fraction = (uint16_t)carried;
    }

    if (current_hp >= max_hp) {
        current_hp = max_hp;

        // "must set frac to zero even if equal" -- the old code's words.
        hp_fraction = 0;
    }
}

bool player_change_max_hp(int16_t new_max) {
    if (max_hp == new_max) {
        return false;
    }

    if (max_hp == 0) {
        // Still being made. The old code did not even store the new maximum
        // here -- create.c fills both numbers in a moment later. This is where
        // the mana's window and this one part company: an empty mana store
        // starts full, an empty hit point store is not touched at all.
        return false;
    }

    // What is left keeps its share of the new maximum. Divide before
    // multiplying to stay inside the long, at the cost of a little accuracy --
    // the old code's words.
    int32_t value = (((int32_t)current_hp << 16) + hp_fraction) / max_hp * new_max;
    current_hp = (int16_t)(value >> 16);
    hp_fraction = (uint16_t)(value & 0xFFFF);

    max_hp = new_max;
    return true;
}

void player_gain_temporary_max_hp(int16_t bonus) {
    // Both numbers move together, so how far from the top the character is does
    // not change, and the fraction is left where it is.
    max_hp = (int16_t)(max_hp + bonus);
    current_hp = (int16_t)(current_hp + bonus);
}

bool player_lose_temporary_max_hp(int16_t bonus) {
    max_hp = (int16_t)(max_hp - bonus);

    if (current_hp > max_hp) {
        current_hp = max_hp;
        hp_fraction = 0;
        return true;
    }

    return false;
}

void player_reset_hp(int16_t max) {
    max_hp = max;
    current_hp = max;
    hp_fraction = 0;
}

bool player_resurrect_hp(void) {
    if (!player_hp_marks_death()) {
        return false;
    }

    current_hp = 0;
    hp_fraction = 0;
    return true;
}

void player_set_hp(int16_t value) {
    current_hp = value;
}

void player_set_max_hp(int16_t value) {
    max_hp = value;
}

void player_set_hp_fraction(uint16_t value) {
    hp_fraction = value;
}
