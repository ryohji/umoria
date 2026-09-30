// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// What happens to the player when hurt: for now the saving throw alone
//
// Moved out of misc3.c unchanged (#42); its prototype stays in externs.h.

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"
#include "player_class.h"
#include "player_level.h"
#include "player_saving_throw.h"
#include "stats.h"

// Saving throws for player character. -RAK-
bool player_saves(void) {
    // MPW C couldn't handle the expression, so split it into two parts
    int16_t temp = class_level_adj[player_class()][CLA_SAVE];

    // 数は窓口から、振るのはここ（#18-12-21B）。**窓口は「どれくらい強いか」を
    // 返し、この関数は「今回こらえたか」を返す** —— 判定には randint と
    // 上の temp（階級の段ごとの表）の両方が要るので、module へは入れない。
    if (randint(100) <= (player_saving_throw() + stat_adj(A_WIS) + (temp * player_level() / 3))) {
        return true;
    } else {
        return false;
    }
}
