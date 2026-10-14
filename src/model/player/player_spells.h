// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Player spell knowledge: which spells are known/worked/forgotten, the order learned, and how many more can be learned.

#ifndef PLAYER_SPELLS_H
#define PLAYER_SPELLS_H

// The value for "no spell learned here yet" in the learned-in order.
#define SPELL_NONE 99

// --- Which spells are known ---

// True if the character can cast the spell right now.
bool spell_is_learned(int spell);

// True if known but lost to level/stat change.
bool spell_is_forgotten(int spell);

// True once cast successfully (tracked for first-success experience).
bool spell_has_worked(int spell);

// True if at least one spell is learned (used by calc_mana).
bool any_spell_learned(void);

// True if at least one spell is forgotten.
bool any_spell_forgotten(void);

// Count of spells currently learned.
int learned_spell_count(void);

// --- Learning and forgetting ---

// Mark spell as learned and add to history.
void spell_learn(int spell);

// Move spell from learned to forgotten.
void spell_forget(int spell);

// Move spell from forgotten back to learned.
void spell_remember(int spell);

// Mark spell as successfully cast (for experience).
void spell_mark_worked(int spell);

// --- Learning order ---

// The nth spell in learning order (0-31), or SPELL_NONE if none there.
int spell_learned_nth(int n);

// Clear the entire learning order history.
void spell_order_forget_all(void);

// --- Bit operations (for save/load) ---

// Return which of the given spells are learned.
uint32_t spells_learned_among(uint32_t spells);

// Return which of the given spells are not learned.
uint32_t spells_not_learned_among(uint32_t spells);

// Get/set the full bit masks (for save file).
uint32_t spells_learned_bits(void);
uint32_t spells_worked_bits(void);
uint32_t spells_forgotten_bits(void);
void spells_set_learned_bits(uint32_t bits);
void spells_set_worked_bits(uint32_t bits);
void spells_set_forgotten_bits(uint32_t bits);

// Direct access to spell order array (for save file).
uint8_t *spell_order_bytes(void);

// --- How many more can be learned ---

// Count of additional spells that may be learned now.
int player_spells_to_learn(void);

// Set the count (replaces, doesn't add).
void player_spells_to_learn_set(int count);

#endif // PLAYER_SPELLS_H
