// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Whether the level the player is standing on is finished, and which one is next

#ifndef LEVEL_EXIT_H
#define LEVEL_EXIT_H

// One bit, and one reader: the main loop (dungeon.c) runs turns until the bit is
// set, then returns to play_game(), which builds a level and calls dungeon()
// again. Everything else only sets it.
//
// The old name was new_level_flag, and it was a little narrower than the bit
// itself: six of the eight places that set it were going somewhere (stairs, a
// trap door, word-of-recall, a deep-descent scroll, the wizard's ^D), but dying
// and quitting set the same bit with nowhere to go. What they have in common is
// "stop playing this level", so that is what the windows say.

// Is this level finished? Asked by the main loop, and by the save state when it
// takes a snapshot.
bool level_is_over(void);

// This level is finished, and the next one is `level`.
//
// This is the pair that always moved together: six of the eight old assignments
// to new_level_flag put a new value in dun_level in the same breath, and a
// half-done pair is two different accidents -- a new depth the loop never goes
// to, or the same level generated again under a depth that has already changed.
//
// The limits stay with the callers, because they belong to those commands and
// not to leaving a level: the wizard's ^D takes 0-99 because that is what its
// prompt says, and the deep-descent scroll stops at 1 because it may not push
// the player above the town. Level 0 is the town, and it is a level like any
// other here -- word-of-recall uses it.
void leave_for_level(int level);

// This level is finished and no other one is named, so dun_level is left as it
// is. Three callers: dying, quitting, and word-of-recall with nowhere to go
// (already in town with no depth recorded yet). For the first two the game ends
// before another level is built; the third rebuilds the town.
void end_level(void);

// A level has begun, so nothing is pending. dungeon.c does this in its setup,
// before the first turn of the level. It says nothing about dun_level, which by
// then already holds the level being started.
void begin_level(void);

#endif // LEVEL_EXIT_H
