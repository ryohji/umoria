// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The tables that turn a player stat into a bonus.

#ifndef STATS_H
#define STATS_H

// Seven stat-to-bonus tables, moved out of misc3.c unchanged. They were spread
// over two places in that file, 400 lines apart, with the screen-printing code
// in between; nothing but the shared shape of the lookup held them together,
// and that shape was invisible while they were apart.
//
// Every one of them reads the player's used stat (py.stats.use_stat) rather
// than taking it as a parameter, so the callers stay as they were. Only
// stat_adj() takes an argument, and that argument is which stat to read.

// Adjustment for wisdom/intelligence -JWT-
// `stat` is one of A_STR .. A_CHR. The only adjustment with a floor of 0.
int stat_adj(int stat);

// Adjustment for charisma -RAK-
// Percent decrease or increase in price of goods
int chr_adj(void);

// Returns a character's adjustment to hit points -JWT-
int con_adj(void);

// Returns a character's adjustment to hit. -JWT-
int tohit_adj(void);

// Returns a character's adjustment to armor class -JWT-
int toac_adj(void);

// Returns a character's adjustment to disarm -RAK-
int todis_adj(void);

// Returns a character's adjustment to damage -JWT-
int todam_adj(void);

#endif // STATS_H
