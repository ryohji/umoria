// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Operations on the pack that need more than the inventory window: taking an
// item out, dropping it, stacking one in, the weight the player can carry,
// and finding where a kind of item sits
//
// Moved out of misc3.c unchanged (#42), in the order they had there; the
// prototypes of all but the static items_can_stack() stay in externs.h.
// inventory.c is left alone, since it is a state module that others link
// on its own.

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"
#include "burden.h"
#include "dungeon_map.h"
#include "equipment.h"
#include "floor_items.h"
#include "inventory.h"
#include "item_ident.h"
#include "player_body_weight.h"
#include "player_pos.h"
#include "player_status_flags.h"

// Destroy an item in the inventory -RAK-
void inven_destroy(int item_val) {
    inven_type *i_ptr = inventory_at(item_val);

    if ((i_ptr->number > 1) && (i_ptr->subval <= ITEM_SINGLE_STACK_MAX)) {
        i_ptr->number--;
        inventory_set_weight(inventory_weight() - i_ptr->weight);
    } else {
        inventory_set_weight(inventory_weight() - i_ptr->weight * i_ptr->number);
        for (int j = item_val; j < inventory_count() - 1; j++) {
            *inventory_at(j) = *inventory_at(j + 1);
        }
        invcopy(inventory_at(inventory_count() - 1), OBJ_NOTHING);
        inventory_set_count(inventory_count() - 1);
    }
    player_request_strength_check();
}

// Copies the object in the second argument over the first argument.
// However, the second always gets a number of one except for ammo etc.
void take_one_item(inven_type *s_ptr, inven_type *i_ptr) {
    *s_ptr = *i_ptr;
    if ((s_ptr->number > 1) && (s_ptr->subval >= ITEM_SINGLE_STACK_MIN) &&
        (s_ptr->subval <= ITEM_SINGLE_STACK_MAX)) {
        s_ptr->number = 1;
    }
}

// Drops an item from inventory to given location -RAK-
void inven_drop(int item_val, int drop_all) {
    if (square_at(player_row(), player_col())->tptr != 0) {
        (void)delete_object(player_row(), player_col());
    }

    int i = popt();
    inven_type *i_ptr = inventory_and_equipment_at(item_val);
    *floor_item_at(i) = *i_ptr;
    square_at(player_row(), player_col())->tptr = i;

    if (item_val >= INVEN_WIELD) {
        takeoff(item_val, -1);
    } else {
        if (drop_all || i_ptr->number == 1) {
            inventory_set_weight(inventory_weight() - i_ptr->weight * i_ptr->number);
            inventory_set_count(inventory_count() - 1);
            while (item_val < inventory_count()) {
                *inventory_at(item_val) = *inventory_at(item_val + 1);
                item_val++;
            }
            invcopy(inventory_at(inventory_count()), OBJ_NOTHING);
        } else {
            floor_item_at(i)->number = 1;
            inventory_set_weight(inventory_weight() - i_ptr->weight);
            i_ptr->number--;
        }

        bigvtype prt1;
        msgtype prt2;
        objdes(prt1, floor_item_at(i), true);
        (void)snprintf(prt2, sizeof(prt2), "Dropped %s", prt1);
        msg_print(prt2);
    }
    player_request_strength_check();
}

// Destroys a type of item on a given percent chance -RAK-
int inven_damage(bool (*typ)(inven_type *), int perc) {
    int j = 0;

    for (int i = 0; i < inventory_count(); i++) {
        if ((*typ)(inventory_at(i)) && (randint(100) < perc)) {
            inven_destroy(i);
            j++;
        }
    }

    return j;
}

// Computes current weight limit -RAK-
int weight_limit(void) {
    // 体の重さは窓口ごしに（#18-12-23B）。**この式の主語は腕力**で、体重は
    // 下駄のほう —— だから上限の計算は module の外に残した（use_stat[] に
    // まだ窓口が無いので、畳むとしても `struct player_stat` のあと）。
    int weight_cap = py.stats.use_stat[A_STR] * PLAYER_WEIGHT_CAP + player_body_weight();

    if (weight_cap > 3000) {
        weight_cap = 3000;
    }

    return weight_cap;
}

// Can the incoming item be merged into the existing inventory item?
// Shared by inven_check_num() and inven_carry(), which must agree:
// if only one of them accepted a stack, items would be lost.
static bool items_can_stack(inven_type *existing, inven_type *incoming) {
    return (existing->tval == incoming->tval) &&
           (existing->subval == incoming->subval) &&
           (incoming->subval >= ITEM_SINGLE_STACK_MIN) &&
           // make sure the number field doesn't overflow
           ((int)existing->number + (int)incoming->number < 256) &&
           // they always stack (subval < 192), or else they have same p1
           ((incoming->subval < ITEM_GROUP_MIN) ||
            (existing->p1 == incoming->p1)) &&
           // only stack if both or neither are identified
           (known1_p(existing) == known1_p(incoming));
}

bool inven_check_num(inven_type *t_ptr) {
    if (inventory_count() < inventory_slot_count()) {
        return true;
    }
    for (int i = 0; i < inventory_count(); i++) {
        if (items_can_stack(inventory_at(i), t_ptr)) {
            return true;
        }
    }
    return false;
}

// return false if picking up an object would change the players speed
bool inven_check_weight(inven_type *i_ptr) {
    int i = weight_limit();
    int new_inven_weight = i_ptr->number * i_ptr->weight + inventory_weight();

    if (i < new_inven_weight) {
        i = new_inven_weight / (i + 1);
    } else {
        i = 0;
    }

    if (pack_speed_penalty() != i) {
        return false;
    } else {
        return true;
    }
}

// Are we strong enough for the current pack and weapon? -CJS-
void check_strength(void) {
    inven_type *i_ptr = equipment_at(INVEN_WIELD);

    if (i_ptr->tval != TV_NOTHING &&
        (py.stats.use_stat[A_STR] * 15 < i_ptr->weight)) {
        if (!weapon_is_too_heavy()) {
            msg_print("You have trouble wielding such a heavy weapon.");
            set_weapon_too_heavy(true);
            calc_bonuses();
        }
    } else if (weapon_is_too_heavy()) {
        set_weapon_too_heavy(false);
        if (i_ptr->tval != TV_NOTHING) {
            msg_print("You are strong enough to wield your weapon.");
        }
        calc_bonuses();
    }

    int i = weight_limit();
    if (i < inventory_weight()) {
        i = inventory_weight() / (i + 1);
    } else {
        i = 0;
    }

    // The remembered penalty is read four times below (compare, direction of the
    // message, and the difference handed to change_speed), and nothing in
    // between writes it, so read it once.
    int remembered = pack_speed_penalty();
    if (remembered != i) {
        if (remembered < i) {
            msg_print("Your pack is so heavy that it slows you down.");
        } else {
            msg_print("You move more easily under the weight of your pack.");
        }
        change_speed(i - remembered);
        set_pack_speed_penalty(i);
    }
    player_clear_strength_check_request();
}

// Add an item to players inventory.  Return the
// item position for a description if needed. -RAK-
// the stacking condition is shared with inven_check_num() via items_can_stack()
int inven_carry(inven_type *i_ptr) {
    int typ = i_ptr->tval;
    int subt = i_ptr->subval;
    // Kinds with no record of their own are always known by name, so they can
    // be carried in order; see item_ident.h.
    int always_known1p = !item_kind_has_record(i_ptr);

    int locn;

    // Now, check to see if player can carry object
    for (locn = 0;; locn++) {
        inven_type *t_ptr = inventory_at(locn);

        if (items_can_stack(t_ptr, i_ptr)) {
            t_ptr->number += i_ptr->number;
            break;
        } else if ((typ == t_ptr->tval && subt < t_ptr->subval && always_known1p) || (typ > t_ptr->tval)) {
            // For items which are always known1p, i.e. never have a 'color',
            // insert them into the inventory in sorted order.

            for (int i = inventory_count() - 1; i >= locn; i--) {
                *inventory_at(i + 1) = *inventory_at(i);
            }
            *inventory_at(locn) = *i_ptr;
            inventory_set_count(inventory_count() + 1);
            break;
        }
    }

    inventory_set_weight(inventory_weight() + i_ptr->number * i_ptr->weight);
    player_request_strength_check();

    return locn;
}

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
