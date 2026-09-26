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

// THE CHARGE IS STILL IN py.flags (step A). Every window goes through the
// pointer below, so #18-12-13C has one place to change: the pointer becomes the
// byte itself and this comment goes away.
extern player_type py;

static uint8_t *charge(void) {
    return &py.flags.confuse_monster;
}

int player_glowing_hands(void) {
    return *charge();
}

void player_glowing_hands_begin(void) {
    // Replacing, not adding. The game never stacks two charges: scrolls.c only
    // gets this far when the hands were dark.
    *charge() = 1;
}

void player_glowing_hands_spend(void) {
    // The whole charge goes at once, however big it was. Both fights ask
    // player_glowing_hands() first, so this is never reached with dark hands.
    *charge() = 0;
}

void player_glowing_hands_restore(int charge_from_file) {
    *charge() = (uint8_t)charge_from_file;
}
