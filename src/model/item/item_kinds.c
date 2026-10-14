// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#include "config.h"
#include "constant.h"
#include "types.h"
#include "externs.h"
#include "item_kinds.h"

// The body of the table: object_list indexes in order of level.
static int16_t sorted_objects[MAX_DUNGEON_OBJ];

// The index into that body: t_level[L] counts the kinds at level L or shallower,
// so level L's band runs from t_level[L - 1] up to t_level[L] - 1.
//
// Starting out as zeroes means nothing at all until object_levels_init() runs,
// which is why main() calls it before the first cave is generated.
static int16_t t_level[MAX_OBJ_LEVEL + 1];

void object_levels_init(void) {
    for (int i = 0; i <= MAX_OBJ_LEVEL; i++) {
        t_level[i] = 0;
    }

    for (int i = 0; i < MAX_DUNGEON_OBJ; i++) {
        t_level[object_list[i].level]++;
    }

    for (int i = 1; i <= MAX_OBJ_LEVEL; i++) {
        t_level[i] += t_level[i - 1];
    }

    // now produce an array with object indexes sorted by level,
    // by using the info in t_level, this is an O(n) sort!
    // this is not a stable sort, but that does not matter
    int tmp[MAX_OBJ_LEVEL + 1];

    for (int i = 0; i <= MAX_OBJ_LEVEL; i++) {
        tmp[i] = 1;
    }

    for (int i = 0; i < MAX_DUNGEON_OBJ; i++) {
        int l = object_list[i].level;
        sorted_objects[t_level[l] - tmp[l]] = i;
        tmp[l]++;
    }
}

int16_t object_at_level_position(int position) {
    return sorted_objects[position];
}

int objects_up_to_level(int level) {
    return t_level[level];
}

int objects_at_level(int level) {
    return t_level[level] - t_level[level - 1];
}

int first_position_at_level(int level) {
    return t_level[level - 1];
}

// Copy fields from an object_list template into an inven_type.
// Used to initialize inventory slots, floor items, and store items from
// the definition table.
void invcopy(inven_type *to, int from_index) {
    treasure_type *from = &object_list[from_index];

    to->index       = from_index;
    to->name2       = SN_NULL;
    to->inscrip[0]  = '\0';
    to->flags       = from->flags;
    to->tval        = from->tval;
    to->tchar       = from->tchar;
    to->p1          = from->p1;
    to->cost        = from->cost;
    to->subval      = from->subval;
    to->number      = from->number;
    to->weight      = from->weight;
    to->tohit       = from->tohit;
    to->todam       = from->todam;
    to->ac          = from->ac;
    to->toac        = from->toac;
    to->damage[0]   = from->damage[0];
    to->damage[1]   = from->damage[1];
    to->level       = from->level;
    to->ident       = 0;
}
