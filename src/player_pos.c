// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Where the player stands: the record of it, and the only ways to touch it

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_pos.h"

// No externs.h here, the same as panel.c, stores.c, options.c, stats.c,
// inventory.c and progress.c: two coordinates need nothing from the rest of the
// game. That leaves two symbols to declare, so they are declared here rather
// than dragging in the global header (which pulls ncurses along with it) for
// two lines -- the reason spelled out in stats.c:17-23.
//
// The definitions are still the ones in player.c. That is deliberate for now:
// this step puts the window and its tests in place without moving anything, so
// the game's behaviour can not have changed. Once every caller in src/ and
// tests/ goes through the window, the definitions move in here and become
// static, and these two lines and the two in externs.h go away together.
extern int16_t char_row;
extern int16_t char_col;

int16_t player_row(void) {
    return char_row;
}

int16_t player_col(void) {
    return char_col;
}

void player_place(int row, int col) {
    char_row = (int16_t)row;
    char_col = (int16_t)col;
}

void player_pos_forget(void) {
    player_place(-1, -1);
}
