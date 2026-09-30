// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// What the player has learned about each kind of object

#ifndef ITEM_IDENT_H
#define ITEM_IDENT_H

#include <stdbool.h>
#include <stdint.h>

// Declared here so this header does not depend on the whole of types.h.
// C11 onwards allows this redundant typedef alongside the one in types.h.
typedef struct inven_type inven_type;

// --- What the player has learned about each kind of object -------------------
//
// One record per kind, holding two marks: the kind is known by name, and the
// kind has been tried. The record is shared by every item of that kind, which
// is the point: drinking one unknown potion teaches the player about every
// potion of that sort, in the pack, on the floor or still in the dungeon.
//
// Which record a kind uses is not the caller's business. It used to be: the
// same three lines of index arithmetic appeared in five places in desc.c and a
// sixth in objdes(), and each of them had to remember that some kinds have no
// record at all. The windows below take the item and work it out.
//
// A kind with no record is one that is never secret -- weapons, armour, and
// the foods above the mushrooms. Asking about one answers "not known, not
// tried", and marking one does nothing; the callers that need "this kind is
// always known by name" ask item_kind_has_record().

// Does this kind have a record at all?
bool item_kind_has_record(inven_type *i_ptr);

bool item_kind_is_known(inven_type *i_ptr);
bool item_kind_was_tried(inven_type *i_ptr);

// Knowing the kind by name supersedes having tried it, so marking it known
// also clears the tried mark. The two always moved together at the one place
// that did this, spelled out as two lines against the same record.
void item_kind_mark_known(inven_type *i_ptr);

void item_kind_mark_tried(inven_type *i_ptr);
void item_kind_clear_tried(inven_type *i_ptr);

// For the save file, which stores the records as a fixed run of bytes.
uint8_t *item_kind_record_bytes(void);
int item_kind_record_count(void);

#endif // ITEM_IDENT_H
