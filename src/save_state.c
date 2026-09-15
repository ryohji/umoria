// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
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

// No externs.h here, the same as panel.c, stores.c, options.c, stats.c,
// inventory.c, progress.c and score_death.c.
//
// This is the one module of the three that has outgoing dependencies, and it
// has them on purpose. The batch of globals it belongs to was split three ways
// to keep each part small, and the split holds: measured across all of src/,
// exactly one expression mixes the three groups (signals.c:199). Rather than
// put a shared core underneath the three -- a layer earning its keep at one
// call site -- the two questions that reach across live here and name what
// they ask. save_state depends on progress and score_death; neither depends
// back, so the arrows all point one way.
//
// The four records below are owned here and are static: the only way in is
// through the windows. The initial values came over from variable.c unchanged --
// savefile had none there either, because main.c fills it from the command line
// or MORIA_SAV before anything reads it.
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
