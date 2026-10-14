// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// What the character's state is right now: one word of thirty bits

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_status_flags.h"

// No externs.h here: this module reaches nothing outside itself. This module
// knows nothing about what a mark means -- no messages, no counters, no drawing.
// It keeps the set and hides the mask arithmetic.

// THE WORD ITSELF, and the only copy of it. The thirty bits live here and nothing
// outside this file can name them. Zero is where a character starts -- no mark set,
// no request waiting, not searching, not resting. create.c sets the real state when
// it makes a character, and save.c puts the saved word back through
// player_set_status_word().
static uint32_t status;

// The fourteen marks, in the order of the enum. THE MAPPING IS THE ONLY PLACE
// THE BIT VALUES ARE NAMED, which is what lets the callers speak of effects
// instead of bits. The bit order and the enum order are different on purpose:
// nothing outside this table is allowed to depend on either.
static const uint32_t effect_marks[PLAYER_EFFECT_COUNT] = {
    PY_HERO,     PY_SHERO,    PY_BLIND,    PY_CONFUSED, PY_FEAR,
    PY_POISONED, PY_FAST,     PY_SLOW,     PY_INVULN,   PY_BLESSED,
    PY_DET_INV,  PY_TIM_INFRA, PY_HUNGRY,  PY_WEAK,
};

static bool is_set(uint32_t mask) {
    return (status & mask) != 0;
}

static void set_bits(uint32_t mask) {
    status |= mask;
}

static void clear_bits(uint32_t mask) {
    status &= ~mask;
}

// Sets the mask and says whether any of it was new. The callers of the marks and
// of the requests both want this shape: "was this already noted?".
static bool set_and_say_if_new(uint32_t mask) {
    if (is_set(mask)) {
        return false;
    }

    set_bits(mask);

    return true;
}

// Reads the mask and leaves none of it behind.
static bool take(uint32_t mask) {
    if (!is_set(mask)) {
        return false;
    }

    clear_bits(mask);

    return true;
}

static void set_or_clear(uint32_t mask, bool wanted) {
    if (wanted) {
        set_bits(mask);
    } else {
        clear_bits(mask);
    }
}

bool player_effect_in_force(player_effect effect) {
    return is_set(effect_marks[effect]);
}

bool player_note_effect_started(player_effect effect) {
    return set_and_say_if_new(effect_marks[effect]);
}

void player_note_effect_ended(player_effect effect) {
    clear_bits(effect_marks[effect]);
}

void player_note_hunger_satisfied(void) {
    clear_bits(PY_HUNGRY | PY_WEAK);
}

void player_request_strength_check(void) {
    set_bits(PY_STR_WGT);
}

bool player_strength_check_requested(void) {
    return is_set(PY_STR_WGT);
}

void player_clear_strength_check_request(void) {
    clear_bits(PY_STR_WGT);
}

void player_request_study_redraw(void) {
    set_bits(PY_STUDY);
}

bool player_study_redraw_requested(void) {
    return is_set(PY_STUDY);
}

void player_clear_study_redraw_request(void) {
    clear_bits(PY_STUDY);
}

void player_request_speed_redraw(void) {
    set_bits(PY_SPEED);
}

bool player_take_speed_redraw_request(void) {
    return take(PY_SPEED);
}

void player_request_armor_redraw(void) {
    set_bits(PY_ARMOR);
}

bool player_take_armor_redraw_request(void) {
    return take(PY_ARMOR);
}

void player_request_hp_redraw(void) {
    set_bits(PY_HP);
}

bool player_take_hp_redraw_request(void) {
    return take(PY_HP);
}

void player_request_mana_redraw(void) {
    set_bits(PY_MANA);
}

bool player_take_mana_redraw_request(void) {
    return take(PY_MANA);
}

// The six stat requests share one run of adjacent bits, starting at PY_STR and
// masked together as PY_STATS. constant.h warns that the six must stay adjacent;
// THIS IS NOW THE ONLY FILE THAT DEPENDS ON IT (misc3.c shifted the bit,
// dungeon.c shifted it again to read it back, and both used the mask).
static uint32_t stat_mark(int stat) {
    return (uint32_t)PY_STR << stat;
}

void player_request_stat_redraw(int stat) {
    set_bits(stat_mark(stat));
}

bool player_any_stat_redraw_requested(void) {
    return is_set(PY_STATS);
}

bool player_stat_redraw_requested(int stat) {
    return is_set(stat_mark(stat));
}

void player_clear_stat_redraw_requests(void) {
    clear_bits(PY_STATS);
}

bool player_is_searching(void) {
    return is_set(PY_SEARCH);
}

void player_start_searching(void) {
    set_bits(PY_SEARCH);
}

void player_stop_searching(void) {
    clear_bits(PY_SEARCH);
}

bool player_is_resting(void) {
    return is_set(PY_REST);
}

void player_start_resting(void) {
    set_bits(PY_REST);
}

void player_stop_resting(void) {
    clear_bits(PY_REST);
}

bool player_status_line_shows_paralysis(void) {
    return is_set(PY_PARALYSED);
}

void player_set_status_line_shows_paralysis(bool shown) {
    set_or_clear(PY_PARALYSED, shown);
}

bool player_status_line_shows_repeat(void) {
    return is_set(PY_REPEAT);
}

void player_set_status_line_shows_repeat(bool shown) {
    set_or_clear(PY_REPEAT, shown);
}

uint32_t player_status_word(void) {
    return status;
}

void player_set_status_word(uint32_t word) {
    status = word;
}
