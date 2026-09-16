// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How this life ended and whether it counts: where the record of it lives

#include "config.h"
#include "constant.h"
#include "types.h"

#include "score_death.h"

// No externs.h here, the same as panel.c, stores.c, options.c, stats.c,
// inventory.c and progress.c: a place to keep the ending of a run needs nothing
// from the rest of the game, and externs.h would drag ncurses in for four
// declarations. Nothing at all is declared from outside now, so this file
// compiles on its own.
//
// The state is owned here and is static: the only way in is through the windows
// below. The initial values came over from variable.c unchanged -- died_from and
// birth_date had none there either, because they are written before they are
// read (character creation fills birth_date, and dying fills died_from).
//
// highscore_fp is not here even though it belongs to the same group of
// globals. It never needed to be a global for the two functions that do the
// reading and writing: display_scores() and highscores() each fopen and fclose
// it inside the one function, and both now use a local (death.c). What is left
// of the global belongs to init_scorefile() (files.c), which opens the file
// while the setuid privileges are still there -- a startup concern, not this
// module's. Keeping the file handle out means this module does no file I/O,
// which is what lets it go without externs.h (externs.h replaces fopen with
// tfopen).
static bool death = false; // True if died
static vtype died_from;
static int32_t birth_date;
static int16_t noscore = 0; // Don't log the game. -CJS-

// --- the death flag ------------------------------------------------------

bool player_is_dead(void) {
    return death;
}

void set_player_dead(bool dead) {
    death = dead;
}

// --- what did it ---------------------------------------------------------

char *death_cause(void) {
    return died_from;
}

// --- when the character was born -----------------------------------------

int32_t character_birth_date(void) {
    return birth_date;
}

void set_character_birth_date(int32_t date) {
    birth_date = date;
}

// --- whether the score counts --------------------------------------------

int16_t score_disqualifications(void) {
    return noscore;
}

void set_score_disqualifications(int16_t reasons) {
    noscore = reasons;
}
