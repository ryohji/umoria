// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#ifndef SETS_H
#define SETS_H

bool set_room(int);
bool set_corr(int);
bool set_floor(int);
bool set_corrodes(inven_type *);
bool set_flammable(inven_type *);
bool set_frost_destroy(inven_type *);
bool set_acid_affect(inven_type *);
bool set_lightning_destroy(inven_type *);
bool set_null(inven_type *);
bool set_acid_destroy(inven_type *);
bool set_fire_destroy(inven_type *);
bool set_large(treasure_type *);
bool general_store(int);
bool armory(int);
bool weaponsmith(int);
bool temple(int);
bool alchemist(int);
bool magic_shop(int);

#endif // SETS_H
