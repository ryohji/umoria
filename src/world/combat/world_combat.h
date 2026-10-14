// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Public interface of world/combat/.

#ifndef WORLD_COMBAT_H
#define WORLD_COMBAT_H

#include "config.h"
#include "constant.h"
#include "types.h"

// --- player_damage.c ---
void take_hit(int, const char *);

// --- projectiles.c ---
void fire_bolt(int, int, int, int, int, const char *);
void fire_ball(int, int, int, int, int, const char *);

// --- throw.c ---
void throw_object(void);

#endif // WORLD_COMBAT_H
