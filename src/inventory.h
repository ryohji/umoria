// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// What the player carries: the pack, how many slots of it are used, and how
// heavy it is.

#ifndef INVENTORY_H
#define INVENTORY_H

// inven_type comes from types.h, which has to be included before this header.

// One array, two windows. The slots 0 .. 21 are the pack and the slots
// 22 .. 33 are what the player wears or wields; equipment.h is the window on
// the second half. Both windows use the same index space, so an index means
// the same thing whichever window it is handed to -- the split is in the
// names and in the intent, not in the arithmetic.
//
// This is storage and only storage, so the windows are thin: they do not
// range-check, clamp or assert. The old code did not check either, and
// checking here would change behaviour rather than preserve it.

// A slot of the pack. `index` is 0 .. inventory_slot_count() - 1.
//
// The pointer stays valid for the rest of the run and always addresses the
// same slot: callers keep it in a local (`inven_type *i_ptr`) across other
// calls, and the save file reads straight into it.
inven_type *inventory_at(int index);

// How many slots the pack has (22). It is also the value the code compares
// inventory_count() against to decide whether the pack is full, and the index
// of the first equipment slot.
int inventory_slot_count(void);

// How many slots of the pack hold something. The pack is kept packed, so the
// used slots are 0 .. inventory_count() - 1.
int inventory_count(void);
void inventory_set_count(int count);

// The weight of everything in the pack, in the same unit as
// inven_type.weight. Callers keep it up to date themselves as they add and
// remove objects; nothing here recomputes it.
int inventory_weight(void);
void inventory_set_weight(int weight);

// --- crossing window -----------------------------------------------------

// A window for the places that walk both the pack and the equipment by index.
// If the number of calls here goes down, the split is paying off.
inven_type *inventory_and_equipment_at(int index);
int inventory_and_equipment_slot_count(void);

#endif // INVENTORY_H
