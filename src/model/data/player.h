// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#ifndef PLAYER_H
#define PLAYER_H

#include "config.h"
#include "constant.h"
#include "types.h"

// Following are player variables
extern const char *player_title[MAX_CLASS][MAX_PLAYER_LEVEL];

extern race_type race[MAX_RACES];

extern background_type background[MAX_BACKGROUND];

extern uint32_t player_exp[MAX_PLAYER_LEVEL];

extern class_type class[MAX_CLASS];

extern int16_t class_level_adj[MAX_CLASS][MAX_LEV_ADJ];

// Warriors don't have spells, so there is no entry for them.
extern spell_type magic_spell[MAX_CLASS - 1][31];

extern const char *spell_names[62];

extern uint16_t player_init[MAX_CLASS][5];

#endif // PLAYER_H
