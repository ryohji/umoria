// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#ifndef HIT_ROLLS_H
#define HIT_ROLLS_H

int attack_blows(int, int *);
int tot_dam(inven_type *, int, creature_handle);
int critical_blow(int, int, int, int);
bool test_hit(int, int, int, int, int);

#endif // HIT_ROLLS_H
