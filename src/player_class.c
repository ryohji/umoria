// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Which of the six classes this character is

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_class.h"

// No externs.h here, the same as the twenty-seven questions before this one.

// THE PLACE THE ANSWER LIVES, for the length of step A only: still `py.misc.pclass`,
// reached through one door so that #18-12-28C can move it by changing this one
// function and nothing else. THAT COMMIT DELETES `struct misc` OUTRIGHT, because
// this is the last field in it.
extern player_type py;

static uint8_t *the_row(void) {
    return &py.misc.pclass;
}

// THE TABLE STAYS WHERE IT IS. This is the one line that reaches out, the same
// arrangement player_race.c has for the race table and player_level.c for the price
// list -- class[] is one of the twenty read-only constant tables in externs.h and
// that group is out of scope for #18. Nothing in the game writes it.
extern class_type class[MAX_CLASS];

int player_class(void) {
    return *the_row();
}

void player_class_set(int row) {
    // The class menu, the zero it writes before the menu loop, and a saved byte put
    // back. One sentence for all three, because all three are a plain replacement.
    //
    // The cast is the store's own width, not a rule this window adds: the field is a
    // uint8_t and `py.misc.pclass = cl[j];` truncated exactly like this.
    *the_row() = (uint8_t)row;
}

const char *player_class_title(void) {
    // No bounds check on the row, the same as the four callers had none (ledger
    // observation 24). A row past the end of the table is a row past the end of the
    // table, exactly as `class[py.misc.pclass].title` was.
    return class[*the_row()].title;
}

int player_class_spell_type(void) {
    // NONE, MAGE or PRIEST. The fifteen callers compare the answer themselves; this
    // window does not turn the number into a yes or a no (see the header).
    return class[*the_row()].spell;
}
