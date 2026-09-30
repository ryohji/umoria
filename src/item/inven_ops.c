// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Operations on the pack that need more than the inventory window: for now
// finding where a kind of item sits in it
//
// Moved out of misc3.c unchanged (#42); its prototype stays in externs.h.
// inventory.c is left alone, since it is a state module that others link
// on its own.

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"
#include "inventory.h"

// Finds range of item in inventory list -RAK-
int find_range(int item1, int item2, int *j, int *k) {
    int i = 0;

    *j = -1;
    *k = -1;

    inven_type *i_ptr = inventory_at(0);

    bool flag = false;

    while (i < inventory_count()) {
        if (!flag) {
            if ((i_ptr->tval == item1) || (i_ptr->tval == item2)) {
                flag = true;
                *j = i;
            }
        } else {
            if ((i_ptr->tval != item1) && (i_ptr->tval != item2)) {
                *k = i - 1;
                break;
            }
        }
        i++;
        i_ptr++;
    }

    if (flag && (*k == -1)) {
        *k = inventory_count() - 1;
    }

    return flag;
}
