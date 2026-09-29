// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How tall and how wide this level is

#include <stdint.h>

#include "dungeon_size.h"

// THE STORAGE. It was `int16_t cur_height, cur_width;` in variable.c -- one
// line, and the only line there that said anything about the shape of a level.
//
// Two shorts, kept as shorts because that is what the save file holds and what
// every caller's arithmetic was written against. The windows hand out plain
// ints: every reader widens them anyway, and nothing here is close to a short's
// limit (the biggest level is 66 by 198).
//
// Zero before the first level is made. Nothing reads them in that window -- the
// game makes a level or restores one before the first turn -- but the value is
// pinned by a test, because it is the one thing about this pair that no caller
// establishes.
static int16_t the_height;
static int16_t the_width;

int dungeon_height(void) { return the_height; }

int dungeon_width(void) { return the_width; }

// One setter for both halves: see the header on why there is no way to set one.
void set_dungeon_size(int height, int width) {
    the_height = (int16_t)height;
    the_width = (int16_t)width;
}
