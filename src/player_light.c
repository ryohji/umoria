// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// What is remembered about the character's light: where the answers live

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_light.h"

// No externs.h here, the same as panel.c, stores.c, options.c, stats.c,
// inventory.c, progress.c, score_death.c, player_pos.c and hp_table.c: one
// remembered flag needs nothing from the rest of the game.

// The flag is owned here and is static: the only way in is through the two
// windows below. It came over from variable.c unchanged (#18-7-3C1), with its
// comment ("Player carrying light"), and it had no initial value there either --
// dungeon() writes it from the light slot before the first turn.
static bool player_light;

bool player_has_light(void) {
    return player_light;
}

void set_player_has_light(bool lit) {
    player_light = lit;
}

// Whether the glow is on the map, owned here and static as well. It came over
// from variable.c (#18-11-1C) with its comment ("Track if temporary light about
// player") and its initial value: the first move_light() of a game finds no glow
// drawn, because nothing has been drawn yet.
static bool player_light_drawn = false;

bool player_light_is_drawn(void) {
    return player_light_drawn;
}

void set_player_light_drawn(bool drawn) {
    player_light_drawn = drawn;
}
