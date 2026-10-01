// Copyright (c) 2026 Umoria Contributors
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

// This module includes no externs.h besides the screen flag (via its accessor).

// Pending inventory command character. 0 means none pending.
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
