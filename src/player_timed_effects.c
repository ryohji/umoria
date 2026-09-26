// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How much longer each of the eighteen temporary states lasts

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_status_flags.h"
#include "player_timed_effects.h"

// No externs.h here, the same as the eight questions before this one. This module
// does not know what any of the eighteen counters leads to -- no messages, no
// rolls, no drawing.
//
// player_status_flags.h IS included, and it is the first time one of these
// modules calls another. That is the point of this unit: the mark and the clock
// are two halves of one fact, and the table below is the only place in the
// program that says which mark belongs to which clock.

// THE EIGHTEEN ARE STILL IN py.flags (step A). The windows go through this table,
// so #18-12-9C has one place to change: the pointers become an array of the
// counters themselves and this comment goes away.
extern player_type py;

static int16_t *const turns_field[PLAYER_TIMED_COUNT] = {
    &py.flags.blind,       &py.flags.paralysis,   &py.flags.confused,
    &py.flags.fast,        &py.flags.slow,        &py.flags.afraid,
    &py.flags.poisoned,    &py.flags.image,       &py.flags.protevil,
    &py.flags.invuln,      &py.flags.hero,        &py.flags.shero,
    &py.flags.blessed,     &py.flags.resist_heat, &py.flags.resist_cold,
    &py.flags.detect_inv,  &py.flags.word_recall, &py.flags.tim_infra,
};

// WHICH MARK PAIRS WITH WHICH CLOCK. Twelve of the fourteen marks in
// player_status_flags.c pair with one of the eighteen counters; the other two
// (hungry, weak) pair with the stomach, and six of the counters have no mark at
// all. Swapping two rows here would announce the wrong state -- that is what the
// test file nails one row at a time.
//
// PLAYER_EFFECT_COUNT is not a mark, so it stands for "this one has none", and
// nothing is ever handed to player_status_flags.c without has_mark() first.
#define NO_MARK PLAYER_EFFECT_COUNT

static const player_effect mark_of[PLAYER_TIMED_COUNT] = {
    PLAYER_EFFECT_BLIND,         // blindness
    NO_MARK,                     // paralysis -- the status line shows it instead
    PLAYER_EFFECT_CONFUSED,      // confusion
    PLAYER_EFFECT_HASTED,        // haste
    PLAYER_EFFECT_SLOWED,        // slowness
    PLAYER_EFFECT_AFRAID,        // fear
    PLAYER_EFFECT_POISONED,      // poison
    NO_MARK,                     // hallucination -- the whole map is redrawn
    NO_MARK,                     // protection from evil -- a message only
    PLAYER_EFFECT_INVULNERABLE,  // invulnerability
    PLAYER_EFFECT_HERO,          // heroism
    PLAYER_EFFECT_SUPER_HERO,    // super heroism
    PLAYER_EFFECT_BLESSED,       // blessing
    NO_MARK,                     // heat resistance -- a message only
    NO_MARK,                     // cold resistance -- a message only
    PLAYER_EFFECT_SEE_INVISIBLE, // seeing the invisible
    NO_MARK,                     // word of recall -- fires at one, not at zero
    PLAYER_EFFECT_INFRA_VISION,  // infra-vision
};

// No range check on the effect, the same as player_status_flags.c: THE ENUM IS
// THE ONLY WAY TO NAME ONE, so a value outside the eighteen can only come from a
// cast, and there is no window that turns a number into an effect (the save
// file's run is not addressed by position here -- see the header).
static bool has_mark(player_timed_effect effect) {
    return mark_of[effect] != NO_MARK;
}

static int16_t *turns_of(player_timed_effect effect) {
    return turns_field[effect];
}

bool player_timed_in_force(player_timed_effect effect) {
    return *turns_of(effect) > 0;
}

int player_timed_turns(player_timed_effect effect) {
    return *turns_of(effect);
}

void player_timed_add(player_timed_effect effect, int turns) {
    *turns_of(effect) = (int16_t)(*turns_of(effect) + turns);
}

void player_timed_set(player_timed_effect effect, int turns) {
    *turns_of(effect) = (int16_t)turns;
}

void player_timed_clear(player_timed_effect effect) {
    player_timed_set(effect, 0);
}

void player_timed_shorten_to(player_timed_effect effect, int turns) {
    if (*turns_of(effect) > turns) {
        *turns_of(effect) = (int16_t)turns;
    }
}

bool player_timed_beginning(player_timed_effect effect) {
    if (!has_mark(effect)) {
        return false;
    }

    return player_note_effect_started(mark_of[effect]);
}

bool player_timed_count_down(player_timed_effect effect) {
    // Unconditionally: minus one is a state the fear counter really reaches
    // (player_timed_effects.h).
    int16_t *turns = turns_of(effect);
    *turns = (int16_t)(*turns - 1);

    if (*turns != 0) {
        return false;
    }

    if (has_mark(effect)) {
        player_note_effect_ended(mark_of[effect]);
    }

    return true;
}
