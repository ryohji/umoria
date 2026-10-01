// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Where the game is saved and how far the saving has got: where the record
// of it lives

#include "config.h"
#include "constant.h"
#include "types.h"

#include "progress.h"
#include "save_state.h"
#include "score_death.h"

// This module has outgoing dependencies: save_state depends on progress and
// score_death; neither depends back. The two cross-cutting questions live here
// rather than in a shared core layer.
//
// State is static: the only way in is through the accessors below. savefile has
// no initial value because main.c fills it from the command line or MORIA_SAV
// before anything reads it.
static vtype savefile;                    // The savefile to use.
static bool character_generated = false;  // don't save score until char gen finished
static bool character_saved = false;      // prevents save on kill after save_char()
static bool panic_save = false;           // this is true if playing from a panic save

// --- where the game is saved ---------------------------------------------

char *save_file_path(void) {
    return savefile;
}

// --- how far the saving has got -------------------------------------------

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
    return panic_save;
}

void set_panic_save(bool panic) {
    panic_save = panic;
}

// --- the questions the callers ask ----------------------------------------

bool save_state_has_unsaved_character(void) {
    return character_generated && !character_saved;
}

bool save_state_has_live_character(void) {
    return !player_is_dead() && save_state_has_unsaved_character();
}

bool save_state_character_is_in_play(void) {
    return progress_turn() >= 0;
}
