// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How well and how often this character looks for what is hidden

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_search_skill.h"

// No externs.h here, the same as the twenty-four questions before this one. The two
// numbers start life in the race table and the class table, but this file never
// reads either -- creation looks them up and hands the answers in, the same way the
// body's weight and the attack bonuses are handed in.

// THE PLACE THE TWO NUMBERS LIVE, for the length of step A only: still the fields of
// the character's record, reached through one door each so that #18-12-25C can move
// them by changing these two functions and nothing else.
extern player_type py;

static int16_t *the_chance(void) {
    return &py.misc.srh;
}

static int16_t *the_frequency(void) {
    return &py.misc.fos;
}

int player_search_chance(void) {
    return *the_chance();
}

int player_search_frequency(void) {
    // SMALLER IS BETTER HERE and it can be negative: a Halfling starts at -5
    // (player.c's race table) and good rings of searching take it lower still. The
    // caller that reads this as a rating turns it upside down; this window does not.
    return *the_frequency();
}

void player_search_chance_set(int chance) {
    // Creation reading the race table, the debugging editor, and a saved file's
    // short put back.
    //
    // The cast is the field's own width, not a rule this window adds:
    // `p_ptr->misc.srh = r_ptr->srh` truncated exactly like this.
    *the_chance() = (int16_t)chance;
}

void player_search_frequency_set(int frequency) {
    // The same three shapes, one window each, because wizard.c sets the chance
    // WITHOUT setting this one -- see the header.
    *the_frequency() = (int16_t)frequency;
}

void player_search_skill_adjust(int chance_amount, int frequency_amount) {
    // The class's two modifiers at creation, and one piece of searching gear going
    // on or coming off. TWO AMOUNTS AND NOT ONE, because the caller owns the signs:
    // py_bonuses() passes (amount, -amount) and creation passes (msrh, mfos), both
    // of which are positive.
    *the_chance() = (int16_t)(*the_chance() + chance_amount);
    *the_frequency() = (int16_t)(*the_frequency() + frequency_amount);
}
