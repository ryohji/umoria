// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Haggling with a store owner, as the store screen (store_ui.c) calls it

#ifndef STORE_HAGGLE_H
#define STORE_HAGGLE_H

// inven_type comes from types.h, which has to be included before this header.

bool increase_insults(int store_num);
void decrease_insults(int store_num);
int purchase_haggle(int store_num, int32_t *price, inven_type *item);
int sell_haggle(int store_num, int32_t *price, inven_type *item);

#endif
