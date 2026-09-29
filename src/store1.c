// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Store code, updating store inventory, pricing objects

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"
#include "floor_items.h"
#include "stores.h"

static void insert_store(int, int, int32_t, inven_type *);
static void store_create(int);

// Check to see if he will be carrying too many objects -RAK-
bool store_check_num(inven_type *t_ptr, int store_num) {
    bool store_check = false;

    store_type *s_ptr = store_at(store_num);

    if (s_ptr->store_ctr < STORE_INVEN_MAX) {
        store_check = true;
    } else if (t_ptr->subval >= ITEM_SINGLE_STACK_MIN) {
        for (int i = 0; i < s_ptr->store_ctr; i++) {
            inven_type *i_ptr = &s_ptr->store_inven[i].sitem;

            // note: items with subval of gte ITEM_SINGLE_STACK_MAX only stack
            // if their subvals match
            if (i_ptr->tval == t_ptr->tval && i_ptr->subval == t_ptr->subval &&
                ((int)i_ptr->number + (int)t_ptr->number < 256) &&
                (t_ptr->subval < ITEM_GROUP_MIN || (i_ptr->p1 == t_ptr->p1))) {
                store_check = true;
            }
        }
    }

    return store_check;
}

// Insert INVEN_MAX at given location
static void insert_store(int store_num, int pos, int32_t icost, inven_type *i_ptr) {
    store_type *s_ptr = store_at(store_num);

    for (int i = s_ptr->store_ctr - 1; i >= pos; i--) {
        s_ptr->store_inven[i + 1] = s_ptr->store_inven[i];
    }

    s_ptr->store_inven[pos].sitem = *i_ptr;
    s_ptr->store_inven[pos].scost = -icost;
    s_ptr->store_ctr++;
}

// Add the item in INVEN_MAX to stores inventory. -RAK-
void store_carry(int store_num, int *ipos, inven_type *t_ptr) {
    *ipos = -1;

    int32_t icost, dummy;
    if (sell_price(store_num, &icost, &dummy, t_ptr) > 0) {
        store_type *s_ptr = store_at(store_num);

        int item_val = 0;
        int item_num = t_ptr->number;
        bool flag = false;
        int typ = t_ptr->tval;
        int subt = t_ptr->subval;
        do {
            inven_type *i_ptr = &s_ptr->store_inven[item_val].sitem;

            if (typ == i_ptr->tval) {
                if (subt == i_ptr->subval && // Adds to other item
                    subt >= ITEM_SINGLE_STACK_MIN &&
                    (subt < ITEM_GROUP_MIN || i_ptr->p1 == t_ptr->p1)) {
                    *ipos = item_val;
                    i_ptr->number += item_num;

                    // must set new scost for group items, do this only for items
                    // strictly greater than group_min, not for torches, this
                    // must be recalculated for entire group
                    if (subt > ITEM_GROUP_MIN) {
                        (void)sell_price(store_num, &icost, &dummy, i_ptr);
                        s_ptr->store_inven[item_val].scost = -icost;
                    } else if (i_ptr->number > 24) {
                        // must let group objects (except torches) stack over 24
                        // since there may be more than 24 in the group
                        i_ptr->number = 24;
                    }
                    flag = true;
                }
            } else if (typ > i_ptr->tval) { // Insert into list
                insert_store(store_num, item_val, icost, t_ptr);
                flag = true;
                *ipos = item_val;
            }
            item_val++;
        } while ((item_val < s_ptr->store_ctr) && (!flag));

        // Becomes last item in list
        if (!flag) {
            insert_store(store_num, (int)s_ptr->store_ctr, icost, t_ptr);
            *ipos = s_ptr->store_ctr - 1;
        }
    }
}

// Destroy an item in the stores inventory.  Note that if
// "one_of" is false, an entire slot is destroyed -RAK-
void store_destroy(int store_num, int item_val, int one_of) {
    store_type *s_ptr = store_at(store_num);
    inven_type *i_ptr = &s_ptr->store_inven[item_val].sitem;

    int number;

    // for single stackable objects, only destroy one half on average,
    // this will help ensure that general store and alchemist have
    // reasonable selection of objects
    if ((i_ptr->subval >= ITEM_SINGLE_STACK_MIN) &&
        (i_ptr->subval <= ITEM_SINGLE_STACK_MAX)) {
        if (one_of) {
            number = 1;
        } else {
            number = randint((int)i_ptr->number);
        }
    } else {
        number = i_ptr->number;
    }

    if (number != i_ptr->number) {
        i_ptr->number -= number;
    } else {
        for (int j = item_val; j < s_ptr->store_ctr - 1; j++) {
            s_ptr->store_inven[j] = s_ptr->store_inven[j + 1];
        }
        invcopy(&s_ptr->store_inven[s_ptr->store_ctr - 1].sitem, OBJ_NOTHING);
        s_ptr->store_inven[s_ptr->store_ctr - 1].scost = 0;
        s_ptr->store_ctr--;
    }
}

// Initializes the stores with owners -RAK-
void store_init(void) {
    int i = MAX_OWNERS / store_count();

    for (int j = 0; j < store_count(); j++) {
        store_type *s_ptr = store_at(j);

        s_ptr->owner      = store_count() * (randint(i) - 1) + j;
        s_ptr->insult_cur = 0;
        s_ptr->store_open = 0;
        s_ptr->store_ctr  = 0;
        s_ptr->good_buy   = 0;
        s_ptr->bad_buy    = 0;

        for (int k = 0; k < STORE_INVEN_MAX; k++) {
            invcopy(&s_ptr->store_inven[k].sitem, OBJ_NOTHING);
            s_ptr->store_inven[k].scost = 0;
        }
    }
}

// Creates an item and inserts it into store's inven -RAK-
static void store_create(int store_num) {
    int tries = 0;
    int cur_pos = popt();

    store_type *s_ptr = store_at(store_num);

    do {
        int i = store_choice[store_num][randint(STORE_CHOICES) - 1];
        invcopy(floor_item_at(cur_pos), i);
        magic_treasure(cur_pos, OBJ_TOWN_LEVEL);

        inven_type *t_ptr = floor_item_at(cur_pos);

        if (store_check_num(t_ptr, store_num)) {
            if ((t_ptr->cost > 0) && // Item must be good
                (t_ptr->cost < owners[s_ptr->owner].max_cost)) {
                // equivalent to calling ident_spell(),
                // except will not change the per-kind records.
                store_bought(t_ptr);

                int dummy;
                store_carry(store_num, &dummy, t_ptr);

                tries = 10;
            }
        }
        tries++;
    } while (tries <= 3);

    pusht((uint8_t)cur_pos);
}

// Initialize and up-keep the store's inventory. -RAK-
void store_maint(void) {
    for (int i = 0; i < store_count(); i++) {
        store_type *s_ptr = store_at(i);

        s_ptr->insult_cur = 0;
        if (s_ptr->store_ctr >= STORE_MIN_INVEN) {
            int j = randint(STORE_TURN_AROUND);
            if (s_ptr->store_ctr >= STORE_MAX_INVEN) {
                j += 1 + s_ptr->store_ctr - STORE_MAX_INVEN;
            }
            while (--j >= 0) {
                store_destroy(i, randint((int)s_ptr->store_ctr) - 1, false);
            }
        }

        if (s_ptr->store_ctr <= STORE_MAX_INVEN) {
            int j = randint(STORE_TURN_AROUND);
            if (s_ptr->store_ctr < STORE_MIN_INVEN) {
                j += STORE_MIN_INVEN - s_ptr->store_ctr;
            }
            while (--j >= 0) {
                store_create(i);
            }
        }
    }
}
