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

// The pair still lives in py.flags (player.c:17). It comes in here at
// #18-12-2C, where these become statics of their own and the two fields leave
// the struct. Declared here rather than taken from externs.h so that what this
// module touches is one line long and visible.
extern player_type py;

int16_t player_food(void) {
    return py.flags.food;
}

void player_gain_food(int amount) {
    // The first bite forgives a starvation debt: what add_food() did before
    // adding anything. Without it a starving character would have to eat back
    // the whole deficit before the counter meant anything again.
    if (py.flags.food < 0) {
        py.flags.food = 0;
    }
    py.flags.food = (int16_t)(py.flags.food + amount);
}

void player_burn_food(int amount) {
    py.flags.food = (int16_t)(py.flags.food - amount);
}

void player_set_food(int16_t amount) {
    py.flags.food = amount;
}

void player_digest(void) {
    py.flags.food = (int16_t)(py.flags.food - py.flags.food_digested);
}

int16_t player_digestion(void) {
    return py.flags.food_digested;
}

void player_set_digestion(int16_t amount) {
    py.flags.food_digested = amount;
}

void player_adjust_digestion(int delta) {
    py.flags.food_digested = (int16_t)(py.flags.food_digested + delta);
}
