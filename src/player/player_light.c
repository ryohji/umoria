// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// What is remembered about the character's light: where the answers live

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_light.h"

// The flag is owned here and is static: the only way in is through the two
// windows below. It has no initial value: dungeon() writes it from the light
// slot before the first turn.
static bool player_light;

bool player_has_light(void) {
    return player_light;
}

void set_player_has_light(bool lit) {
    player_light = lit;
}

// Whether the glow is on the map, owned here and static as well. The first
// move_light() of a game finds no glow drawn, because nothing has been drawn yet.
static bool player_light_drawn = false;

bool player_light_is_drawn(void) {
    return player_light_drawn;
}

void set_player_light_drawn(bool drawn) {
    player_light_drawn = drawn;
}
