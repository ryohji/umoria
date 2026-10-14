// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// What the player has learned about each kind of object (see item_ident.h),
// reachable only through the windows below.

#include "config.h"
#include "constant.h"
#include "types.h"

#include "item_ident.h"

// One record per kind of secret object: seven groups of sixty-four. Starting
// out as zeroes says "nothing known, nothing tried", which is what a new
// character should have; loading a save file reads the whole table back byte
// for byte.
static uint8_t object_ident[OBJECT_IDENT_SIZE];

// Which of the seven groups of secret kinds an item belongs to, or -1 for a
// kind that is never secret.
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
