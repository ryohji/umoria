// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#ifndef STORE_STOCK_H
#define STORE_STOCK_H

bool store_check_num(inven_type *, int);
void store_carry(int, int *, inven_type *);
void store_destroy(int, int, int);
void store_init(void);
void store_maint(void);

#endif // STORE_STOCK_H
