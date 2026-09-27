// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How good this character is at getting traps and locks open

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_disarm.h"

// No externs.h here, the same as the eighteen questions before this one.

// THE NUMBER ITSELF. It was py.misc.disarm until #18-12-20C; now this one short
// is the only place the answer lives, and the windows below are the only way to
// reach it. (`struct misc` is down to fifteen fields.)
//
// ZERO IS WHERE A CHARACTER STARTS AND ALSO A REAL ANSWER: nothing is chosen yet
// until create.c picks a race, and a Human's racial base happens to be zero too
// -- with a dexterity of 8 through 12 the creation-time bonus is zero as well,
// so a Human really does sit at zero until a class is picked.
static int16_t the_chance_number;

static int16_t *the_chance(void) {
    return &the_chance_number;
}

int player_disarm(void) {
    return *the_chance();
}

void player_disarm_set(int chance) {
    // The race's base with the creation-time dexterity bonus baked in, the saved
    // file's short put back, and the wizard screen's prompt. One sentence for all
    // three, because all three are a plain replacement.
    *the_chance() = (int16_t)chance;
}

void player_disarm_adjust(int chance) {
    // The class's mdis, added to whatever the race left here. This was
    // `m_ptr->disarm += c_ptr->mdis;` in create.c: one statement then, one
    // statement now, so the store is still touched once.
    *the_chance() = (int16_t)(*the_chance() + chance);
}
