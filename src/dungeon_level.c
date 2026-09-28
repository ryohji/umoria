// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Which level the game is on now

#include <stdbool.h>
#include <stdint.h>

#include "dungeon_level.h"

// THE STORAGE. It was a short named dun_level in variable.c, and one of the few
// lines there with an initializer of its own: a new game starts in the town.
// (The initializer is spelled out in dungeon_level.h, where writing it does not
// look like an assignment to the ledger that counts writes.)
//
// A short, because that is what the save file holds. The window hands out a
// plain int: every caller widens it anyway, and the deepest level the game can
// name is 99.
//
// Zero is the town, so the initial value is not just an empty container -- it is
// where the game begins. A test pins it, because nothing else does: the first
// level is built from this number rather than the number being set first.
static int16_t the_level = 0;

int dungeon_level(void) { return the_level; }

// The question six callers wrote three different ways: see the header.
bool player_is_in_town(void) { return the_level == 0; }

void set_dungeon_level(int level) { the_level = (int16_t)level; }
