// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Whether a teleport is waiting to happen: where the flag lives

#include "config.h"
#include "constant.h"
#include "types.h"

#include "pending_teleport.h"

// No externs.h here: one bit needs nothing from the rest of the game. In
// particular this module does not know how to teleport anybody -- it only holds
// the note.

// The flag is owned here and is static: the only way in is through the four
// windows below. The initial value is false, so a new game begins with no
// teleport waiting.
static bool teleport_pending = false;

void schedule_teleport(void) {
    teleport_pending = true;
}

bool teleport_is_pending(void) {
    return teleport_pending;
}

void teleport_done(void) {
    teleport_pending = false;
}

void forget_pending_teleport(void) {
    teleport_pending = false;
}
