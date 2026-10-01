// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Whether the screen has been flushed: where the flag lives

#include "config.h"
#include "constant.h"
#include "types.h"

#include "screen_touched.h"

// No externs.h here, the same as panel.c, stores.c, options.c, stats.c,
// inventory.c, progress.c, score_death.c, player_pos.c, hp_table.c,
// player_light.c and missile_serial.c: one bit needs nothing from the rest of
// the game.

// Flag indicating whether the screen has been flushed. Starts false.
static bool screen_flushed = false;

void note_screen_flushed(void) {
    screen_flushed = true;
}

bool screen_was_flushed(void) {
    return screen_flushed;
}

void forget_screen_flushed(void) {
    screen_flushed = false;
}
