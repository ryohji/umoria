// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Store prices: what an item is worth, what a store asks for it, and the
// record of how well the player has haggled there

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"
#include "item_flags.h"
#include "player_race.h"
#include "stores.h"

// None of these prints or asks for input. The callers are the stock
// (store_stock.c), the haggling (store_haggle.c) and the score of the dead
// (death.c).

// Returns the value for any given object -RAK-
int32_t item_value(inven_type *i_ptr) {
    int32_t value = i_ptr->cost;

    // don't purchase known cursed items
    if (item_noted_damned(i_ptr)) {
        value = 0;
    } else if (((i_ptr->tval >= TV_BOW) && (i_ptr->tval <= TV_SWORD)) || ((i_ptr->tval >= TV_BOOTS) && (i_ptr->tval <= TV_SOFT_ARMOR))) {
        // Weapons and armor

        if (!known2_p(i_ptr)) {
            value = object_list[i_ptr->index].cost;
        } else if ((i_ptr->tval >= TV_BOW) && (i_ptr->tval <= TV_SWORD)) {
            if (i_ptr->tohit < 0) {
                value = 0;
            } else if (i_ptr->todam < 0) {
                value = 0;
            } else if (i_ptr->toac < 0) {
                value = 0;
            } else {
                value = i_ptr->cost + (i_ptr->tohit + i_ptr->todam + i_ptr->toac) * 100;
            }
        } else {
            if (i_ptr->toac < 0) {
                value = 0;
            } else {
                value = i_ptr->cost + i_ptr->toac * 100;
            }
        }
    } else if ((i_ptr->tval >= TV_SLING_AMMO) && (i_ptr->tval <= TV_SPIKE)) {
        // Ammo

        if (!known2_p(i_ptr)) {
            value = object_list[i_ptr->index].cost;
        } else {
            if (i_ptr->tohit < 0) {
                value = 0;
            } else if (i_ptr->todam < 0) {
                value = 0;
            } else if (i_ptr->toac < 0) {
                value = 0;
            } else {
                // use 5, because missiles generally appear in groups of 20,
                // so 20 * 5 == 100, which is comparable to weapon bonus above
                value = i_ptr->cost + (i_ptr->tohit + i_ptr->todam + i_ptr->toac) * 5;
            }
        }
    } else if ((i_ptr->tval == TV_SCROLL1) || (i_ptr->tval == TV_SCROLL2) || (i_ptr->tval == TV_POTION1) || (i_ptr->tval == TV_POTION2)) {
        // Potions, Scrolls, and Food

        if (!known1_p(i_ptr)) {
            value = 20;
        }
    } else if (i_ptr->tval == TV_FOOD) {
        if ((i_ptr->subval < (ITEM_SINGLE_STACK_MIN + MAX_MUSH)) &&
            !known1_p(i_ptr)) {
            value = 1;
        }
    } else if ((i_ptr->tval == TV_AMULET) || (i_ptr->tval == TV_RING)) {
        // Rings and amulets

        // player does not know what type of ring/amulet this is
        if (!known1_p(i_ptr)) {
            value = 45;
        } else if (!known2_p(i_ptr)) {
            // player knows what type of ring, but does not know whether it
            // is cursed or not, if refuse to buy cursed objects here, then
            // player can use this to 'identify' cursed objects
            value = object_list[i_ptr->index].cost;
        }
    } else if ((i_ptr->tval == TV_STAFF) || (i_ptr->tval == TV_WAND)) {
        // Wands and staffs

        if (!known1_p(i_ptr)) {
            if (i_ptr->tval == TV_WAND) {
                value = 50;
            } else {
                value = 70;
            }
        } else if (known2_p(i_ptr)) {
            value = i_ptr->cost + (i_ptr->cost / 20) * i_ptr->p1;
        }
    } else if (i_ptr->tval == TV_DIGGING) {
        // Picks and shovels

        if (!known2_p(i_ptr)) {
            value = object_list[i_ptr->index].cost;
        } else {
            if (i_ptr->p1 < 0) {
                value = 0;
            } else {
                // some digging tools start with non-zero p1 values, so only
                // multiply the plusses by 100, make sure result is positive
                value = i_ptr->cost + (i_ptr->p1 - object_list[i_ptr->index].p1) * 100;
                if (value < 0) {
                    value = 0;
                }
            }
        }
    }

    // Multiply value by number of items if it is a group stack item.
    // Do not include torches here.
    if (i_ptr->subval > ITEM_GROUP_MIN) {
        value = value * i_ptr->number;
    }

    return value;
}

// Asking price for an item -RAK-
int32_t sell_price(int snum, int32_t *max_sell, int32_t *min_sell, inven_type *item) {
    store_type *s_ptr = store_at(snum);

    int32_t i = item_value(item);

    // check item->cost in case it is cursed, check i in case it is damaged
    if ((item->cost > 0) && (i > 0)) {
        // Adjust by race pairing table (owner's race vs player's race).
        i = i * rgold_adj[owners[s_ptr->owner].owner_race][player_race()] / 100;
        if (i < 1) {
            i = 1;
        }
        *max_sell = i * owners[s_ptr->owner].max_inflate / 100;
        *min_sell = i * owners[s_ptr->owner].min_inflate / 100;
        if (*min_sell > *max_sell) {
            *min_sell = *max_sell;
        }

        return i;
    } else {
        // don't let the item get into the store inventory
        return 0;
    }
}

// eliminate need to bargain if player has haggled well in the past -DJB-
bool noneedtobargain(int store_num, int32_t minprice) {
    store_type *s_ptr = store_at(store_num);

    if (s_ptr->good_buy == MAX_SHORT) {
        return true;
    }
    int bargain_record = (s_ptr->good_buy - 3 * s_ptr->bad_buy - 5);

    return ((bargain_record > 0) && ((int32_t)bargain_record * (int32_t)bargain_record > minprice / 50));
}

// update the bargin info -DJB-
void updatebargain(int store_num, int32_t price, int32_t minprice) {
    store_type *s_ptr = store_at(store_num);

    if (minprice > 9) {
        if (price == minprice) {
            if (s_ptr->good_buy < MAX_SHORT) {
                s_ptr->good_buy++;
            }
        } else {
            if (s_ptr->bad_buy < MAX_SHORT) {
                s_ptr->bad_buy++;
            }
        }
    }
}
