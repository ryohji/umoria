// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The options screen: shows the player's boolean options and lets them be
// switched on and off
//
// set_options() came over from misc2.c unchanged. It is kept out of
// data/options.c on purpose: that module is the table alone and calls nothing
// on the screen, so its test does not have to link the screen code.

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"
#include "options.h"

// Set or unset various boolean options. -CJS-
//
// The options themselves are in options.c, so that this screen and the save
// file cannot disagree about what they are.
void set_options(void) {
    prt("  ESC when finished, y/n to set options, <return> or - to move cursor", 0, 0);

    int max;
    for (max = 0; game_options[max].prompt != NULL; max++) {
        vtype string;

        (void)sprintf(string, "%-38s: %s", game_options[max].prompt, (*game_options[max].value ? "yes" : "no "));
        prt(string, max + 1, 0);
    }
    erase_line(max + 1, 0);

    int i = 0;
    for (;;) {
        move_cursor(i + 1, 40);
        switch (inkey()) {
        case ESCAPE:
            return;
        case '-':
            if (i > 0) {
                i--;
            } else {
                i = max - 1;
            }
            break;
        case ' ': case '\n': case '\r':
            if (i + 1 < max) {
                i++;
            } else {
                i = 0;
            }
            break;
        case 'y': case 'Y':
            put_buffer("yes", i + 1, 40);
            *game_options[i].value = true;
            if (i + 1 < max) {
                i++;
            } else {
                i = 0;
            }
            break;
        case 'n': case 'N':
            put_buffer("no ", i + 1, 40);
            *game_options[i].value = false;
            if (i + 1 < max) {
                i++;
            } else {
                i = 0;
            }
            break;
        default:
            bell();
            break;
        }
    }
}
