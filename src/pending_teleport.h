// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Whether a teleport is waiting to happen

#ifndef PENDING_TELEPORT_H
#define PENDING_TELEPORT_H

// One bit, and it exists because of one trap. Every other teleport in the game
// calls teleport() on the spot; the teleport trap (moria3.c) may not, because it
// is stepped on in the middle of a move and the player has to be lit where they
// are first -- "Light up the teleport trap, before we teleport away". So the trap
// leaves a note, and the main loop (dungeon.c) reads it once the turn's commands
// are done and calls teleport(100).

// Teleport the player when this turn's commands are over. Only the teleport trap
// says this.
void schedule_teleport(void);

// Is one waiting? Asked by the main loop, and by the save state when it takes a
// snapshot.
bool teleport_is_pending(void);

// It happened. Said by teleport() (misc3.c) on its way out, for every teleport,
// not only the scheduled kind -- so a teleport from any other cause also clears
// a note left by the trap. That is the old behaviour and it is harmless: the
// player has been moved, which is all the note asked for.
void teleport_done(void);

// Nothing is waiting: a level has begun. dungeon.c does this in its setup, next
// to begin_level() (level_exit.h). It is not "it happened" -- no teleport is
// performed -- so it is a separate window from teleport_done().
void forget_pending_teleport(void);

#endif // PENDING_TELEPORT_H
