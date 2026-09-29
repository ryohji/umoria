// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How well and how often this character looks for what is hidden

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_search_skill.h"

// No externs.h here, and after #18-12-25C NOT ONE LINE THAT REACHES ANYTHING -- the
// third unit in a row that ends this way (the body's weight, the attack bonuses,
// this). The two numbers start life in the race table and the class table, but this
// file never reads either: creation looks them up and hands the answers in.

// THE PLACE THE TWO NUMBERS LIVE. They were the srh and fos fields of the
// character's record until #18-12-25C; now these two shorts are the only place the
// answers live, and the five windows below are the only way to reach them.
// (`struct misc` is down to eight fields.) The `the_chance()` / `the_frequency()`
// doors that step A used to reach py.misc went with the fields.
//
// ZERO IS TWO DIFFERENT THINGS HERE, and only for one of the two numbers. A chance
// of 0 is an honest answer -- search() really is handed 0 and finds nothing -- so a
// character who has not been rolled yet looks like one who cannot search. A
// FREQUENCY of 0 is NOT an ordinary answer: every real character has one from the
// race table, and randint(0) is not a question anybody asks. The caller that reads
// it guards with `<= 1` first, so a 0 here means "look every turn".
static int16_t the_chance;
static int16_t the_frequency;

int player_search_chance(void) {
    return the_chance;
}

int player_search_frequency(void) {
    // SMALLER IS BETTER HERE and it can be negative: a Halfling starts at -5
    // (player.c's race table) and good rings of searching take it lower still. The
    // caller that reads this as a rating turns it upside down; this window does not.
    return the_frequency;
}

void player_search_chance_set(int chance) {
    // Creation reading the race table, the debugging editor, and a saved file's
    // short put back.
    //
    // The cast is the store's own width, not a rule this window adds: the field was
    // int16_t and `p_ptr->misc.srh = r_ptr->srh` truncated exactly like this, so the
    // static is int16_t as well.
    the_chance = (int16_t)chance;
}

void player_search_frequency_set(int frequency) {
    // The same three shapes, one window each, because wizard.c sets the chance
    // WITHOUT setting this one -- see the header.
    the_frequency = (int16_t)frequency;
}

void player_search_skill_adjust(int chance_amount, int frequency_amount) {
    // The class's two modifiers at creation, and one piece of searching gear going
    // on or coming off. TWO AMOUNTS AND NOT ONE, because the caller owns the signs:
    // py_bonuses() passes (amount, -amount) and creation passes (msrh, mfos), both
    // of which are positive.
    the_chance = (int16_t)(the_chance + chance_amount);
    the_frequency = (int16_t)(the_frequency + frequency_amount);
}
