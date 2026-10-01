// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How this life ended and whether it counts: where the record of it lives

#include "config.h"
#include "constant.h"
#include "types.h"

#include "score_death.h"

// State is static: the only way in is through the accessors below. died_from
// and birth_date have no initial values because they are written before they
// are read (character creation fills birth_date, dying fills died_from).
static bool death = false; // True if died
static vtype died_from;
static int32_t birth_date;
static int16_t noscore = 0; // Don't log the game. -CJS-
static bool total_winner = false;
static int32_t max_score = 0;

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

// --- whether the game was won --------------------------------------------

bool player_has_won(void) {
    return total_winner;
}

void set_player_has_won(bool won) {
    total_winner = won;
}

// --- the best score the run has reached ----------------------------------

int32_t best_score_so_far(void) {
    return max_score;
}

void set_best_score_so_far(int32_t score) {
    max_score = score;
}
