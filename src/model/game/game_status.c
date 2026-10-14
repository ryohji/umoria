// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Game status: progress, save state, death and scoring

#include "config.h"
#include "constant.h"
#include "types.h"

#include "game_status.h"

// --- From progress.c: turn counter and seeds ---

static int32_t turn = -1;         // Cur turn of game
static uint32_t randes_seed;      // for restarting randes_state
static uint32_t town_seed;        // for restarting town_seed
static bool wizard = false;       // Wizard flag
static bool to_be_wizard = false; // used during startup, when -w option used

int32_t progress_turn(void) {
    return turn;
}

void progress_set_turn(int32_t new_turn) {
    turn = new_turn;
}

void progress_advance_turn(void) {
    turn++;
}

uint32_t progress_color_seed(void) {
    return randes_seed;
}

void progress_set_color_seed(uint32_t seed) {
    randes_seed = seed;
}

uint32_t progress_town_seed(void) {
    return town_seed;
}

void progress_set_town_seed(uint32_t seed) {
    town_seed = seed;
}

bool progress_wizard_mode(void) {
    return wizard;
}

void progress_set_wizard_mode(bool on) {
    wizard = on;
}

bool progress_wizard_requested(void) {
    return to_be_wizard;
}

void progress_set_wizard_requested(bool requested) {
    to_be_wizard = requested;
}

// --- From save_state.c: where saved and how far ---

static vtype savefile;                    // The savefile to use.
static bool character_generated = false;  // don't save score until char gen finished
static bool character_saved = false;      // prevents save on kill after save_char()
static bool panic_save_flag = false;      // this is true if playing from a panic save

char *save_file_path(void) {
    return savefile;
}

bool character_is_generated(void) {
    return character_generated;
}

void set_character_generated(bool generated) {
    character_generated = generated;
}

bool character_is_saved(void) {
    return character_saved;
}

void set_character_saved(bool saved) {
    character_saved = saved;
}

bool is_panic_save(void) {
    return panic_save_flag;
}

void set_panic_save(bool panic) {
    panic_save_flag = panic;
}

bool save_state_has_unsaved_character(void) {
    return character_generated && !character_saved;
}

bool save_state_has_live_character(void) {
    return !player_is_dead() && save_state_has_unsaved_character();
}

bool save_state_character_is_in_play(void) {
    return progress_turn() >= 0;
}

// --- From score_death.c: death and scoring ---

static bool death = false; // True if died
static vtype died_from;
static int32_t birth_date;
static int16_t noscore = 0; // Don't log the game. -CJS-
static bool total_winner = false;
static int32_t max_score = 0;

bool player_is_dead(void) {
    return death;
}

void set_player_dead(bool dead) {
    death = dead;
}

char *death_cause(void) {
    return died_from;
}

int32_t character_birth_date(void) {
    return birth_date;
}

void set_character_birth_date(int32_t date) {
    birth_date = date;
}

int16_t score_disqualifications(void) {
    return noscore;
}

void set_score_disqualifications(int16_t reasons) {
    noscore = reasons;
}

bool player_has_won(void) {
    return total_winner;
}

void set_player_has_won(bool won) {
    total_winner = won;
}

int32_t best_score_so_far(void) {
    return max_score;
}

void set_best_score_so_far(int32_t score) {
    max_score = score;
}
