// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The map as the player sees it: which part of the dungeon is on the screen,
// and what is lit around the player.

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "dungeon_map.h"
#include "externs.h"
#include "floor_items.h"
#include "monster_list.h"
#include "panel.h"
#include "player_pos.h"
#include "player_status_flags.h"
#include "player_timed_effects.h"
#include "running.h"

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

// Moves the panel if the player has walked off the edge of it, and redraws
// nothing: the caller does that when we return true. Force forces the panel
// bounds to be recalculated, useful for 'W'here.
//
// The move itself is panel.c's; what is left here is the one thing it must not
// do, which is to reach back into the movement code.
int get_panel(int y, int x, int force) {
    if (!panel_move_to(y, x, force != 0)) {
        return false;
    }

    // stop movement if any
    if (find_bound) {
        end_find();
    }
    return true;
}

// Returns symbol for given row, column -RAK-
uint8_t loc_symbol(int y, int x) {
    cave_type *cave_ptr = square_at(y, x);

    if ((cave_ptr->cptr == 1) && (!player_is_running() || find_prself)) {
        return '@';
    } else if (player_effect_in_force(PLAYER_EFFECT_BLIND)) {
        return ' ';
    } else if (player_timed_in_force(PLAYER_TIMED_HALLUCINATION) && (randint(12) == 1)) {
        return randint(95) + 31;
    } else if ((cave_ptr->cptr > 1) && (monster_list_at(cave_ptr->cptr)->ml)) {
        return monster_get_creature(monster_list_at(cave_ptr->cptr)->creature)->cchar;
    } else if (!cave_ptr->pl && !cave_ptr->tl && !cave_ptr->fm) {
        return ' ';
    } else if ((cave_ptr->tptr != 0) && (floor_item_at(cave_ptr->tptr)->tval != TV_INVIS_TRAP)) {
        return floor_item_at(cave_ptr->tptr)->tchar;
    } else if (cave_ptr->fval <= MAX_CAVE_FLOOR) {
        return '.';
    } else if (cave_ptr->fval == GRANITE_WALL || cave_ptr->fval == BOUNDARY_WALL || highlight_seams == false) {
        return '#';
    } else {
        // Originally set highlight bit, but that is not portable,
        // now use the percent sign instead.
        return '%';
    }
}

// Tests a spot for light or field mark status -RAK-
bool test_light(int y, int x) {
    cave_type *cave_ptr = square_at(y, x);
    if (cave_ptr->pl || cave_ptr->tl || cave_ptr->fm) {
        return true;
    } else {
        return false;
    }
}

// Prints the map of the dungeon -RAK-
void prt_map(void) {
    // Top to bottom. The screen row is the same walk print() makes when it
    // converts a dungeon row into a screen row, so ask for that.
    for (int i = panel_top_row(); i <= panel_bottom_row(); i++) {
        erase_line(panel_screen_row(i), PANEL_MAP_LEFT_COL);

        // Left to right
        for (int j = panel_left_col(); j <= panel_right_col(); j++) {
            uint8_t tmp = loc_symbol(i, j);
            if (tmp != ' ') {
                print(tmp, i, j);
            }
        }
    }
}
