// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The panel: the part of the dungeon the screen is showing

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"
#include "panel.h"

// The ten variables are still the globals in variable.c: the callers this
// module does not cover yet reach them directly, so both sides have to see the
// same state. They move in here once nobody else names them.

int panel_row_index(void) {
    return panel_row;
}

int panel_col_index(void) {
    return panel_col;
}

// Calculates current boundaries -RAK-
static void recalculate_bounds(void) {
    panel_row_min = panel_row * (SCREEN_HEIGHT / 2);
    panel_row_max = panel_row_min + SCREEN_HEIGHT - 1;
    panel_row_prt = panel_row_min - PANEL_MAP_TOP_ROW;
    panel_col_min = panel_col * (SCREEN_WIDTH / 2);
    panel_col_max = panel_col_min + SCREEN_WIDTH - 1;
    panel_col_prt = panel_col_min - PANEL_MAP_LEFT_COL;
}

// Given an row (y) and col (x), this routine detects -RAK-
// when a move off the screen has occurred and figures new borders.
bool panel_move_to(int y, int x, bool force) {
    int prow = panel_row;
    int pcol = panel_col;

    if (force || (y < panel_row_min + 2) || (y > panel_row_max - 2)) {
        prow = ((y - SCREEN_HEIGHT / 4) / (SCREEN_HEIGHT / 2));
        if (prow > max_panel_rows) {
            prow = max_panel_rows;
        } else if (prow < 0) {
            prow = 0;
        }
    }
    if (force || (x < panel_col_min + 3) || (x > panel_col_max - 3)) {
        pcol = ((x - SCREEN_WIDTH / 4) / (SCREEN_WIDTH / 2));
        if (pcol > max_panel_cols) {
            pcol = max_panel_cols;
        } else if (pcol < 0) {
            pcol = 0;
        }
    }
    if ((prow == panel_row) && (pcol == panel_col)) {
        return false;
    }

    panel_row = prow;
    panel_col = pcol;
    recalculate_bounds();

    return true;
}

void panel_forget_position(void) {
    // Ensure we display the panel. Used to do this with a global var. -CJS-
    panel_row = panel_col = -1;
}

void panel_forget_bounds(void) {
    panel_row_min = 0;
    panel_row_max = 0;
    panel_col_min = 0;
    panel_col_max = 0;
}

int panel_top_row(void) {
    return panel_row_min;
}

int panel_bottom_row(void) {
    return panel_row_max;
}

int panel_left_col(void) {
    return panel_col_min;
}

int panel_right_col(void) {
    return panel_col_max;
}

// Tests a given point to see if it is within the screen -RAK-
// boundaries.
bool panel_contains(int y, int x) {
    return (y >= panel_row_min) && (y <= panel_row_max) && (x >= panel_col_min) && (x <= panel_col_max);
}

int panel_screen_row(int dungeon_row) {
    return dungeon_row - panel_row_prt;
}

int panel_screen_col(int dungeon_col) {
    return dungeon_col - panel_col_prt;
}

void panel_set_dungeon_size(int height, int width) {
    max_panel_rows = (int16_t)((height / SCREEN_HEIGHT) * 2 - 2);
    max_panel_cols = (int16_t)((width / SCREEN_WIDTH) * 2 - 2);
    panel_row = max_panel_rows;
    panel_col = max_panel_cols;
}

int panel_max_row_index(void) {
    return max_panel_rows;
}

int panel_max_col_index(void) {
    return max_panel_cols;
}

void panel_set_max_indexes(int max_row, int max_col) {
    max_panel_rows = (int16_t)max_row;
    max_panel_cols = (int16_t)max_col;
}
