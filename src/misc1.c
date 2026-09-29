// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Misc utility and initialization code, magic objects code

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"
#include "dungeon_map.h"
#include "dungeon_size.h"
#include "progress.h"

// gets a new random seed for the random number generator
void init_seeds(uint32_t seed) {
    uint32_t clock_var;

    if (seed == 0) {
        clock_var = (uint32_t)time((time_t *)0);
    } else {
        clock_var = seed;
    }
    progress_set_color_seed(clock_var);

    clock_var += 8762;
    progress_set_town_seed(clock_var);

    clock_var += 113452L;
    set_rnd_seed(clock_var);
    // make it a little more random
    for (clock_var = (uint32_t)randint(100); clock_var != 0; clock_var--) {
        (void)rnd();
    }
}

// holds the previous rnd state
static uint32_t old_seed;

// change to different random number generator state
void set_seed(uint32_t seed) {
    old_seed = get_rnd_seed();

    // want reproducible state here
    set_rnd_seed(seed);
}

// restore the normal random generator state
void reset_seed(void) {
    set_rnd_seed(old_seed);
}

// Generates a random integer x where 1<=X<=MAXVAL -RAK-
int randint(int maxval) {
    int32_t randval = rnd();
    return ((int)(randval % maxval) + 1);
}

// Generates a random integer number of NORMAL distribution -RAK-
int randnor(int mean, int stand) {
    // alternate randnor code, slower but much smaller since no table
    // 2 per 1,000,000 will be > 4*SD, max is 5*SD
    //
    // tmp = damroll(8, 99);   // mean 400, SD 81
    // tmp = (tmp - 400) * stand / 81;
    // return tmp + mean;

    int tmp = randint(MAX_SHORT);

    // off scale, assign random value between 4 and 5 times SD
    if (tmp == MAX_SHORT) {
        int offset = 4 * stand + randint(stand);

        // one half are negative
        if (randint(2) == 1) {
            offset = -offset;
        }

        return mean + offset;
    }

    // binary search normal normal_table to get index that
    // matches tmp this takes up to 8 iterations.
    int low = 0;
    int iindex = NORMAL_TABLE_SIZE >> 1;
    int high = NORMAL_TABLE_SIZE;

    while (true) {
        if ((normal_table[iindex] == tmp) || (high == (low + 1))) {
            break;
        }
        if (normal_table[iindex] > tmp) {
            high = iindex;
            iindex = low + ((iindex - low) >> 1);
        } else {
            low = iindex;
            iindex = iindex + ((high - iindex) >> 1);
        }
    }

    // might end up one below target, check that here
    if (normal_table[iindex] < tmp) {
        iindex = iindex + 1;
    }

    // normal_table is based on SD of 64, so adjust the
    // index value here, round the half way case up.
    int offset = ((stand * iindex) + (NORMAL_TABLE_SD >> 1)) / NORMAL_TABLE_SD;

    // one half should be negative
    if (randint(2) == 1) {
        offset = -offset;
    }

    return mean + offset;
}

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

// generates damage for 2d6 style dice rolls
int damroll(int num, int sides) {
    int sum = 0;
    for (int i = 0; i < num; i++) {
        sum += randint(sides);
    }
    return sum;
}

int pdamroll(const uint8_t *array) {
    return damroll((int)array[0], (int)array[1]);
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

// Gives Max hit points -RAK-
int max_hp(const uint8_t *array) {
    return (array[0] * array[1]);
}

// Should the object be enchanted -RAK-
bool magik(int chance) {
    if (randint(100) <= chance) {
        return true;
    } else {
        return false;
    }
}
