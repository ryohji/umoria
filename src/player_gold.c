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

// Owned here and static: the only way in is through the four windows below.
// It came over from py.misc.au (#18-12-1C) with its initial value -- the field
// was inside a struct with no initializer, so an empty purse is what a program
// starts with, and character creation puts the starting money in at the end.
//
// The name follows the window rather than the old field: "au" was aurum.
static int32_t gold = 0;

int32_t player_gold(void) {
    return gold;
}

void player_gain_gold(int32_t amount) {
    gold += amount;
}

void player_pay_gold(int32_t amount) {
    gold -= amount;
}

void player_set_gold(int32_t amount) {
    gold = amount;
}
