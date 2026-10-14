// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#ifndef OBJECT_ALLOC_H
#define OBJECT_ALLOC_H

void place_trap(int, int, int);
void place_rubble(int, int);
void place_gold(int, int);
int get_obj_num(int, bool);
void place_object(int, int, bool);
// The function argument is set_room, set_corr or set_floor from sets.c. Each
// is a bool f(int) that takes cave[][].fval (the kind of floor).
void alloc_object(bool (*)(int), int, int);
void random_object(int, int, int);

#endif // OBJECT_ALLOC_H
