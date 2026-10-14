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
#include "dungeon_state.h"
#include "monster_state.h"
#include "game_settings.h"
#include "lighting.h"
#include "map_view.h"
#include "monsters.h"
#include "rnd.h"
#include "run_path.h"
#include "ui_io.h"
#include "panel.h"
#include "render.h"
#include "player_condition.h"

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

// definitions used by screen_map()
// index into border character array
#define TL 0 // top left
#define TR 1
#define BL 2
#define BR 3
#define HE 4 // horizontal edge
#define VE 5

// character set to use
#define CH(x) (screen_border[0][x])

// Display highest priority object in the RATIO by RATIO area
#define RATIO 3

void screen_map(void) {
    static uint8_t screen_border[2][6] = {
        {'+', '+', '+', '+', '-', '|'}, // normal chars
        {201, 187, 200, 188, 205, 186}, // graphics chars
    };

    uint8_t map[MAX_WIDTH / RATIO + 1];
    int priority[256];

    char prntscrnbuf[80];

    for (int i = 0; i < 256; i++) {
        priority[i] = 0;
    }

    priority['<'] = 5;
    priority['>'] = 5;
    priority['@'] = 10;
    priority['#'] = -5;
    priority['.'] = -10;
    priority['\''] = -3;
    priority[' '] = -15;

    save_screen();
    clear_screen();
    render_char(0, 0, CH(TL), RENDER_COLOR_DEFAULT);

    for (int i = 0; i < MAX_WIDTH / RATIO; i++) {
        render_char(0, i + 1, CH(HE), RENDER_COLOR_DEFAULT);
    }

    render_char(0, MAX_WIDTH / RATIO + 1, CH(TR), RENDER_COLOR_DEFAULT);
    map[MAX_WIDTH / RATIO] = '\0';

    int myrow = 0;
    int mycol = 0;
    int orow = -1;

    for (int i = 0; i < MAX_HEIGHT; i++) {
        int row = i / RATIO;
        if (row != orow) {
            if (orow >= 0) {
                // can not use mvprintw() on ibmpc, because PC-Curses is horribly
                // written, and mvprintw() causes the fp emulation library to be
                // linked with PC-Moria, makes the program 10K bigger
                (void)sprintf(prntscrnbuf, "%c%s%c", CH(VE), map, CH(VE));
                render_string(orow + 1, 0, prntscrnbuf, RENDER_COLOR_DEFAULT);
            }

            for (int j = 0; j < MAX_WIDTH / RATIO; j++) {
                map[j] = ' ';
            }

            orow = row;
        }

        for (int j = 0; j < MAX_WIDTH; j++) {
            int col = j / RATIO;
            uint8_t tmp = loc_symbol(i, j);

            if (priority[map[col]] < priority[tmp]) {
                map[col] = tmp;
            }

            if (map[col] == '@') {
                mycol = col + 1; // account for border
                myrow = row + 1;
            }
        }
    }

    if (orow >= 0) {
        (void)sprintf(prntscrnbuf, "%c%s%c", CH(VE), map, CH(VE));
        render_string(orow + 1, 0, prntscrnbuf, RENDER_COLOR_DEFAULT);
    }

    render_char(orow + 2, 0, CH(BL), RENDER_COLOR_DEFAULT);

    for (int i = 0; i < MAX_WIDTH / RATIO; i++) {
        render_char(orow + 2, i + 1, CH(HE), RENDER_COLOR_DEFAULT);
    }

    render_char(orow + 2, MAX_WIDTH / RATIO + 1, CH(BR), RENDER_COLOR_DEFAULT);

    render_string(23, 23, "Hit any key to continue", RENDER_COLOR_DEFAULT);

    if (mycol > 0) {
        render_move_cursor(myrow, mycol);
    }

    (void)inkey();
    restore_screen();
}
