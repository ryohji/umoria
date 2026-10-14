// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#ifndef LIGHTING_H
#define LIGHTING_H

bool no_light(void);
void move_rec(int, int, int, int);
void light_room(int, int);
void lite_spot(int, int);
void move_light(int, int, int, int);

#endif // LIGHTING_H
