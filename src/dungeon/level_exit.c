// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Whether this level is finished: where the flag lives

#include "config.h"
#include "constant.h"
#include "types.h"

#include "dungeon_level.h"
#include "level_exit.h"

// No externs.h here, the same as panel.c, stores.c, options.c, stats.c,
// inventory.c, progress.c, score_death.c, player_pos.c, hp_table.c,
// player_light.c, missile_serial.c, inven_command_state.c and screen_touched.c:
// one bit needs nothing from the rest of the game. The depth has a window of
// its own now (dungeon_level.h), so there is nothing left to declare here.

// The flag is owned here and is static: the only way in is through the four
// windows below. It starts out false, so a new game begins with the level it is
// on unfinished.
static bool level_over = false;

bool level_is_over(void) {
    return level_over;
}

void leave_for_level(int level) {
    // The depth lives in src/dungeon/dungeon_level.c: this unit owns the pair,
    // not the number -- see the header.
    set_dungeon_level(level);

    // Both halves of leaving, in one place: see the header for what goes wrong
    // when only one of them happens.
    end_level();
}

void end_level(void) {
    level_over = true;
}

void begin_level(void) {
    level_over = false;
}
