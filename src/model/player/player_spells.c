// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Player spell knowledge: which spells are known/worked/forgotten, order learned, and how many more can be learned.

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_spells.h"

// --- Spell knowledge state ---

static uint32_t spell_learned = 0;   // bit mask of spells learned
static uint32_t spell_worked = 0;    // bit mask of spells tried and worked
static uint32_t spell_forgotten = 0; // bit mask of spells learned but forgotten

// Order spells were learned in (spell numbers). Filled with SPELL_NONE at start.
static uint8_t spell_order[32];

// --- Spells to learn counter ---

// Count of additional spells that may be learned now.
static uint8_t spells_to_learn_count;

// --- Helper ---

// Get bit for one spell. SPELL_NONE and out-of-range return 0.
static uint32_t bit_of(int spell) {
    if (spell < 0 || spell > 31) {
        return 0;
    }
    return (uint32_t)(1L << spell);
}

// --- Which spells are known ---

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

// --- Learning and forgetting ---

void spell_learn(int spell) {
    spell_learned |= bit_of(spell);

    // Append to history at first SPELL_NONE position.
    for (int n = 0; n < 32; n++) {
        if (spell_order[n] == SPELL_NONE) {
            spell_order[n] = (uint8_t)spell;
            return;
        }
    }
    // Full history drops the spell rather than overflowing (shouldn't happen).
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

// --- Learning order ---

int spell_learned_nth(int n) {
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

// --- Bit operations ---

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

// --- How many more can be learned ---

int player_spells_to_learn(void) {
    return spells_to_learn_count;
}

void player_spells_to_learn_set(int count) {
    spells_to_learn_count = (uint8_t)count;
}
