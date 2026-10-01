// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Whether the character is resting, and for how many more turns

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_resting.h"

// No externs.h here. And no player_status_flags.h either: PY_REST is a different
// question and its callers keep it (player_resting.h says why this one does not
// fold the pair the way player_timed_effects.c does).

// THE ANSWER ITSELF. This one short is the only place it lives, and the windows
// below are the only way to reach it.
//
// No reset window: zero means "not resting", which is where a new character
// starts, the same as the status word and the eighteen clocks. Loading a saved
// game writes it through player_rest_set() (save.c).
static int16_t turns_left;

bool player_resting(void) {
    return turns_left != 0;
}

int player_rest_turns(void) {
    return turns_left;
}

bool player_rest_is_until_healed(void) {
    return turns_left < 0;
}

void player_rest_set(int rest_turns) {
    turns_left = (int16_t)rest_turns;
}

void player_rest_stop(void) {
    player_rest_set(0);
}

bool player_rest_count_down(void) {
    // Towards zero from whichever side it is on. The "not resting" case answers
    // false without touching the count: dungeon.c's old shape was
    // `if (rest > 0) ... else if (rest < 0) ...`, so zero moved nowhere.
    if (turns_left > 0) {
        turns_left = (int16_t)(turns_left - 1);
    } else if (turns_left < 0) {
        turns_left = (int16_t)(turns_left + 1);
    } else {
        return false;
    }

    return turns_left == 0;
}
