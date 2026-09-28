// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Whether the character's hands are glowing, ready to confuse what they touch

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_glowing_hands.h"

// No externs.h here, the same as the twelve questions before this one -- which
// matters more here than usual, because externs.h is where the unrelated
// function confuse_monster() is declared (player_glowing_hands.h says why the
// names collide).

// THE ANSWER ITSELF. It was py.flags.confuse_monster until #18-12-13C; now this
// one byte is the only place it lives, and the windows below are the only way to
// reach it.
//
// No reset window: zero means "the hands are not glowing", which is where every
// character starts and where every blow that connects leaves them. Only scroll
// 11 and a saved file ever put something else here.
static uint8_t the_charge;

int player_glowing_hands(void) {
    return the_charge;
}

void player_glowing_hands_begin(void) {
    // Replacing, not adding. The game never stacks two charges: scrolls.c only
    // gets this far when the hands were dark.
    the_charge = 1;
}

void player_glowing_hands_spend(void) {
    // The whole charge goes at once, however big it was. Both fights ask
    // player_glowing_hands() first, so this is never reached with dark hands.
    the_charge = 0;
}

void player_glowing_hands_restore(int charge_from_file) {
    the_charge = (uint8_t)charge_from_file;
}
