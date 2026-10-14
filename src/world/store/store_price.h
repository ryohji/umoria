// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#ifndef STORE_PRICE_H
#define STORE_PRICE_H

int32_t item_value(inven_type *);
int32_t sell_price(int, int32_t *, int32_t *, inven_type *);
bool noneedtobargain(int, int32_t);
void updatebargain(int, int32_t, int32_t);

#endif // STORE_PRICE_H
