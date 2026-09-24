// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Whether this level is finished: where the flag lives

#include "config.h"
#include "constant.h"
#include "types.h"

#include "level_exit.h"

// No externs.h here, the same as panel.c, stores.c, options.c, stats.c,
// inventory.c, progress.c, score_death.c, player_pos.c, hp_table.c,
// player_light.c, missile_serial.c, inven_command_state.c and screen_touched.c:
// one bit and one depth need nothing from the rest of the game. That leaves two
// symbols to declare, so they are declared here rather than dragging in the
// global header (which pulls ncurses along with it) for two lines -- the same
// choice as stats.c and object_levels.c.

// In this step the flag is still the global new_level_flag (variable.c), so that
// exactly one copy of it exists while dungeon.c, moria1.c, moria3.c and
// scrolls.c move over to the windows; #18-11-4C brings the storage in here and
// makes it static.
extern bool new_level_flag;

// The current depth stays a global for now: it is in another group (the dungeon
// itself, 44 references) and is not what this unit is about. It is here because
// leave_for_level() owns the pair -- see the header. Whoever encapsulates
// dun_level takes this line over.
extern int16_t dun_level;

bool level_is_over(void) {
    return new_level_flag;
}

void leave_for_level(int level) {
    dun_level = (int16_t)level;

    // Both halves of leaving, in one place: see the header for what goes wrong
    // when only one of them happens.
    end_level();
}

void end_level(void) {
    new_level_flag = true;
}

void begin_level(void) {
    new_level_flag = false;
}
