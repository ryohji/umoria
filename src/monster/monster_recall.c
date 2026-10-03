// src/monster/monster_recall.c

#include "headers.h"
#include "config.h"
#include "constant.h"
#include "types.h"
#include "externs.h"

static recall_type c_recall[MAX_CREATURES]; // Monster memories

recall_type *recall_get(creature_handle h) {
    return c_recall + h.place;
}

void recall_update_characteristics(creature_handle h, int defence) {
    recall_get(h)->r_cdefense |= defence;
}

void recall_update_move(creature_handle h, int move) {
    recall_get(h)->r_cmove |= move;
}

static inline uint8_t get_carry(uint32_t cmove) {
    return (cmove & CM_TREASURE) >> CM_TR_SHIFT;
}

void recall_update_carry(creature_handle h, uint8_t number) {
    recall_type *const recall = recall_get(h);
    uint32_t current = get_carry(recall->r_cmove);
    recall->r_cmove &= ~CM_TREASURE;
    recall->r_cmove |= MAX(current, number) << CM_TR_SHIFT;
}

void recall_update_spell(creature_handle h, uint32_t type) {
    recall_type *const recall = recall_get(h);
    recall->r_spells |= type;
}

void recall_increment_spell_chance(creature_handle h) {
    recall_type *const recall = recall_get(h);
    if ((recall->r_spells & CS_FREQ) != CS_FREQ) {
        recall->r_spells += 1;
    }
}

void recall_increment_kill(creature_handle h) {
    recall_type *const recall = recall_get(h);
    if (recall->r_kills < MAX_SHORT) {
        recall->r_kills += 1;
    }
}

void recall_increment_death(creature_handle h) {
    recall_type *const recall = recall_get(h);
    if (recall->r_deaths < MAX_SHORT) {
        recall->r_deaths += 1;
    }
}
