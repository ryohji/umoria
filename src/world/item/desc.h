// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#ifndef DESC_H
#define DESC_H

void magic_init(void);
void known1(inven_type *);
bool known1_p(inven_type *);
void known2(inven_type *);
bool known2_p(inven_type *);
void clear_known2(inven_type *);
void clear_empty(inven_type *);
void store_bought(inven_type *);
bool store_bought_p(inven_type *);
void sample(inven_type *);
void identify(int *);
void unmagic_name(inven_type *);
void objdes(char *, inven_type *, int);
void desc_charges(int);
void desc_remain(int);

#endif // DESC_H
