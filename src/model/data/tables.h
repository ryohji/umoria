// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#ifndef TABLES_H
#define TABLES_H

extern uint8_t rgold_adj[MAX_RACES][MAX_RACES];

// Following are store definitions
extern owner_type owners[MAX_OWNERS];

// Store records are static in stores.c (see stores.h for store_at / store_count).
extern uint16_t store_choice[MAX_STORES][STORE_CHOICES];

// Defined in tables.c. Each function tests if a store will buy an item tval.
extern bool (*store_buy[MAX_STORES])(int);

// Following are arrays for descriptive pieces
extern const char *colors[MAX_COLORS];

extern const char *mushrooms[MAX_MUSH];

extern const char *woods[MAX_WOODS];

extern const char *metals[MAX_METALS];

extern const char *rocks[MAX_ROCKS];

extern const char *amulets[MAX_AMULETS];

extern const char *syllables[MAX_SYLLABLES];

extern uint8_t blows_table[7][6];

#endif // TABLES_H
