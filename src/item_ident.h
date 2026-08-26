// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// What the player learns by using a consumable item

#ifndef ITEM_IDENT_H
#define ITEM_IDENT_H

#include <stdbool.h>

// Declared here so this header does not depend on the whole of types.h.
// C11 onwards allows this redundant typedef alongside the one in types.h.
typedef struct inven_type inven_type;

// Called after a potion / food / scroll has taken effect.
//
// When `effect_identified` is true and the item was not known yet, the player
// gains experience for the discovery and the item becomes identified.
// Otherwise the item is only marked as "tried".
//
// `item_val` is updated in place because identify() may merge stacks and
// renumber the inventory. The returned pointer is the item's new location;
// callers that keep an `inven_type *` must reassign it from the return value,
// as the old pointer can be stale.
inven_type *learn_item_effect(bool effect_identified, int *item_val);

#endif // ITEM_IDENT_H
