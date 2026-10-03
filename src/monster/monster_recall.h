// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// What the player has learned about each kind of monster

#ifndef MONSTER_RECALL_H
#define MONSTER_RECALL_H

// No includes: the callers bring config.h, constant.h and types.h in first.

recall_type *recall_get(creature_handle h);
void recall_update_characteristics(creature_handle h, int defence);
void recall_update_move(creature_handle h, int move);
void recall_update_carry(creature_handle h, uint8_t number);
void recall_update_spell(creature_handle h, uint32_t type);
void recall_increment_spell_chance(creature_handle h);
void recall_increment_kill(creature_handle h);
void recall_increment_death(creature_handle h);

#endif // MONSTER_RECALL_H
