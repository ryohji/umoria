// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How much gold the player is carrying: where it is kept

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_gold.h"

// No externs.h here, the same as panel.c, stores.c, options.c, stats.c,
// inventory.c, progress.c, score_death.c, player_pos.c, hp_table.c,
// player_light.c, missile_serial.c, inven_command_state.c, screen_touched.c,
// level_exit.c, pending_teleport.c, input_ended.c, running.c and
// command_state.c: remembering one number needs nothing from the rest of the
// game. In particular this module does not know prices, does not know what a
// thief can reach, and does not print anything.

// The amount still lives in py.misc.au (player.c:17). It comes in here at
// #18-12-1C, where this declaration becomes a static of its own and the field
// leaves the struct. Declared here rather than taken from externs.h so that
// what this module touches is one line long and visible.
extern player_type py;

int32_t player_gold(void) {
    return py.misc.au;
}

void player_gain_gold(int32_t amount) {
    py.misc.au += amount;
}

void player_pay_gold(int32_t amount) {
    py.misc.au -= amount;
}

void player_set_gold(int32_t amount) {
    py.misc.au = amount;
}
