// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Distance between two points.

#ifndef DISTANCE_H
#define DISTANCE_H

#include "coordinate.h"

// The distance between two squares: the longer leg plus half the shorter one,
// rounded down.
int distance(coordinate from, coordinate to);

#endif // DISTANCE_H
