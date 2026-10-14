// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#ifndef STAT_OPS_H
#define STAT_OPS_H

void set_use_stat(int);
bool inc_stat(int);
bool dec_stat(int);
bool res_stat(int);
void bst_stat(int, int);

#endif // STAT_OPS_H
