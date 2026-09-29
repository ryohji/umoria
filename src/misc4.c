// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Misc code for maintaining the dungeon, printing player info

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "dungeon_map.h"
#include "externs.h"
#include "player_pos.h"
#include "player_timed_effects.h"

#include <stdarg.h>

// We need to reset the view of things. -CJS-
void check_view(void) {
    cave_type *c_ptr = square_at(player_row(), player_col());

    // Check for new panel
    if (get_panel(player_row(), player_col(), false)) {
        prt_map();
    }

    // Move the light source
    move_light(player_row(), player_col(), player_row(), player_col());

    if (c_ptr->fval == LIGHT_FLOOR) {
        // A room of light should be lit.

        if (!player_timed_in_force(PLAYER_TIMED_BLINDNESS) && !c_ptr->pl) {
            light_room(player_row(), player_col());
        }
    } else if (c_ptr->lr && !player_timed_in_force(PLAYER_TIMED_BLINDNESS)) {
        // In doorway of light-room?

        for (int i = (player_row() - 1); i <= (player_row() + 1); i++) {
            for (int j = (player_col() - 1); j <= (player_col() + 1); j++) {
                cave_type *d_ptr = square_at(i, j);
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
