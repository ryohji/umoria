// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The display hints and the notes kept in an item's ident byte. The knowledge
// bits of the same byte (ID_KNOWN2, ID_STOREBOUGHT) stay with desc.c, and the
// notes are written by add_inscribe() (inscription.c).

#ifndef ITEM_FLAGS_H
#define ITEM_FLAGS_H

// No includes: the callers bring config.h, constant.h and types.h in first.

// Display hint setters
void item_show_hit_dam(inven_type *i_ptr);
void item_show_p1(inven_type *i_ptr);
void item_hide_p1(inven_type *i_ptr);

// Display hint readers
bool item_shows_hit_dam(const inven_type *i_ptr);
bool item_shows_p1(const inven_type *i_ptr);
bool item_hides_p1(const inven_type *i_ptr);

// Note readers
bool item_noted_magical(const inven_type *i_ptr);
bool item_noted_empty(const inven_type *i_ptr);
bool item_noted_damned(const inven_type *i_ptr);
bool item_has_any_note(const inven_type *i_ptr);

#endif // ITEM_FLAGS_H
