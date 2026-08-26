// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Magic device (staff / wand) use success calculation

#ifndef DEVICE_H
#define DEVICE_H

#include <stdbool.h>

// Penalty applied to the chance calculation per device kind.
// Staffs are harder to use than wands; this is the only difference
// between the two call sites.
#define DEVICE_PENALTY_STAFF 5
#define DEVICE_PENALTY_WAND 0

// Chance of using a magic device properly. Rolls randint() once to give
// a hopeless character a slight chance, so calling this has a side effect
// on the random number state.
int device_use_chance(int save, int int_adj, int item_level, int penalty, int class_device_adj, int player_level, int confused);

// Rolls against the chance returned by device_use_chance().
bool device_use_succeeds(int chance);

#endif // DEVICE_H
