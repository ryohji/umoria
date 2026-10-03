// src/monster/monster_recall.h
// Monster memory (recall) functions

#ifndef MONSTER_RECALL_H
#define MONSTER_RECALL_H

recall_type *recall_get(creature_handle h);
void recall_update_characteristics(creature_handle h, int defence);
void recall_update_move(creature_handle h, int move);
void recall_update_carry(creature_handle h, uint8_t number);
void recall_update_spell(creature_handle h, uint32_t type);
void recall_increment_spell_chance(creature_handle h);
void recall_increment_kill(creature_handle h);
void recall_increment_death(creature_handle h);

#endif
