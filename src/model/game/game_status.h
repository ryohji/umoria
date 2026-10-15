// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Game status: progress, save state, death and scoring

#ifndef GAME_STATUS_H
#define GAME_STATUS_H

#include <stdbool.h>
#include <stdint.h>

// vtype comes from types.h, which must be included before this header.

// --- Turn counter and seeds ---

// Current turn. Starts at -1 (no character in play). Raised to 0 on first turn.
int32_t progress_turn(void);
void progress_set_turn(int32_t turn);
void progress_advance_turn(void);

// Seeds for reproducible world generation.
uint32_t progress_color_seed(void);
void progress_set_color_seed(uint32_t seed);

uint32_t progress_town_seed(void);
void progress_set_town_seed(uint32_t seed);

// Wizard mode: granted and requested.
bool progress_wizard_mode(void);
void progress_set_wizard_mode(bool on);

bool progress_wizard_requested(void);
void progress_set_wizard_requested(bool requested);

// --- Where saved and how far ---

// Save file path. Pointer is valid for the entire run.
char *save_file_path(void);

// True once character generation has finished.
bool character_is_generated(void);
void set_character_generated(bool generated);

// True once character has been written out. Cleared on resume.
bool character_is_saved(void);
void set_character_saved(bool saved);

// True when the file on disk came from a panic save.
bool is_panic_save(void);
void set_panic_save(bool panic);

// Questions: is there a character that exists but is unsaved?
bool save_state_has_unsaved_character(void);

// Same question with "and still alive" on top. Reads death flag.
bool save_state_has_live_character(void);

// Is a character in play at all? (turn >= 0)
bool save_state_character_is_in_play(void);

// --- Death and scoring ---

// True once player has died.
bool player_is_dead(void);
void set_player_dead(bool dead);

// Cause of death string. Pointer is valid for the entire run.
char *death_cause(void);

// Wall clock at character creation.
int32_t character_birth_date(void);
void set_character_birth_date(int32_t date);

// Disqualification flags (0x1=resurrected, 0x2=wizard, 0x4=duplicate).
#define SCORE_DISQUALIFY_RESURRECTED 0x1
#define SCORE_DISQUALIFY_WIZARD      0x2
#define SCORE_DISQUALIFY_DUPLICATE   0x4

int16_t score_disqualifications(void);
void set_score_disqualifications(int16_t reasons);

// True once player has beaten the game (CM_WIN monster).
bool player_has_won(void);
void set_player_has_won(bool won);

// Highest score the run has been worth (never decreases).
int32_t best_score_so_far(void);
void set_best_score_so_far(int32_t score);

#endif // GAME_STATUS_H
