// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The display hints and the notes kept in an item's ident byte

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "item_flags.h"

// Display hint setters
void item_show_hit_dam(inven_type *i_ptr) {
    i_ptr->ident |= ID_SHOW_HITDAM;
}

void item_show_p1(inven_type *i_ptr) {
    i_ptr->ident |= ID_SHOW_P1;
}

void item_hide_p1(inven_type *i_ptr) {
    i_ptr->ident |= ID_NOSHOW_P1;
}

// Display hint readers
bool item_shows_hit_dam(const inven_type *i_ptr) {
    return (i_ptr->ident & ID_SHOW_HITDAM) != 0;
}

bool item_shows_p1(const inven_type *i_ptr) {
    return (i_ptr->ident & ID_SHOW_P1) != 0;
}

bool item_hides_p1(const inven_type *i_ptr) {
    return (i_ptr->ident & ID_NOSHOW_P1) != 0;
}

// Note readers
bool item_noted_magical(const inven_type *i_ptr) {
    return (i_ptr->ident & ID_MAGIK) != 0;
}

bool item_noted_empty(const inven_type *i_ptr) {
    return (i_ptr->ident & ID_EMPTY) != 0;
}

bool item_noted_damned(const inven_type *i_ptr) {
    return (i_ptr->ident & ID_DAMD) != 0;
}

bool item_has_any_note(const inven_type *i_ptr) {
    return (i_ptr->ident & (ID_MAGIK | ID_EMPTY | ID_DAMD)) != 0;
}
