// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
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

// Two remembered numbers: the only way in is through the windows below.
// The starting values are: no weapon is too heavy for an empty hand,
// and an empty pack costs no speed.
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
