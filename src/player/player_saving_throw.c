// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How well this character shrugs off a spell, a trap or a curse

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_saving_throw.h"

// No externs.h here, the same as the twenty questions before this one.

// THE NUMBER ITSELF. It was py.misc.save until #18-12-21C; now this one short is
// the only place the answer lives, and the windows below are the only way to
// reach it. (`struct misc` is down to fourteen fields.)
//
// ZERO IS WHERE A CHARACTER STARTS AND ALSO A REAL ANSWER: nothing is chosen yet
// until create.c picks a race, and a Human's racial base happens to be zero too.
// Unlike the disarming skill next door, NOTHING IS BAKED IN at creation, so a
// Human really does sit at exactly zero until a class is picked, whatever the
// stats rolled.
static int16_t the_chance;

int player_saving_throw(void) {
    return the_chance;
}

void player_saving_throw_set(int chance) {
    // The race's base, the saved file's short put back, and the wizard screen's
    // prompt. One sentence for all three, because all three are a plain
    // replacement -- and nothing is added on the way in, which is where this
    // question is simpler than the disarming skill next door.
    the_chance = (int16_t)chance;
}

void player_saving_throw_adjust(int chance) {
    // The class's msav, added to whatever the race left here. This was
    // `m_ptr->save += c_ptr->msav;` in create.c: one statement then, one
    // statement now, so the store is still touched once.
    the_chance = (int16_t)(the_chance + chance);
}
