// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Which spells the character knows: where the four answers live

#include "config.h"
#include "constant.h"
#include "types.h"

#include "spells_known.h"

// No externs.h here, the same as panel.c, stores.c, options.c, stats.c,
// inventory.c, progress.c, score_death.c, player_pos.c, hp_table.c,
// player_light.c and burden.c: what the character has learned needs nothing
// from the rest of the game.

// Step A keeps all four where they have always been (player.c) and only adds
// the windows over them, so this commit cannot change behaviour. Step C moves
// the definitions in here and makes them static.
extern uint32_t spell_learned;
extern uint32_t spell_worked;
extern uint32_t spell_forgotten;
extern uint8_t spell_order[32];

// The bit for one spell. SPELL_NONE (and any other number outside 0..31) gets
// no bit at all: `1L << 99` has no defined result, and the callers walk the
// learned-in order, which is full of SPELL_NONE until the character has learned
// 32 spells. The old code spelled this test out at both of those walks.
static uint32_t bit_of(int spell) {
    if (spell < 0 || spell > 31) {
        return 0;
    }
    return (uint32_t)(1L << spell);
}

bool spell_is_learned(int spell) {
    return (spell_learned & bit_of(spell)) != 0;
}

bool spell_is_forgotten(int spell) {
    return (spell_forgotten & bit_of(spell)) != 0;
}

bool spell_has_worked(int spell) {
    return (spell_worked & bit_of(spell)) != 0;
}

bool any_spell_learned(void) {
    return spell_learned != 0;
}

bool any_spell_forgotten(void) {
    return spell_forgotten != 0;
}

int learned_spell_count(void) {
    int count = 0;
    for (uint32_t mask = 0x1; mask != 0; mask <<= 1) {
        if ((spell_learned & mask) != 0) {
            count++;
        }
    }
    return count;
}

void spell_learn(int spell) {
    spell_learned |= bit_of(spell);

    // Append to the history. The position is the first SPELL_NONE: there is no
    // separate count of how many spells have been learned, and the caller
    // (gain_spells) used to find the position by the same scan.
    for (int n = 0; n < 32; n++) {
        if (spell_order[n] == SPELL_NONE) {
            spell_order[n] = (uint8_t)spell;
            return;
        }
    }

    // A full history drops the spell from the order rather than writing past
    // the end of it, which is what the old `spell_order[last_known++] = ...`
    // did once last_known reached 32. There are only 31 spells to a class and
    // 32 slots, so neither can happen in a game that has not gone wrong.
}

void spell_forget(int spell) {
    uint32_t bit = bit_of(spell);
    spell_learned &= ~bit;
    spell_forgotten |= bit;
}

void spell_remember(int spell) {
    uint32_t bit = bit_of(spell);
    spell_forgotten &= ~bit;
    spell_learned |= bit;
}

void spell_mark_worked(int spell) {
    spell_worked |= bit_of(spell);
}

int spell_learned_nth(int n) {
    // Out of range answers "nothing learned there". The forgetting loop in
    // calc_spells() counts down from 31 and stops when nothing is left to
    // forget, so it can only pass a negative number if a spell is marked
    // learned without being in the history -- which the windows above make
    // impossible, and which used to read past the front of the array.
    if (n < 0 || n > 31) {
        return SPELL_NONE;
    }
    return spell_order[n];
}

void spell_order_forget_all(void) {
    for (int n = 0; n < 32; n++) {
        spell_order[n] = SPELL_NONE;
    }
}

uint32_t spells_learned_among(uint32_t spells) {
    return spells & spell_learned;
}

uint32_t spells_not_learned_among(uint32_t spells) {
    return spells & ~spell_learned;
}

uint32_t spells_learned_bits(void) {
    return spell_learned;
}

uint32_t spells_worked_bits(void) {
    return spell_worked;
}

uint32_t spells_forgotten_bits(void) {
    return spell_forgotten;
}

void spells_set_learned_bits(uint32_t bits) {
    spell_learned = bits;
}

void spells_set_worked_bits(uint32_t bits) {
    spell_worked = bits;
}

void spells_set_forgotten_bits(uint32_t bits) {
    spell_forgotten = bits;
}

uint8_t *spell_order_bytes(void) {
    return spell_order;
}
