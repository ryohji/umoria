// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The serial number that tells one batch of missiles from another: where the
// counter lives and how it steps

#include "config.h"
#include "constant.h"
#include "types.h"

#include "missile_serial.h"

// No externs.h here, the same as panel.c, stores.c, options.c, stats.c,
// inventory.c, progress.c, score_death.c, player_pos.c, hp_table.c and
// player_light.c: a counter needs nothing from the rest of the game. MAX_SHORT
// comes from constant.h.

// The counter is owned here and is static: the only way in is through the three
// windows below. It came over from variable.c (#18-11-2C) with its comment
// ("Counter for missiles") and its initial value, so the first batch of a new
// game is still stamped 1.
static int16_t missile_ctr = 0;

int16_t next_missile_serial(void) {
    // Step first, hand out afterwards, which is the order the old code used: the
    // counter starts at 0 and the first batch is stamped 1.
    if (missile_ctr == MAX_SHORT) {
        missile_ctr = -MAX_SHORT - 1;
    } else {
        missile_ctr++;
    }
    return missile_ctr;
}

int16_t missile_serial_value(void) {
    return missile_ctr;
}

void set_missile_serial(int16_t serial) {
    missile_ctr = serial;
}
