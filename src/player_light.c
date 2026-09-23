// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Whether the character is carrying a light that is still burning: where the
// answer lives

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_light.h"

// No externs.h here, the same as panel.c, stores.c, options.c, stats.c,
// inventory.c, progress.c, score_death.c, player_pos.c and hp_table.c: one
// remembered flag needs nothing from the rest of the game.

// Step A keeps the flag where it has always been (variable.c) and only adds the
// window over it, so this commit cannot change behaviour. Step C moves the
// definition in here and makes it static.
extern bool player_light;

bool player_has_light(void) {
    return player_light;
}

void set_player_has_light(bool lit) {
    player_light = lit;
}
