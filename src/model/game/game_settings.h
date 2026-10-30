// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Game settings: the player's options and other configuration globals

#ifndef GAME_SETTINGS_H
#define GAME_SETTINGS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// --- The options table ---

// One option: its prompt, where the value lives, which bit in save files, initial value.
struct game_option {
    const char *prompt; // shown on the options screen; NULL ends the table
    bool *value;        // still a global, hence the pointer
    uint32_t save_bit;  // position in the option word of a save file
    bool initial;       // what a fresh game starts with
};

// The options, in display order. Terminated by NULL prompt.
extern const struct game_option game_options[];

// Number of options (not counting terminator).
int game_options_count(void);

// Sets every option to its initial value. Called once at startup.
void game_options_reset(void);

// Returns the eleven option bits for saving. Caller adds other flags that share the word.
uint32_t game_options_pack(void);

// Sets every option from the saved word. Bits outside the eleven are ignored.
void game_options_unpack(uint32_t bits);

// These are options, set with set_options command -CJS-
extern bool rogue_like_commands;

extern bool find_cut;          // Cut corners on a run

extern bool find_examine;      // Check corners on a run

extern bool find_prself;       // Print yourself on a run (slower)

extern bool find_bound;        // Stop run when the map shifts

extern bool prompt_carry_flag; // Prompt to pick something up

extern bool show_weight_flag;  // Display weights in inventory

extern bool highlight_seams;   // Highlight magma and quartz

extern bool find_ignore_doors; // Run through open doors

extern bool sound_beep_flag;   // Beep for invalid character

extern bool display_counts;    // Display rest/repeat counts

#endif // GAME_SETTINGS_H
