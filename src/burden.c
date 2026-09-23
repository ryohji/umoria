// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// What the character carries is more than they can manage: where the two
// answers live

#include "config.h"
#include "constant.h"
#include "types.h"

#include "burden.h"

// No externs.h here, the same as panel.c, stores.c, options.c, stats.c,
// inventory.c, progress.c, score_death.c, player_pos.c, hp_table.c and
// player_light.c: two remembered numbers need nothing from the rest of the game.

// Both used to be globals in variable.c, declared in externs.h and written from
// six files. Nothing outside this file names them now, so they are static: the
// windows below are the only way in.
//
// The starting values are the ones variable.c spelled out: no weapon is too
// heavy for an empty hand, and an empty pack costs no speed.
static bool weapon_heavy = false;
static int pack_heavy = 0;

bool weapon_is_too_heavy(void) {
    return weapon_heavy;
}

void set_weapon_too_heavy(bool too_heavy) {
    weapon_heavy = too_heavy;
}

int pack_speed_penalty(void) {
    return pack_heavy;
}

void set_pack_speed_penalty(int steps) {
    pack_heavy = steps;
}
