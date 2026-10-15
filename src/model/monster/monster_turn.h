// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Which monster creatures() is acting for, and so whether the monster list may
// be renumbered while a monster is removed. Not saved.

#ifndef MONSTER_TURN_H
#define MONSTER_TURN_H

#include <stdbool.h>

// Bracket one call in creatures() that may compact the monster list.
void monster_turn_begin(int index);
void monster_turn_end(void);

// May the hole at `index` be closed by moving the last monster into it?
// True when no turn is in progress or `index` is above the current monster.
bool monster_delete_may_shift(int index);

// The current monster's index, -1 between turns.
int monster_turn_index(void);

#endif // MONSTER_TURN_H
