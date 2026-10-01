// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Where the player stands: the record of it, and the only ways to touch it

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_pos.h"

// This file needs nothing declared at all -- the record it hands out is its own.
// Every caller in src/ and tests/ goes through the four entry points below; the
// record is static, and the compiler is what guarantees nobody reaches around the
// window.
//
// No initialiser, which is what player.c had as well. The game writes the
// position (generate.c, or the save file's reader) before anything reads it.
static int16_t char_row;
static int16_t char_col;

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
