// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Player skills: disarm, stealth, saving throw, search, infra-vision range, and speed.

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_skills.h"

// Macro for simple single-value skills with get/set/adjust pattern.
// Each skill has a static int16_t field and three functions.
#define PLAYER_SKILL_IMPL(name, field_name) \
    static int16_t field_name; \
    int player_##name(void) { \
        return field_name; \
    } \
    void player_##name##_set(int value) { \
        field_name = (int16_t)value; \
    } \
    void player_##name##_adjust(int amount) { \
        field_name = (int16_t)(field_name + amount); \
    }

// Disarm skill - percentage-like chance (0-200).
// Race base + creation-time dexterity bonus, then class modifier.
PLAYER_SKILL_IMPL(disarm, the_disarm_chance)

// Stealth - number of halvings (-1 to 18).
// Race base, then class modifier, then gear adjustments.
PLAYER_SKILL_IMPL(stealth, the_stealth)

// Saving throw - resistance percentage (~0-100+).
// Race base, then class and level modifiers.
PLAYER_SKILL_IMPL(saving_throw, the_saving_throw_chance)

// Infra-vision range - squares visible in dark (0-5).
// Race base, then gear adjustments.
PLAYER_SKILL_IMPL(infra_range, the_infra_range)

// Speed - movement rate modifier (negative=slower, 0=normal, positive=faster).
// No racial/class base, only from gear and temporary effects.
PLAYER_SKILL_IMPL(speed, the_speed)

// Search skill - two values, so handle manually.
// Chance is percentage-like for finding hidden things.
// Frequency is turns between automatic searches (smaller is better, can be negative).
static int16_t the_search_chance;
static int16_t the_search_frequency;

int player_search_chance(void) {
    return the_search_chance;
}

int player_search_frequency(void) {
    // Smaller is better and can be negative (Halfling starts at -5).
    return the_search_frequency;
}

void player_search_chance_set(int chance) {
    // Creation, wizard screen, and save file.
    the_search_chance = (int16_t)chance;
}

void player_search_frequency_set(int frequency) {
    // Same three callers, but wizard.c sets chance without setting frequency.
    the_search_frequency = (int16_t)frequency;
}

void player_search_skill_adjust(int chance_amount, int frequency_amount) {
    // Class modifiers at creation, and gear on/off.
    // Two amounts because caller owns the signs:
    // py_bonuses() passes (amount, -amount), creation passes (msrh, mfos).
    the_search_chance = (int16_t)(the_search_chance + chance_amount);
    the_search_frequency = (int16_t)(the_search_frequency + frequency_amount);
}
