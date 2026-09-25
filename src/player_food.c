// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How full the player's stomach is, and how fast it empties

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_food.h"

// No externs.h here, the same as player_gold.c and the rest: a counter and its
// rate need nothing from the game around them. In particular this module does
// not know the hunger thresholds, does not touch the status flags, does not
// deal damage and does not print anything.

// Owned here and static: the only way in is through the windows below. The pair
// came over from py.flags.food and py.flags.food_digested (#18-12-2C) with
// their initial values -- the fields were inside a struct with no initializer,
// so an empty stomach that digests nothing is what a program starts with, and
// main.c puts the 7500 and the 2 in once the character exists.
//
// The names follow the windows rather than the old fields: "food" was a
// counter, not a larder, and "food_digested" was a rate.
static int16_t food = 0;
static int16_t digestion = 0;

int16_t player_food(void) {
    return food;
}

void player_gain_food(int amount) {
    // The first bite forgives a starvation debt: what add_food() did before
    // adding anything. Without it a starving character would have to eat back
    // the whole deficit before the counter meant anything again.
    if (food < 0) {
        food = 0;
    }
    food = (int16_t)(food + amount);
}

void player_burn_food(int amount) {
    food = (int16_t)(food - amount);
}

void player_set_food(int16_t amount) {
    food = amount;
}

void player_digest(void) {
    food = (int16_t)(food - digestion);
}

int16_t player_digestion(void) {
    return digestion;
}

void player_set_digestion(int16_t amount) {
    digestion = amount;
}

void player_adjust_digestion(int delta) {
    digestion = (int16_t)(digestion + delta);
}
