// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The player's boolean options, and the one table that describes them

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"
#include "options.h"

// The prompts are the ones the options screen has always shown, and the bits
// are the ones save files have always used.
const struct game_option game_options[] = {
    {"Running: cut known corners", &find_cut, 0x1},
    {"Running: examine potential corners", &find_examine, 0x2},
    {"Running: print self during run", &find_prself, 0x4},
    {"Running: stop when map sector changes", &find_bound, 0x8},
    {"Running: run through open doors", &find_ignore_doors, 0x100},
    {"Prompt to pick up objects", &prompt_carry_flag, 0x10},
    {"Rogue like commands", &rogue_like_commands, 0x20},
    {"Show weights in inventory", &show_weight_flag, 0x40},
    {"Highlight and notice mineral seams", &highlight_seams, 0x80},
    {"Beep for invalid character", &sound_beep_flag, 0x200},
    {"Display rest/repeat counts", &display_counts, 0x400},
    {NULL, NULL, 0},
};

int game_options_count(void) {
    int count = 0;
    while (game_options[count].prompt != NULL) {
        count++;
    }
    return count;
}

uint32_t game_options_pack(void) {
    uint32_t bits = 0;

    for (int i = 0; game_options[i].prompt != NULL; i++) {
        if (*game_options[i].value) {
            bits |= game_options[i].save_bit;
        }
    }
    return bits;
}

void game_options_unpack(uint32_t bits) {
    for (int i = 0; game_options[i].prompt != NULL; i++) {
        *game_options[i].value = (bits & game_options[i].save_bit) != 0;
    }
}
