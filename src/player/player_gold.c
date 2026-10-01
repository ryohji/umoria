// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How much gold the player is carrying: where it is kept

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_gold.h"

// Owned here and static: the only way in is through the four windows below.
// An empty purse is what a program starts with; character creation puts the
// starting money in at the end.
static int32_t gold = 0;

int32_t player_gold(void) {
    return gold;
}

void player_gain_gold(int32_t amount) {
    gold += amount;
}

void player_pay_gold(int32_t amount) {
    gold -= amount;
}

void player_set_gold(int32_t amount) {
    gold = amount;
}
