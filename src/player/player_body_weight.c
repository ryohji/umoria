// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How heavy this character's body is

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_body_weight.h"

// This file reaches out to nothing at all. Creation looks up the race's weight
// columns itself and hands the result in.

// THE POUNDS THEMSELVES. This one short is the only place the answer lives, and the
// two windows below are the only way to reach it.
//
// ZERO IS WHERE A CHARACTER STARTS and it is not a real answer -- a body with no
// weight is a character that creation has not reached yet. THE TESTS LEAN ON THAT
// ZERO: tests/check_strength_test.c:244 expects a carrying limit of 1300, which is
// 10 * PLAYER_WEIGHT_CAP + 0.
static uint16_t the_pounds;

int player_body_weight(void) {
    return the_pounds;
}

void player_body_weight_set(int pounds) {
    // Creation (one line per sex), a saved file's short put back, and the
    // wizard's tweak. One sentence for all three, because all three are a plain
    // replacement -- and there is no `_adjust` to go with it, because NOBODY ADDS
    // TO THIS NUMBER. The character does not put on weight.
    //
    // The cast keeps the width as a uint16_t.
    the_pounds = (uint16_t)pounds;
}
