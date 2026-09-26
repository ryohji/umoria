// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Whether the character is resting, and for how many more turns

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_resting.h"

// No externs.h here, the same as the nine questions before this one. And no
// player_status_flags.h either: PY_REST is a different question and its callers
// keep it (player_resting.h says why this one does not fold the pair the way
// player_timed_effects.c does).

// THE COUNT IS STILL IN py.flags (step A). Every window goes through the pointer
// below, so #18-12-10C has one place to change: the pointer becomes the count
// itself and this comment goes away.
extern player_type py;

static int16_t *turns(void) {
    return &py.flags.rest;
}

bool player_resting(void) {
    return *turns() != 0;
}

int player_rest_turns(void) {
    return *turns();
}

bool player_rest_is_until_healed(void) {
    return *turns() < 0;
}

void player_rest_set(int rest_turns) {
    *turns() = (int16_t)rest_turns;
}

void player_rest_stop(void) {
    player_rest_set(0);
}

bool player_rest_count_down(void) {
    int16_t *left = turns();

    // Towards zero from whichever side it is on. The "not resting" case answers
    // false without touching the count: dungeon.c's old shape was
    // `if (rest > 0) ... else if (rest < 0) ...`, so zero moved nowhere.
    if (*left > 0) {
        *left = (int16_t)(*left - 1);
    } else if (*left < 0) {
        *left = (int16_t)(*left + 1);
    } else {
        return false;
    }

    return *left == 0;
}
