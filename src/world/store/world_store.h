// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Public interface of world/store/.

#ifndef WORLD_STORE_H
#define WORLD_STORE_H

#include "config.h"
#include "constant.h"
#include "types.h"

// --- store_haggle.c ---
// Haggling with a store owner, as the store screen (store_ui.c) calls it
// inven_type comes from types.h, which has to be included before this header.

// Haggle result codes
enum {
    HAGGLE_AGREED = 0,    // Deal accepted
    HAGGLE_CANCELLED = 1, // Player cancelled
    HAGGLE_INSULTED = 2,  // Too many insults, kicked out
    HAGGLE_WORTHLESS = 3  // Item has no value (sell only)
};
bool increase_insults(int store_num);
void decrease_insults(int store_num);
int purchase_haggle(int store_num, int32_t *price, inven_type *item);
int sell_haggle(int store_num, int32_t *price, inven_type *item);

// --- store_price.c ---
int32_t item_value(inven_type *);

// --- store_stock.c ---
bool store_check_num(inven_type *, int);
void store_carry(int, int *, inven_type *);
void store_destroy(int, int, int);
void store_init(void);
void store_maint(void);

#endif // WORLD_STORE_H
