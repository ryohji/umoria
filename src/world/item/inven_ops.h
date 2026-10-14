// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#ifndef INVEN_OPS_H
#define INVEN_OPS_H

void inven_destroy(int);
void take_one_item(inven_type *, inven_type *);
void inven_drop(int, int);
// Function argument is a predicate from sets.c (set_corrodes, set_flammable, etc.),
// each taking bool f(inven_type *).
int inven_damage(bool (*)(inven_type *), int);
int weight_limit(void);
bool inven_check_num(inven_type *);
bool inven_check_weight(inven_type *);
void check_strength(void);
int inven_carry(inven_type *);
int find_range(int, int, int *, int *);

#endif // INVEN_OPS_H
