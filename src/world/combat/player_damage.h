// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#ifndef PLAYER_DAMAGE_H
#define PLAYER_DAMAGE_H

bool player_saves(void);
void corrode_gas(const char *);
void poison_gas(int, const char *);
void fire_dam(int, const char *);
void cold_dam(int, char *);
void light_dam(int, char *);
void acid_dam(int, const char *);
void take_hit(int, const char *);

#endif // PLAYER_DAMAGE_H
