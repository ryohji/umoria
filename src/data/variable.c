// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Global variables

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

// That handle is never read and never closed (bugs.md B19).
FILE *highscore_fp;               // File pointer to high score file

bool free_turn_flag; // Used in MORIA, do not move creatures

// options set via the '=' command
//
// The starting values live in options.c, beside the prompt and the save file
// bit of each option; game_options_reset() puts them here at startup.
bool rogue_like_commands;
bool find_cut;
bool find_examine;
bool find_bound;
bool find_prself;
bool prompt_carry_flag;
bool show_weight_flag;
bool highlight_seams;
bool find_ignore_doors;
bool sound_beep_flag;
bool display_counts;

int closing_flag = 0; // Used for closing
