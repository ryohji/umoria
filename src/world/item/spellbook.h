// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#ifndef SPELLBOOK_H
#define SPELLBOOK_H

void print_spells(int *, int, int, int);
void calc_spells(int);
void gain_spells(void);
void calc_mana(int);
int cast_spell(const char *, int, int *, int *);

#endif // SPELLBOOK_H
