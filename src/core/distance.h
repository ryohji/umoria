// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Distance between two points.

#ifndef DISTANCE_H
#define DISTANCE_H

// The distance between (y1, x1) and (y2, x2): the longer leg plus half the
// shorter one, rounded down. The same prototype is in externs.h, which is
// where every caller reads it.
int distance(int y1, int x1, int y2, int x2);

#endif // DISTANCE_H
