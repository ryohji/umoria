// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How heavy this character's body is

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_body_weight.h"

// No externs.h here, the same as the twenty-two questions before this one. This
// one does not even need the read-only table the race reached for -- creation
// looks up the race's weight columns itself and hands the result in.

// THE STORE. Still the field in the character's record until #18-12-23C, when
// these two lines become a `static uint16_t` and `struct misc` drops to twelve
// fields.
extern player_type py;

static uint16_t *the_pounds(void) {
    return &py.misc.wt;
}

int player_body_weight(void) {
    return *the_pounds();
}

void player_body_weight_set(int pounds) {
    // Creation (one line per sex), a saved file's short put back, and the
    // wizard's tweak. One sentence for all three, because all three are a plain
    // replacement -- and there is no `_adjust` to go with it, because NOBODY ADDS
    // TO THIS NUMBER. The character does not put on weight.
    //
    // The cast is the field's own width, not a rule this window adds: `wt` is a
    // uint16_t and `py.misc.wt = randnor(...)` truncated exactly like this.
    *the_pounds() = (uint16_t)pounds;
}
