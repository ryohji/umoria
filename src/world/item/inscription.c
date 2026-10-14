// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Inscriptions: the comment a player writes on an object, and the marks the
// game adds to its description.

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "equipment.h"
#include "externs.h"
#include "inventory.h"

// The three prototypes are in externs.h, the same as the other files that
// include it (desc.c, eat.c): scribe_object() asks for the item through
// get_item() and prints through io.c, so this file needs externs.h anyway.

// Add a comment to an object description. -CJS-
void scribe_object(void) {
    if (inventory_count() > 0 || equipment_count() > 0) {
        int item_val;

        if (get_item(&item_val, "Which one? ", 0, inventory_and_equipment_slot_count(), CNIL, CNIL)) {
            msgtype out_val;
            bigvtype tmp_str;

            objdes(tmp_str, inventory_and_equipment_at(item_val), true);
            (void)snprintf(out_val, sizeof(out_val), "Inscribing %s", tmp_str);
            msg_print(out_val);
            if (inventory_and_equipment_at(item_val)->inscrip[0] != '\0') {
                (void)sprintf(out_val, "Replace %s New inscription:",
                              inventory_and_equipment_at(item_val)->inscrip);
            } else {
                (void)strcpy(out_val, "Inscription: ");
            }
            int j = 78 - (int)strlen(tmp_str);
            if (j > 12) {
                j = 12;
            }
            prt(out_val, 0, 0);
            if (get_string(out_val, 0, (int)strlen(out_val), j)) {
                inscribe(inventory_and_equipment_at(item_val), out_val);
            }
        }
    } else {
        msg_print("You are not carrying anything to inscribe.");
    }
}

// Append an additional comment to an object description. -CJS-
void add_inscribe(inven_type *i_ptr, uint8_t type) {
    i_ptr->ident |= type;
}

// Replace any existing comment in an object description with a new one. -CJS-
void inscribe(inven_type *i_ptr, const char *str) {
    (void)strcpy(i_ptr->inscrip, str);
}
