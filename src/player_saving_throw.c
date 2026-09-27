// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How well this character shrugs off a spell, a trap or a curse

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_saving_throw.h"

// No externs.h here, the same as the nineteen questions before this one.

// THE NUMBER IS STILL IN py.misc (step A). Every window goes through the pointer
// below, so #18-12-21C has one place to change: the pointer becomes the number
// itself and this comment goes away.
extern player_type py;

static int16_t *the_chance(void) {
    return &py.misc.save;
}

int player_saving_throw(void) {
    return *the_chance();
}

void player_saving_throw_set(int chance) {
    // The race's base, the saved file's short put back, and the wizard screen's
    // prompt. One sentence for all three, because all three are a plain
    // replacement -- and nothing is added on the way in, which is where this
    // question is simpler than the disarming skill next door.
    *the_chance() = (int16_t)chance;
}

void player_saving_throw_adjust(int chance) {
    // The class's msav, added to whatever the race left here. This was
    // `m_ptr->save += c_ptr->msav;` in create.c: one statement then, one
    // statement now, so the store is still touched once.
    *the_chance() = (int16_t)(*the_chance() + chance);
}
