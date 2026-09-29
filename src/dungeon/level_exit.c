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
// one bit needs nothing from the rest of the game. Until #18-14-6 there was one
// hand-written extern below for the depth this unit writes, because the depth
// was still a global; it has a window of its own now (dungeon_level.h), so there
// is nothing left to declare here.

// The flag is owned here and is static: the only way in is through the four
// windows below. It came over from variable.c (#18-11-4C) with its initial value
// -- the old global had no initializer, so it started out false, and a new game
// begins with the level it is on unfinished. The name follows the windows rather
// than the old global (new_level_flag), which said "next level" where two of its
// eight setters meant "no next level, this one is simply over".
static bool level_over = false;

bool level_is_over(void) {
    return level_over;
}

void leave_for_level(int level) {
    // The depth moved into src/dungeon/dungeon_level.c at #18-14-6, which took over the
    // hand-written extern that used to stand here: this unit owns the pair, not
    // the number -- see the header.
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
