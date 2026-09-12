// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The panel: the part of the dungeon the screen is showing.

#ifndef PANEL_H
#define PANEL_H

#include <stdbool.h>

// Two indexes say which panel we are looking at; six dungeon coordinates say
// what that panel covers. The six are derived from the two, but every one of
// them used to be a global, and four files each did their own arithmetic on
// them: misc1.c derived them, io.c subtracted the two print offsets to reach
// screen coordinates, spells.c and misc1.c walked between the four edges, and
// generate.c set four of them by hand without deriving them at all. Only this
// module now knows how the six follow from the two.

// The map does not fill the terminal: the message line is above it and the
// status column to its left. prt_map() draws from here, and the two print
// offsets convert dungeon coordinates into that area, so both have to agree.
#define PANEL_MAP_TOP_ROW 1
#define PANEL_MAP_LEFT_COL 13

// --- which panel we are looking at ---

int panel_row_index(void);
int panel_col_index(void);

// Moves the window if (y, x) has come within two rows or three columns of an
// edge, and returns true if it moved. `force` recalculates regardless, which
// is what the 'W'here command wants. -RAK-
bool panel_move_to(int y, int x, bool force);

// "We are not looking at any panel yet." Both are set outside the valid range
// so that the next panel_move_to() is bound to see a change and redraw. The
// two halves are apart because the code that used them is apart: dungeon()
// forgets the position when a level starts, generate_cave() forgets the bounds
// while it builds one. Each on its own guarantees the redraw; the game does
// both.
void panel_forget_position(void);
void panel_forget_bounds(void);

// --- the dungeon coordinates that panel covers ---

int panel_top_row(void);
int panel_bottom_row(void);
int panel_left_col(void);
int panel_right_col(void);

// Tests a given point to see if it is within the screen boundaries. -RAK-
bool panel_contains(int y, int x);

// --- dungeon coordinates to screen coordinates ---

int panel_screen_row(int dungeon_row);
int panel_screen_col(int dungeon_col);

// --- how many panels the dungeon is worth ---

// Also parks the window on the last panel, which is what generate_cave() has
// always done right before it builds the level. Nothing reads the window
// before dungeon() forgets it again, but keep the two together: they were one
// block of code, written twice.
void panel_set_dungeon_size(int height, int width);

int panel_max_row_index(void);
int panel_max_col_index(void);

// Only for the save file, which stores the two counts rather than the dungeon
// size they come from.
void panel_set_max_indexes(int max_row, int max_col);

#endif // PANEL_H
