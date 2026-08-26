// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// What the player learns by using a consumable item
//
// Extracted from the identical `ident` blocks in potions.c, eat.c and
// scrolls.c. The only difference between the three was that scrolls.c omitted
// the `i_ptr` reassignment, because it never reads i_ptr afterwards; returning
// the pointer lets each caller decide whether it needs it.
//
// This file deliberately depends on nothing but known1_p() / identify() /
// sample() / prt_experience() and the global player and inventory, so that the
// rule can be tested without pulling in the item effect switches.

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

#include "item_ident.h"

inven_type *learn_item_effect(bool effect_identified, int *item_val) {
    inven_type *i_ptr = &inventory[*item_val];

    if (effect_identified) {
        if (!known1_p(i_ptr)) {
            // use identified it, gain experience
            struct misc *m_ptr = &py.misc;

            // round half-way case up
            m_ptr->exp += (i_ptr->level + (m_ptr->lev >> 1)) / m_ptr->lev;
            prt_experience();

            identify(item_val);
            i_ptr = &inventory[*item_val];
        }
    } else if (!known1_p(i_ptr)) {
        sample(i_ptr);
    }

    return i_ptr;
}
