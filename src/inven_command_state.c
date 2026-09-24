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

// In this step the character is still the global doing_inven (variable.c), so
// that exactly one copy of it exists while moria1.c, dungeon.c and store2.c move
// over to the windows; #18-11-3C brings the storage in here and makes it static.
// Declared here rather than through externs.h for the reason given above.
extern char doing_inven;

char pending_inven_command(void) {
    return doing_inven;
}

void suspend_inven_command(char command) {
    doing_inven = command;

    // Both halves of suspending, in one place: see the header for what goes
    // wrong when only one of them happens.
    forget_screen_flushed();
}

void finish_inven_command(void) {
    doing_inven = 0;
}
