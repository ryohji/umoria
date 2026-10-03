// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The tables that turn a player stat into a bonus.

#ifndef STATS_H
#define STATS_H

// Seven stat-to-bonus tables.
//
// Every one of them reads the player's used stat (py.stats.use_stat) rather
// than taking it as a parameter. Only stat_adj() takes an argument, and that
// argument is which stat to read.

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

// Read player stat fields.
// Each returns the value of one of the four arrays in py.stats for the given
// stat index (A_STR, A_INT, A_WIS, A_DEX, A_CON, or A_CHR).
uint8_t player_stat_max(int stat);
uint8_t player_stat_cur(int stat);
int16_t player_stat_mod(int stat);
uint8_t player_stat_use(int stat);

// Write player stat fields.
void player_stat_set_max(int stat, uint8_t value);
void player_stat_set_cur(int stat, uint8_t value);
void player_stat_add_mod(int stat, int16_t amount);
void player_stat_set_use(int stat, uint8_t value);

#endif // STATS_H
