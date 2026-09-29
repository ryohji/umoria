// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Where each level's monsters sit in the definition table

#ifndef MONSTER_LEVELS_H
#define MONSTER_LEVELS_H

// The monster definitions are already written out in order of the level they
// belong to, so the monsters of one level sit together in a band, and the counts
// below ARE monster numbers -- feed them to monster_make_creature_handle().
// (The objects needed a second table for this, sorted_objects; monsters do not,
// which is why this module is an index and nothing else. See object_levels.h.)
//
// Level L's band runs from first_monster_at_level(L) up to
// monsters_up_to_level(L) - 1. Level 0's band starts at number 0: those are the
// town monsters, and every other reader skips them by starting at
// first_monster_at_level(1).
//
// PAST THE LAST BAND SIT THE WIN MONSTERS, the ones whose level is deeper than
// MAX_MONS_LEVEL and which therefore never turn up by level: they are counted
// nowhere, so monsters_up_to_level(MAX_MONS_LEVEL) is the first of them.
// place_win_monster() is the only reader that wants that number.
//
// This was the global m_level, an int16_t array built once at startup by a
// static function in main.c -- out of reach of any test, the same as
// init_t_level() before #18-10. Nothing saves it: loading a game rebuilds it.

// Build the index. Call once, before anything asks for a band.
void monster_levels_init(void);

// How many kinds of monster appear at `level` or shallower.
int monsters_up_to_level(int level);

// How many kinds appear at exactly `level`, and the number the band starts at.
// Both take a level of 1 or more; level 0's band starts at 0 and its width is
// monsters_up_to_level(0).
int monsters_at_level(int level);
int first_monster_at_level(int level);

#endif // MONSTER_LEVELS_H
