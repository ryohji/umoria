// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How quietly this character moves

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_stealth.h"

// No externs.h here, the same as the twenty-six questions before this one. The two
// tables the number starts in (race[] and class[]) are read by creation, which hands
// the answers in; this file reaches neither.

// THE PLACE THE ANSWER LIVES, for the length of step A only: still `py.misc.stl`,
// reached through one door so that #18-12-27C can move it by changing this one
// function and nothing else.
extern player_type py;

static int16_t *the_halvings(void) {
    return &py.misc.stl;
}

int player_stealth(void) {
    return *the_halvings();
}

void player_stealth_set(int stealth) {
    // The race's base, the saved file's short put back, and the wizard screen's
    // prompt. One sentence for all three, because all three are a plain replacement.
    *the_halvings() = (int16_t)stealth;
}

void player_stealth_adjust(int amount) {
    // The class's mstl at creation and the gear in py_bonuses(), where the caller's
    // factor has already made `amount` negative if the thing is coming off. This was
    // `m_ptr->stl += c_ptr->mstl;` and `py.misc.stl += amount;`: one statement each
    // then, one statement now, so the store is still touched once.
    *the_halvings() = (int16_t)(*the_halvings() + amount);
}
