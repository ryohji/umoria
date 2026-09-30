// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// What the player can see: lighting rooms and squares, and moving the light
// with the player

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

#include "dungeon_map.h"
#include "floor_items.h"
#include "panel.h"
#include "player_light.h"
#include "player_pos.h"
#include "player_timed_effects.h"
#include "running.h"

// Returns true if player has no light -RAK-
bool no_light(void) {
    cave_type *c_ptr = square_at(player_row(), player_col());

    if (!c_ptr->tl && !c_ptr->pl) {
        return true;
    }
    return false;
}

// Moves creature record from one space to another -RAK-
void move_rec(int y1, int x1, int y2, int x2) {
    // this always works correctly, even if y1==y2 and x1==x2
    int tmp = square_at(y1, x1)->cptr;
    square_at(y1, x1)->cptr = 0;
    square_at(y2, x2)->cptr = tmp;
}

// Room is lit, make it appear -RAK-
void light_room(int y, int x) {
    int tmp1 = (SCREEN_HEIGHT / 2);
    int tmp2 = (SCREEN_WIDTH / 2);

    int start_row = (y / tmp1) * tmp1;
    int start_col = (x / tmp2) * tmp2;

    int end_row = start_row + tmp1 - 1;
    int end_col = start_col + tmp2 - 1;

    for (int i = start_row; i <= end_row; i++) {
        for (int j = start_col; j <= end_col; j++) {
            cave_type *c_ptr = square_at(i, j);

            if (c_ptr->lr && !c_ptr->pl) {
                c_ptr->pl = true;
                if (c_ptr->fval == DARK_FLOOR) {
                    c_ptr->fval = LIGHT_FLOOR;
                }
                if (!c_ptr->fm && c_ptr->tptr != 0) {
                    int tval = floor_item_at(c_ptr->tptr)->tval;
                    if (tval >= TV_MIN_VISIBLE && tval <= TV_MAX_VISIBLE) {
                        c_ptr->fm = true;
                    }
                }
                print(loc_symbol(i, j), i, j);
            }
        }
    }
}

// Lights up given location -RAK-
void lite_spot(int y, int x) {
    if (panel_contains(y, x)) {
        print(loc_symbol(y, x), y, x);
    }
}

// Normal movement
// When FIND_FLAG,  light only permanent features
static void sub1_move_light(int y1, int x1, int y2, int x2) {
    if (player_light_is_drawn()) {
        // Turn off lamp light
        for (int i = y1 - 1; i <= y1 + 1; i++) {
            for (int j = x1 - 1; j <= x1 + 1; j++) {
                square_at(i, j)->tl = false;
            }
        }
        if (player_is_running() && !find_prself) {
            set_player_light_drawn(false);
        }
    } else if (!player_is_running() || find_prself) {
        set_player_light_drawn(true);
    }

    for (int i = y2 - 1; i <= y2 + 1; i++) {
        for (int j = x2 - 1; j <= x2 + 1; j++) {
            cave_type *c_ptr = square_at(i, j);

            // only light up if normal movement
            if (player_light_is_drawn()) {
                c_ptr->tl = true;
            }
            if (c_ptr->fval >= MIN_CAVE_WALL) {
                c_ptr->pl = true;
            } else if (!c_ptr->fm && c_ptr->tptr != 0) {
                int tval = floor_item_at(c_ptr->tptr)->tval;
                if ((tval >= TV_MIN_VISIBLE) && (tval <= TV_MAX_VISIBLE)) {
                    c_ptr->fm = true;
                }
            }
        }
    }

    int top, left, bottom, right;

    // From uppermost to bottom most lines player was on.
    if (y1 < y2) {
        top = y1 - 1;
        bottom = y2 + 1;
    } else {
        top = y2 - 1;
        bottom = y1 + 1;
    }
    if (x1 < x2) {
        left = x1 - 1;
        right = x2 + 1;
    } else {
        left = x2 - 1;
        right = x1 + 1;
    }
    for (int i = top; i <= bottom; i++) {
        // Leftmost to rightmost do
        for (int j = left; j <= right; j++) {
            print(loc_symbol(i, j), i, j);
        }
    }
}

// When blinded,  move only the player symbol.
// With no light,  movement becomes involved.
static void sub3_move_light(int y1, int x1, int y2, int x2) {
    if (player_light_is_drawn()) {
        for (int i = y1 - 1; i <= y1 + 1; i++) {
            for (int j = x1 - 1; j <= x1 + 1; j++) {
                square_at(i, j)->tl = false;
                print(loc_symbol(i, j), i, j);
            }
        }
        set_player_light_drawn(false);
    } else if (!player_is_running() || find_prself) {
        print(loc_symbol(y1, x1), y1, x1);
    }

    if (!player_is_running() || find_prself) {
        print('@', y2, x2);
    }
}

// Package for moving the character's light about the screen
// Four cases : Normal, Finding, Blind, and Nolight -RAK-
void move_light(int y1, int x1, int y2, int x2) {
    if (player_timed_in_force(PLAYER_TIMED_BLINDNESS) || !player_has_light()) {
        sub3_move_light(y1, x1, y2, x2);
    } else {
        sub1_move_light(y1, x1, y2, x2);
    }
}
