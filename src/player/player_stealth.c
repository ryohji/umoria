// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How quietly this character moves

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_stealth.h"

// No externs.h here: this module reaches nothing outside itself. The two tables
// the number starts in (race[] and class[]) are read by creation, which hands the
// answer in; this file reads neither.

// THE PLACE THE ANSWER LIVES. This one short is the only place the answer lives,
// and the three windows below are the only way to reach it.
//
// int16_t, WHICH IS THE FIELD'S OWN WIDTH and far more room than the answer needs:
// the game can only reach -1 through 18. The width is kept because the saved file
// keeps a short and because nothing is gained by narrowing it (the number goes
// straight into an int at every reader).
//
// ZERO IS A CHARACTER WHO HAS NOT BEEN ROLLED AND ALSO A REAL ANSWER -- a Human's
// racial base is zero. It cannot survive creation, though: every class adds at least
// one, so a rolled character is never at zero unless gear takes it back there.
static int16_t the_halvings;

int player_stealth(void) {
    return the_halvings;
}

void player_stealth_set(int stealth) {
    // The race's base, the saved file's short put back, and the wizard screen's
    // prompt. One sentence for all three, because all three are a plain replacement.
    the_halvings = (int16_t)stealth;
}

void player_stealth_adjust(int amount) {
    // The class's mstl at creation and the gear in py_bonuses(), where the caller's
    // factor has already made `amount` negative if the thing is coming off. This was
    // `m_ptr->stl += c_ptr->mstl;` and `py.misc.stl += amount;`: one statement each
    // then, one statement now, so the store is still touched once.
    the_halvings = (int16_t)(the_halvings + amount);
}
