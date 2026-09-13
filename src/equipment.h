// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// What the player wears and wields: one slot per place on the body, and how
// many of them are filled.

#ifndef EQUIPMENT_H
#define EQUIPMENT_H

// inven_type comes from types.h, which has to be included before this header.

// The equipment shares its storage and its index space with the pack (see
// inventory.h): the slots are 22 .. 33, named by the INVEN_* constants in
// constant.h. An index is not translated on the way through this window, so
// the arithmetic at the call sites stays as it was.
//
// This is storage and only storage, so the windows are thin: they do not
// range-check, clamp or assert.

// The slot for one place on the body. `index` is one of the INVEN_* constants,
// that is equipment_first_slot() .. equipment_end_slot() - 1.
//
// The pointer stays valid for the rest of the run and always addresses the
// same slot.
inven_type *equipment_at(int index);

// The range of slots, as a half-open interval, so that a loop over the
// equipment reads `for (i = equipment_first_slot(); i < equipment_end_slot();
// i++)` -- the shape the code already uses.
//
// There is no constant here for the number of slots, because the one place
// that needs the count needs it at compile time (moria1.c:356 sizes a local
// array with INVEN_ARRAY_SIZE - INVEN_WIELD) and a function cannot serve that.
int equipment_first_slot(void);
int equipment_end_slot(void);

// How many equipment slots are filled. Unlike the pack, the filled slots are
// not packed together -- the count is a total, not a bound to loop up to.
int equipment_count(void);
void equipment_set_count(int count);

#endif // EQUIPMENT_H
