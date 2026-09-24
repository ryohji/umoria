// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Whether a teleport is waiting to happen: where the flag lives

#include "config.h"
#include "constant.h"
#include "types.h"

#include "pending_teleport.h"

// No externs.h here, the same as panel.c, stores.c, options.c, stats.c,
// inventory.c, progress.c, score_death.c, player_pos.c, hp_table.c,
// player_light.c, missile_serial.c, inven_command_state.c and screen_touched.c:
// one bit needs nothing from the rest of the game. In particular this module does
// not know how to teleport anybody -- it only holds the note.

// In this step the flag is still the global teleport_flag (variable.c), so that
// exactly one copy of it exists while dungeon.c, moria3.c and misc3.c move over
// to the windows; #18-11-4C brings the storage in here and makes it static.
// Declared here rather than through externs.h for the reason given above.
extern bool teleport_flag;

void schedule_teleport(void) {
    teleport_flag = true;
}

bool teleport_is_pending(void) {
    return teleport_flag;
}

void teleport_done(void) {
    teleport_flag = false;
}

void forget_pending_teleport(void) {
    teleport_flag = false;
}
