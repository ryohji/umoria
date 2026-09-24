// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
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

// In this step the flag is still the global screen_change (variable.c), so that
// exactly one copy of it exists while io.c, creature.c and moria1.c move over to
// the windows; #18-11-3C brings the storage in here and makes it static.
// Declared here rather than through externs.h for the reason given above.
extern bool screen_change;

void note_screen_flushed(void) {
    screen_change = true;
}

bool screen_was_flushed(void) {
    return screen_change;
}

void forget_screen_flushed(void) {
    screen_change = false;
}
