// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Geometry on the map: whether a point is inside it, how far apart two points
// are, whether one can be seen from the other, and what surrounds a point

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "dungeon_map.h"
#include "dungeon_size.h"
#include "externs.h"
#include "floor_items.h"

// Checks a co-ordinate for in bounds status -RAK-
bool in_bounds(int y, int x) {
    if ((y > 0) && (y < dungeon_height() - 1) && (x > 0) && (x < dungeon_width() - 1)) {
        return true;
    } else {
        return false;
    }
}

// Distance between two points -RAK-
int distance(int y1, int x1, int y2, int x2) {
    int dy = y1 - y2;
    if (dy < 0) {
        dy = -dy;
    }

    int dx = x1 - x2;
    if (dx < 0) {
        dx = -dx;
    }

    return ((((dy + dx) << 1) - (dy > dx ? dx : dy)) >> 1);
}

// Checks points north, south, east, and west for a wall -RAK-
// note that y,x is always in_bounds(), i.e. inside the boundary ring:
// 0 < y < height-1 and 0 < x < width-1 (see dungeon_size.h)
int next_to_walls(int y, int x) {
    int i = 0;
    cave_type *c_ptr = square_at(y - 1, x);

    if (c_ptr->fval >= MIN_CAVE_WALL) {
        i++;
    }
    c_ptr = square_at(y + 1, x);
    if (c_ptr->fval >= MIN_CAVE_WALL) {
        i++;
    }
    c_ptr = square_at(y, x - 1);
    if (c_ptr->fval >= MIN_CAVE_WALL) {
        i++;
    }
    c_ptr = square_at(y, x + 1);
    if (c_ptr->fval >= MIN_CAVE_WALL) {
        i++;
    }

    return i;
}

// Checks all adjacent spots for corridors -RAK-
// note that y, x is always in_bounds(), hence no need to check that
// j, k are in_bounds(), even if they are 0 or cur_x-1 is still works
int next_to_corr(int y, int x) {
    int i = 0;

    for (int j = y - 1; j <= (y + 1); j++) {
        for (int k = x - 1; k <= (x + 1); k++) {
            cave_type *c_ptr = square_at(j, k);

            // should fail if there is already a door present
            if (c_ptr->fval == CORR_FLOOR &&
                (c_ptr->tptr == 0 || floor_item_at(c_ptr->tptr)->tval < TV_MIN_DOORS)) {
                i++;
            }
        }
    }

    return i;
}

// A simple, fast, integer-based line-of-sight algorithm.  By Joseph Hall,
// 4116 Brewster Drive, Raleigh NC 27606.  Email to jnh@ecemwl.ncsu.edu.
//
// Returns true if a line of sight can be traced from x0, y0 to x1, y1.
//
// The LOS begins at the center of the tile [x0, y0] and ends at the center of
// the tile [x1, y1].  If los() is to return true, all of the tiles this line
// passes through must be transparent, WITH THE EXCEPTIONS of the starting and
// ending tiles.
//
// We don't consider the line to be "passing through" a tile if it only passes
// across one corner of that tile.

// Because this function uses (short) ints for all calculations, overflow may
// occur if deltaX and deltaY exceed 90.
bool los(int fromY, int fromX, int toY, int toX) {
    int deltaX = toX - fromX;
    int deltaY = toY - fromY;

    // Adjacent?
    if ((deltaX < 2) && (deltaX > -2) && (deltaY < 2) && (deltaY > -2)) {
        return true;
    }

    // Handle the cases where deltaX or deltaY == 0.
    if (deltaX == 0) {
        int p_y; // y position -- loop variable

        if (deltaY < 0) {
            int tmp = fromY;
            fromY = toY;
            toY = tmp;
        }

        for (p_y = fromY + 1; p_y < toY; p_y++) {
            if (square_at(p_y, fromX)->fval >= MIN_CLOSED_SPACE) {
                return false;
            }
        }
        return true;
    } else if (deltaY == 0) {
        int px; // x position -- loop variable

        if (deltaX < 0) {
            int tmp = fromX;
            fromX = toX;
            toX = tmp;
        }

        for (px = fromX + 1; px < toX; px++) {
            if (square_at(fromY, px)->fval >= MIN_CLOSED_SPACE) {
                return false;
            }
        }
        return true;
    }

    // Now, we've eliminated all the degenerate cases.
    // In the computations below, dy (or dx) and m are multiplied by a scale factor,
    // scale = abs(deltaX * deltaY * 2), so that we can use integer arithmetic.
    {
        int px,     // x position
            p_y,    // y position
            scale2; // above scale factor / 2
        int scale,  // above scale factor
            xSign,  // sign of deltaX
            ySign,  // sign of deltaY
            m;      // slope or 1/slope of LOS

        scale2 = abs(deltaX * deltaY);
        scale = scale2 << 1;
        xSign = (deltaX < 0) ? -1 : 1;
        ySign = (deltaY < 0) ? -1 : 1;

        // Travel from one end of the line to the other, oriented along the longer axis.

        if (abs(deltaX) >= abs(deltaY)) {
            int dy; // "fractional" y position

            // We start at the border between the first and second tiles, where
            // the y offset = .5 * slope.  Remember the scale factor.
            // We have:     m = deltaY / deltaX * 2 * (deltaY * deltaX)
            //                = 2 * deltaY * deltaY.

            dy = deltaY * deltaY;
            m = dy << 1;
            px = fromX + xSign;

            // Consider the special case where slope == 1.
            if (dy == scale2) {
                p_y = fromY + ySign;
                dy -= scale;
            } else {
                p_y = fromY;
            }

            while (toX - px) {
                if (square_at(p_y, px)->fval >= MIN_CLOSED_SPACE) {
                    return false;
                }

                dy += m;
                if (dy < scale2) {
                    px += xSign;
                } else if (dy > scale2) {
                    p_y += ySign;
                    if (square_at(p_y, px)->fval >= MIN_CLOSED_SPACE) {
                        return false;
                    }
                    px += xSign;
                    dy -= scale;
                } else {
                    // This is the case, dy == scale2, where the LOS
                    // exactly meets the corner of a tile.
                    px += xSign;
                    p_y += ySign;
                    dy -= scale;
                }
            }
            return true;
        } else {
            int dx; // "fractional" x position
            dx = deltaX * deltaX;
            m = dx << 1;

            p_y = fromY + ySign;
            if (dx == scale2) {
                px = fromX + xSign;
                dx -= scale;
            } else {
                px = fromX;
            }

            while (toY - p_y) {
                if (square_at(p_y, px)->fval >= MIN_CLOSED_SPACE) {
                    return false;
                }
                dx += m;
                if (dx < scale2) {
                    p_y += ySign;
                } else if (dx > scale2) {
                    px += xSign;
                    if (square_at(p_y, px)->fval >= MIN_CLOSED_SPACE) {
                        return false;
                    }
                    p_y += ySign;
                    dx -= scale;
                } else {
                    px += xSign;
                    p_y += ySign;
                    dx -= scale;
                }
            }
            return true;
        }
    }
}

// Given direction "dir", returns new row, column location -RAK-
int mmove(int dir, int *y, int *x) {
    int new_row = 0;
    int new_col = 0;

    switch (dir) {
    case 1:
        new_row = *y + 1;
        new_col = *x - 1;
        break;
    case 2:
        new_row = *y + 1;
        new_col = *x;
        break;
    case 3:
        new_row = *y + 1;
        new_col = *x + 1;
        break;
    case 4:
        new_row = *y;
        new_col = *x - 1;
        break;
    case 5:
        new_row = *y;
        new_col = *x;
        break;
    case 6:
        new_row = *y;
        new_col = *x + 1;
        break;
    case 7:
        new_row = *y - 1;
        new_col = *x - 1;
        break;
    case 8:
        new_row = *y - 1;
        new_col = *x;
        break;
    case 9:
        new_row = *y - 1;
        new_col = *x + 1;
        break;
    }

    bool moved = false;

    if ((new_row >= 0) && (new_row < dungeon_height()) && (new_col >= 0) && (new_col < dungeon_width())) {
        *y = new_row;
        *x = new_col;
        moved = true;
    }

    return moved;
}
