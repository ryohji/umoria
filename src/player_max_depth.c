// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How deep this character has ever been

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_max_depth.h"

// No externs.h here, the same as the fourteen questions before this one. This is
// the first one out of `struct misc`.

// THE RECORD ITSELF. It was py.misc.max_dlv until #18-12-16C; now this one short
// is the only place it lives, and the windows below are the only way to reach it.
//
// Zero -- "has never gone below the town" -- is where every character starts, so
// there is nothing to set up when a game begins.
//
// With this short moved, `struct misc` holds twenty-one fields. It is the first
// question to leave that struct, the way the purse was the first to leave and
// then twelve more followed out of `struct flags` until it was empty.
static uint16_t the_deepest;

int player_max_depth(void) {
    return the_deepest;
}

void player_note_depth_reached(int level) {
    // The comparison that used to be dungeon.c's "Check for a maximum level".
    // Keeping the deeper of the two is the whole meaning of the number, so it
    // belongs on this side of the window: there is no way in from outside that
    // can make the record shallower.
    if (level > the_deepest) {
        the_deepest = (uint16_t)level;
    }
}

void player_max_depth_set(int level) {
    // Replacing, not keeping the deeper one. The saved file's short is the whole
    // record, and putting it back has to give the number that was written, so
    // this door does not compare (player_max_depth.h says who may use it).
    the_deepest = (uint16_t)level;
}
