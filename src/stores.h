// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The town's shops: one record each, holding the owner, the shelves, and how
// well the player has haggled there.

#ifndef STORES_H
#define STORES_H

// store_type comes from types.h, which has to be included before this header.

// There is nothing to derive here -- unlike the panel or the message history,
// this is storage and only storage. What it buys is a single owner: four files
// used to name the array, and store1.c, store2.c and save.c each began by
// turning an index into a pointer. Now only this module knows where the
// records are.

// The number of shops in the town, and the number of records below. Doors 1..6
// on the town map lead to index 0..5, which is also how store_choice[] (what a
// shop stocks) and store_buy[] (what it will buy) are indexed.
int store_count(void);

// The record for one shop. `index` is 0 .. store_count() - 1.
//
// The pointer stays valid for the rest of the run and always addresses the
// same record: callers keep it in a local (`store_type *s_ptr`) across other
// calls, and the save file reads straight into it.
store_type *store_at(int index);

#endif // STORES_H
