// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Distance between two points

#include "distance.h"

// This module calls nothing outside itself.

// Distance between two points -RAK-
int distance(coordinate from, coordinate to) {
    int dy = from.row - to.row;
    if (dy < 0) {
        dy = -dy;
    }

    int dx = from.column - to.column;
    if (dx < 0) {
        dx = -dx;
    }

    return ((((dy + dx) << 1) - (dy > dx ? dx : dy)) >> 1);
}
