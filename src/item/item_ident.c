// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
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
// object (see item_ident.h), reachable only through the windows below. The
// table used to be a global in treasure.c, declared in externs.h, and eight
// lines in three files read or wrote it directly.

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

#include "inventory.h"
#include "item_ident.h"
#include "player_level.h"

// One record per kind of secret object: seven groups of sixty-four. Starting
// out as zeroes says "nothing known, nothing tried", which is what a new
// character should have; loading a save file reads the whole table back byte
// for byte.
static uint8_t object_ident[OBJECT_IDENT_SIZE];

// Which of the seven groups of secret kinds an item belongs to, or -1 for a
// kind that is never secret. Moved here from desc.c in #18-9-B: the five places
// there that called it were all working out which record to use, and this is
// the only caller left.
static int16_t group_of(inven_type *t_ptr) {
    switch (t_ptr->tval) {
    case TV_AMULET:
        return 0;
    case TV_RING:
        return 1;
    case TV_STAFF:
        return 2;
    case TV_WAND:
        return 3;
    case TV_SCROLL1:
    case TV_SCROLL2:
        return 4;
    case TV_POTION1:
    case TV_POTION2:
        return 5;
    case TV_FOOD:
        if ((t_ptr->subval & (ITEM_SINGLE_STACK_MIN - 1)) < MAX_MUSH) {
            return 6;
        }
        return -1;
    default:
        return -1;
    }
}

// The record for one kind of object, or NULL for a kind that has none.
//
// Sixty-four records to a group, so the group number shifts up six places; the
// low six bits of subval say which kind inside the group. The bit above those
// six means "stacks as a single item" and is no part of the kind's number,
// which is why it is masked off -- kind 64 and kind 0 share one record.
static uint8_t *record_of(inven_type *i_ptr) {
    int16_t group = group_of(i_ptr);
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
