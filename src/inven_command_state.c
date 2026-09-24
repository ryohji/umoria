// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Which inventory command is waiting to be resumed: where the character lives

#include "config.h"
#include "constant.h"
#include "types.h"

#include "inven_command_state.h"
#include "screen_touched.h"

// No externs.h here, the same as panel.c, stores.c, options.c, stats.c,
// inventory.c, progress.c, score_death.c, player_pos.c, hp_table.c,
// player_light.c and missile_serial.c. The one thing this module needs from
// elsewhere is the screen flag, and it asks for that through its window.

// The character is owned here and is static: the only way in is through the three
// windows below. It came over from variable.c (#18-11-3C) with its initial value,
// so a new game starts with nothing waiting. The FIXME that sat on the old global
// (doing_inven: "was a bool, but also holds an ASCII character") is answered by
// the type being what it always was, a command character, with 0 for "none" --
// see the header.
static char pending_command = 0;

char pending_inven_command(void) {
    return pending_command;
}

void suspend_inven_command(char command) {
    pending_command = command;

    // Both halves of suspending, in one place: see the header for what goes
    // wrong when only one of them happens.
    forget_screen_flushed();
}

void finish_inven_command(void) {
    pending_command = 0;
}
