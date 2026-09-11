// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The player's boolean options, and the one table that describes them

#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>
#include <stdint.h>

// One option: what to call it, where its value lives, which bit holds it in a
// save file.
//
// The prompt and the pointer used to live in misc2.c (for the options screen)
// while the bit assignment lived twice in save.c (once to write, once to read).
// Three places had to agree about eleven options; nothing made them.
struct game_option {
    const char *prompt; // shown on the options screen; NULL ends the table
    bool *value;        // still a global, hence the pointer
    uint32_t save_bit;  // position in the option word of a save file
};

// The options, in the order the options screen lists them. Terminated by an
// entry whose prompt is NULL, because set_options() walks it to find the end.
//
// The listing order is not the bit order: find_ignore_doors is shown fifth but
// was assigned the ninth bit. Carrying the bit per entry is what lets both
// orders stay what they were.
extern const struct game_option game_options[];

// Number of options, not counting the terminator.
int game_options_count(void);

// The eleven option bits of the word a save file stores. Only those bits: the
// caller adds the flags that share the word (death, total_winner).
uint32_t game_options_pack(void);

// Sets every option from that word. Bits outside the eleven are ignored.
void game_options_unpack(uint32_t bits);

#endif // OPTIONS_H
