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
// declarations.
//
// The four are still defined in variable.c at this step, so they are declared
// by hand here and the window reads and writes them through these names. That
// keeps behaviour identical while the callers are moved over one group at a
// time; the definitions come here and turn static once no caller reaches past
// the window.
//
// highscore_fp is not here even though it belongs to the same group of
// globals. It never needed to be a global at all: display_scores() and
// highscores() each fopen and fclose it inside the one function, so it becomes
// a local of each. Keeping the file handle out means this module does no file
// I/O, which is what lets it go without externs.h (externs.h replaces fopen
// with tfopen).
extern bool death;
extern vtype died_from;
extern int32_t birth_date;
extern int16_t noscore;

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
