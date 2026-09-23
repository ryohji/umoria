// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Misc code for maintaining the dungeon, printing player info

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "equipment.h"
#include "externs.h"
#include "inventory.h"
#include "player_pos.h"

#include <stdarg.h>

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

// We need to reset the view of things. -CJS-
void check_view(void) {
    cave_type *c_ptr = &cave[player_row()][player_col()];

    // Check for new panel
    if (get_panel(player_row(), player_col(), false)) {
        prt_map();
    }

    // Move the light source
    move_light(player_row(), player_col(), player_row(), player_col());

    if (c_ptr->fval == LIGHT_FLOOR) {
        // A room of light should be lit.

        if ((py.flags.blind < 1) && !c_ptr->pl) {
            light_room(player_row(), player_col());
        }
    } else if (c_ptr->lr && (py.flags.blind < 1)) {
        // In doorway of light-room?

        for (int i = (player_row() - 1); i <= (player_row() + 1); i++) {
            for (int j = (player_col() - 1); j <= (player_col() + 1); j++) {
                cave_type *d_ptr = &cave[i][j];
                if ((d_ptr->fval == LIGHT_FLOOR) && !d_ptr->pl) {
                    light_room(i, j);
                }
            }
        }
    }
}

// concatenate var length string arguments (last should be NULL) into buffer.
// returns buffer.
char *concat(char *const buffer, ...) {
    char *p = buffer;
    const char *s;
    va_list list;

    va_start(list, buffer);
    buffer[0] = '\0';
    while ((s = va_arg(list, const char *))) {
        p = strcpy(p, s) + strlen(s);
    }
    va_end(list);

    return buffer;
}
