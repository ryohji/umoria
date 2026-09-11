// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The rolling history of the messages shown on the top line -CJS-

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"
#include "messages.h"

// The ring itself, and the slot holding the newest message. Declared in
// externs.h for now, so that the call sites can be moved over one at a time;
// they used to be defined in variable.c.
vtype old_msg[MAX_SAVE_MSG];
int16_t last_msg = 0;

// Turns "how far back" into a slot. Adding the capacity keeps the result
// positive: C's % gives a negative answer for a negative left operand.
static int slot_of(int back) {
    return ((last_msg - back) % MAX_SAVE_MSG + MAX_SAVE_MSG) % MAX_SAVE_MSG;
}

const char *msg_history_recent(int back) {
    return old_msg[slot_of(back)];
}

void msg_history_push(const char *message) {
    last_msg = last_msg + 1 == MAX_SAVE_MSG ? 0 : last_msg + 1;

    (void)strncpy(old_msg[last_msg], message, VTYPESIZ);
    old_msg[last_msg][VTYPESIZ - 1] = '\0';
}

void msg_history_append(const char *message) {
    char *newest = old_msg[last_msg];
    size_t used = strlen(newest);

    // snprintf rather than sprintf: the caller checks that the two messages fit
    // on one line before asking for this, so nothing is truncated in practice,
    // but the size belongs in the call rather than in that reasoning.
    (void)snprintf(newest + used, VTYPESIZ - used, "  %s", message);
}

int msg_history_slot_count(void) {
    return MAX_SAVE_MSG;
}

char *msg_history_slot(int slot) {
    return old_msg[slot];
}

int msg_history_newest_slot(void) {
    return last_msg;
}

void msg_history_set_newest_slot(int slot) {
    last_msg = (int16_t)slot;
}
