// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Which of the eight races this character is

#include "config.h"
#include "constant.h"
#include "types.h"
#include "externs.h"

#include "player_race.h"

// THE ROW NUMBER ITSELF. This one byte is the only place the answer lives, and
// the windows below are the only way to reach it.
//
// ZERO IS WHERE A CHARACTER STARTS AND ALSO A REAL ANSWER: nothing is chosen yet
// until the race menu is answered, and row zero happens to be Human. The byte is
// NOT A QUANTITY -- it is a row, so there is nothing here to add to and no
// `_adjust` window to go with the setter.
static uint8_t the_row;

int player_race(void) {
    return the_row;
}

void player_race_set(int row) {
    // The race menu and the saved file's byte put back. One sentence for both,
    // because both are a plain replacement -- and there is no `_adjust` to go with
    // it, because a character does not become more of a Dwarf.
    //
    // The cast is the store's own width (uint8_t), not a rule this window adds.
    the_row = (uint8_t)row;
}

const char *player_race_name(void) {
    // Not bounds-checked, deliberately (findings.md 24). The menu cannot produce a
    // bad row and nothing else writes this but a saved file.
    return race[the_row].trace;
}
