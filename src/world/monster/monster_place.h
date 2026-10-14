// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#ifndef MONSTER_PLACE_H
#define MONSTER_PLACE_H

bool compact_monsters(void);
bool place_monster(int, int, creature_handle, int);
void place_win_monster(void);
void alloc_monster(int, int, int);
bool summon_monster(int *, int *, int);
bool summon_undead(int *, int *);

#endif // MONSTER_PLACE_H
