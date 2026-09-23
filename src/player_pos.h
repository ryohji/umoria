// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Where the player stands on the level that is currently in play

#ifndef PLAYER_POS_H
#define PLAYER_POS_H

// This is storage and only storage, the same as progress.h. The window does not
// range-check the row and column and does not know which squares are legal,
// because the old code did neither and doing it here would change behaviour
// rather than preserve it.
//
// The two getters are named after the player, not after this file. They are
// read 284 times, nearly always inside an expression that already says what it
// is doing -- cave[player_row()][player_col()], los(player_row(), player_col(),
// y, x) -- and player_pos_row() would only add noise at every one of them. The
// two entry points that are not reads keep the module's prefix, so a reader
// searching for who moves the player finds them by name.

// The player's row and column in cave[][].
int16_t player_row(void);
int16_t player_col(void);

// Puts the player somewhere. Both the callers that step one square and the ones
// that teleport across the level come through here. Row and column are always
// set together; no caller ever changed one without the other.
//
// The parameters are int because every caller has int coordinates to hand. The
// narrowing to int16_t happens here instead of at 5 assignment sites, and it is
// the same narrowing those assignments were already doing.
void player_place(int row, int col);

// Forgets where the player was, which is what generating a new level does
// before deciding where to put the player (generate.c). It writes -1, a row and
// column that are on no level. Nothing reads the -1 back -- no caller compares
// the position with -1 or with zero -- so this is a reset and not a "nowhere"
// state that anything tests for.
void player_pos_forget(void);

#endif // PLAYER_POS_H
