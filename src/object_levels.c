// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The kinds of object the dungeon can produce, ordered by depth
//
// The building of the table came from init_t_level() in main.c, where it was
// static -- so the counting sort at its heart had never been reachable from a
// test. Moving it here is what made it testable.
//
// This file needs three things from outside: the object definitions, and (until
// #18-10-C) the two tables themselves. Rather than include externs.h, which
// drags in ncurses for the sake of a few lines, the declarations are written
// out here -- the same choice stats.c made and for the same reason.

#include "config.h"
#include "constant.h"
#include "types.h"

#include "object_levels.h"

extern treasure_type object_list[MAX_OBJECTS];

// Still the globals in treasure.c at this step, so that there is exactly one
// copy while the readers are moved across one at a time; #18-10-C turns these
// into static definitions here.
//
// The body of the table: object_list indexes in order of level.
extern int16_t sorted_objects[MAX_DUNGEON_OBJ];

// The index into that body: t_level[L] counts the kinds at level L or shallower,
// so level L's band runs from t_level[L - 1] up to t_level[L] - 1.
extern int16_t t_level[MAX_OBJ_LEVEL + 1];

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
