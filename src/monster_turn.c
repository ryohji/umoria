// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Which monster creatures() is acting for: where the number lives

#include "config.h"
#include "constant.h"
#include "types.h"

#include "monster_turn.h"

// No externs.h here, the same as panel.c, stores.c, options.c, stats.c,
// inventory.c, progress.c, score_death.c, player_pos.c, hp_table.c,
// player_light.c, missile_serial.c and screen_touched.c: one number needs
// nothing from the rest of the game.

// The number is owned here and is static: the only way in is through the four
// windows. It came over from variable.c (#18-14-1) with its type and its
// initial value -- `int hack_monptr = -1;` -- so a new game starts at "nobody's
// turn", and so does a restored one (save.c never wrote it).
//
// -1 IS NOT A MONSTER AND CANNOT BE ONE. Monster numbers start at MIN_MONIX (2),
// which is what lets one comparison serve as both "the walk has not got here
// yet" and "there is no walk".
//
// FOR ONE STEP THERE ARE TWO OF THESE. The old `hack_monptr` is still in
// variable.c and the game is still using it; nothing calls the windows below
// yet, so the row here is only reachable from the unit test. The step that
// points creature.c, misc1.c, moria3.c and game_state.c at the windows moves
// the storage at the same time, and the step after that deletes the old line.
static int the_turn = -1;

void monster_turn_begin(int index) {
    the_turn = index;
}

void monster_turn_end(void) {
    the_turn = -1;
}

bool monster_delete_may_shift(int index) {
    // This is the comparison both readers used to spell out for themselves,
    // `hack_monptr < i`, and it is unchanged: strictly less than, so a monster
    // asking about its own entry gets a no.
    return the_turn < index;
}

int monster_turn_index(void) {
    return the_turn;
}
