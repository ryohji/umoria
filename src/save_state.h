// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Where the game is saved and how far the saving has got: the file name,
// whether a character exists yet, whether it has been written out, and whether
// what was written out was a panic save.

#ifndef SAVE_STATE_H
#define SAVE_STATE_H

// vtype comes from types.h, which has to be included before this header.

// This is storage plus the three questions the callers keep asking of it. It
// does not range-check or clamp, and it does no file I/O: the file name lives
// here, opening it does not (which is what lets this file go without
// externs.h, where fopen is replaced by tfopen).

// --- where the game is saved ---------------------------------------------

// The path of the save file. The pointer addresses the record itself and stays
// valid for the rest of the run: main.c writes through it with strcpy() from
// the command line, the environment or MORIA_SAV, and save.c rewrites it when
// the player names a different file.
char *save_file_path(void);

// --- how far the saving has got -------------------------------------------

// True once character generation has finished. Until then there is nothing
// worth saving and nothing worth scoring.
bool character_is_generated(void);
void set_character_generated(bool generated);

// True once the character has been written out. Cleared again when a run is
// resumed, so it is "is the file up to date", not a one-way latch.
bool character_is_saved(void);
void set_character_saved(bool saved);

// True when the file on disk came from a panic save (written from a fatal
// signal handler rather than from the player quitting). Such a run is not
// scored.
bool is_panic_save(void);
void set_panic_save(bool panic);

// --- the questions the callers ask ----------------------------------------

// Is there a character that exists but has not been written out? Asked at four
// places in three different spellings before this window existed:
//   signals.c:166  !character_saved && character_generated
//   death.c:462    character_generated && !character_saved
//   io.c:72        !character_generated || character_saved   (the negation)
//   signals.c:199  the same, with !death on top
// The three spellings are one question, so it gets one name.
bool save_state_has_unsaved_character(void);

// The same question with "and still alive" on top -- what signals.c:199 asks
// before trying a panic save. This is the one place where the three groups of
// this batch of globals meet, so it is the one place with a dependency: it
// reads the death flag from score_death.h.
bool save_state_has_live_character(void);

// Is a character in play at all? This is what a turn counter of 0 or more
// means; a negative counter is not a turn but "nobody is playing". Reads the
// counter from progress.h.
//
// Covers the `turn >= 0` tests (save.c:471, save.c:963, death.c:455) and,
// negated, the `turn < 0` ones (save.c:730, save.c:878). It does NOT cover
// signals.c:170, which asks `turn > 0` and so excludes turn 0; that caller
// keeps its own comparison, because folding it in here would change what
// happens on the first turn.
bool save_state_character_is_in_play(void);

#endif // SAVE_STATE_H
