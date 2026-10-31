// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// A square of the dungeon map, by its row and column. Two ints side by side
// say nothing about which is which; this type says it.

#ifndef COORDINATE_H
#define COORDINATE_H

// Row 0 is the top of the map, column 0 its left.
typedef struct {
    int row;
    int column;
} coordinate;

#endif // COORDINATE_H
