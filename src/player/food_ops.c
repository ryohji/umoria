// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Eating: filling the stomach, and what happens when it is too full

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"
#include "player_food.h"
#include "player_timed_effects.h"

// It is the sibling of player_food.c rather than a part of it: the stomach is
// player_food.c's, but the penalty for overeating prints and slows the player,
// which are not (player_food.h says why).

// Add to the players food time -RAK-
void add_food(int num) {

    // 飢えの借金を消すのは窓口の中（#18-12-2A）。腹だけの規則で、ほかに
    // 訊く人がいないので内側に入れた。ここに残るのは食べすぎの罰 ——
    // 画面に言い、速さを落とす。どちらも腹の話ではない。
    player_gain_food(num);

    if (player_food() > PLAYER_FOOD_MAX) {
        msg_print("You are bloated from overeating.");

        // Calculate how much of num is responsible for the bloating. Give the
        // player food credit for 1/50, and slow him for that many turns also.
        int extra = player_food() - PLAYER_FOOD_MAX;
        if (extra > num) {
            extra = num;
        }
        int penalty = extra / 50;

        player_timed_add(PLAYER_TIMED_SLOWNESS, penalty);
        if (extra == num) {
            player_set_food((int16_t)(player_food() - num + penalty));
        } else {
            player_set_food((int16_t)(PLAYER_FOOD_MAX + penalty));
        }
    } else if (player_food() > PLAYER_FOOD_FULL) {
        msg_print("You are full.");
    }
}
