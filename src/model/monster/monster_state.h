// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Monster state: levels index, the list of monsters on the current level, and recall

#ifndef MONSTER_STATE_H
#define MONSTER_STATE_H

#include "config.h"
#include "constant.h"
#include "types.h"

#include <stdbool.h>
#include <stdint.h>

// monster_type, recall_type, creature_handle come from types.h which must be included first.

// --- Level index ---

// Initializes the level index. Called once at startup.
void monster_levels_init(void);

// Total monsters at level L or shallower (cumulative count).
int monsters_up_to_level(int level);

// Number of monsters at exactly level L.
int monsters_at_level(int level);

// Index of first monster at level L. Level L's band runs from first_monster_at_level(L) to monsters_up_to_level(L) - 1.
int first_monster_at_level(int level);

// --- The list on this level ---

// Resets the monster list to empty. Links all free space together.
void monster_list_reset(void);

// Returns pointer to monster at given index in the list.
monster_type *monster_list_at(int index);

// How far the packed list is filled (next free slot).
int16_t monster_list_used(void);
void set_monster_list_used(int16_t used);

// True when the list is full.
bool monster_list_is_full(void);

// Number of free slots remaining.
int monster_list_free_slots(void);

// Claims the next slot and advances the mark. Returns the claimed index.
int monster_list_claim_slot(void);

// Drops the last claimed slot and moves the mark back.
void monster_list_drop_last(void);

// --- What the player has learned ---

// Returns pointer to recall for the given monster kind.
recall_type *recall_get(creature_handle h);

// Updates learned characteristics (defence flags).
void recall_update_characteristics(creature_handle h, int defence);

// Updates learned movement (move flags).
void recall_update_move(creature_handle h, int move);

// Updates learned treasure carry count.
void recall_update_carry(creature_handle h, uint8_t number);

// Updates learned spells (spell type flags).
void recall_update_spell(creature_handle h, uint32_t type);

// Increments spell frequency counter (capped at CS_FREQ).
void recall_increment_spell_chance(creature_handle h);

// Increments kill count (capped at MAX_SHORT).
void recall_increment_kill(creature_handle h);

// Increments death count (capped at MAX_SHORT).
void recall_increment_death(creature_handle h);

#endif // MONSTER_STATE_H
