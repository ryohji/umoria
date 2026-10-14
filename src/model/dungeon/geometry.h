// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#ifndef GEOMETRY_H
#define GEOMETRY_H

bool in_bounds(int, int);
bool los(int, int, int, int);
int next_to_walls(int, int);
int next_to_corr(int, int);
int mmove(int, int *, int *);

#endif // GEOMETRY_H
