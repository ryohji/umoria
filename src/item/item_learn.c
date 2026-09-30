// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// What the player learns by using a consumable item
//
// Kept apart from item_ident.c: the experience it grants reaches level_ops.c
// (prt_experience), which the records themselves do not need.

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

#include "inventory.h"
#include "item_learn.h"
#include "player_level.h"

inven_type *learn_item_effect(bool effect_identified, int *item_val) {
    inven_type *i_ptr = inventory_at(*item_val);

    if (effect_identified) {
        if (!known1_p(i_ptr)) {
            // use identified it, gain experience
            // round half-way case up
            player_gain_experience((i_ptr->level + (player_level() >> 1)) / player_level());
            prt_experience();

            identify(item_val);
            i_ptr = inventory_at(*item_val);
        }
    } else if (!known1_p(i_ptr)) {
        sample(i_ptr);
    }

    return i_ptr;
}
