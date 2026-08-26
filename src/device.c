// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Magic device (staff / wand) use success calculation
//
// Extracted from the duplicated chance calculation in staffs.c and wands.c.
// The only difference between the two was the staff's constant penalty,
// which is now the `penalty` argument.
//
// This file deliberately depends on nothing but randint() so that the
// calculation can be tested without pulling in the display and global
// player state.

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

#include "device.h"

int device_use_chance(int save, int int_adj, int item_level, int penalty, int class_device_adj, int player_level, int confused) {
    int chance = save + int_adj - item_level - penalty + (class_device_adj * player_level / 3);

    if (confused > 0) {
        chance = chance / 2;
    }
    if ((chance < USE_DEVICE) && (randint(USE_DEVICE - chance + 1) == 1)) {
        chance = USE_DEVICE; // Give everyone a slight chance
    }
    if (chance <= 0) {
        chance = 1;
    }
    return chance;
}

bool device_use_succeeds(int chance) {
    return !(randint(chance) < USE_DEVICE);
}
