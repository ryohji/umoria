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
//
// It also holds the records of what the player has learned about each kind of
// object (see item_ident.h). During step A of #18-9 the table itself is still
// the global object_ident in treasure.c and the windows below point at it;
// step C moves it in here.

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

#include "inventory.h"
#include "item_ident.h"

// The record for one kind of object, or NULL for a kind that has none.
//
// object_offset() answers which of the seven groups the kind belongs to
// (amulets, rings, staves, wands, scrolls, potions, mushrooms) or -1 for a kind
// that is never secret. Sixty-four records to a group, so the group number
// shifts up six places; the low six bits of subval say which kind inside the
// group. The bit above those six means "stacks as a single item" and is no part
// of the kind's number, which is why it is masked off -- kind 64 and kind 0
// share one record.
static uint8_t *record_of(inven_type *i_ptr) {
    int16_t group = object_offset(i_ptr);
    if (group < 0) {
        return NULL;
    }

    int index = (group << 6) + (i_ptr->subval & (ITEM_SINGLE_STACK_MIN - 1));
    return &object_ident[index];
}

bool item_kind_has_record(inven_type *i_ptr) {
    return record_of(i_ptr) != NULL;
}

bool item_kind_is_known(inven_type *i_ptr) {
    uint8_t *record = record_of(i_ptr);
    return record != NULL && (*record & OD_KNOWN1) != 0;
}

bool item_kind_was_tried(inven_type *i_ptr) {
    uint8_t *record = record_of(i_ptr);
    return record != NULL && (*record & OD_TRIED) != 0;
}

void item_kind_mark_known(inven_type *i_ptr) {
    uint8_t *record = record_of(i_ptr);
    if (record == NULL) {
        return;
    }
    *record |= OD_KNOWN1;
    // Clear the tried mark, since the kind is now known by name.
    *record &= ~OD_TRIED;
}

void item_kind_mark_tried(inven_type *i_ptr) {
    uint8_t *record = record_of(i_ptr);
    if (record == NULL) {
        return;
    }
    *record |= OD_TRIED;
}

void item_kind_clear_tried(inven_type *i_ptr) {
    uint8_t *record = record_of(i_ptr);
    if (record == NULL) {
        return;
    }
    *record &= ~OD_TRIED;
}

uint8_t *item_kind_record_bytes(void) {
    return object_ident;
}

int item_kind_record_count(void) {
    return OBJECT_IDENT_SIZE;
}

inven_type *learn_item_effect(bool effect_identified, int *item_val) {
    inven_type *i_ptr = inventory_at(*item_val);

    if (effect_identified) {
        if (!known1_p(i_ptr)) {
            // use identified it, gain experience
            struct misc *m_ptr = &py.misc;

            // round half-way case up
            m_ptr->exp += (i_ptr->level + (m_ptr->lev >> 1)) / m_ptr->lev;
            prt_experience();

            identify(item_val);
            i_ptr = inventory_at(*item_val);
        }
    } else if (!known1_p(i_ptr)) {
        sample(i_ptr);
    }

    return i_ptr;
}
