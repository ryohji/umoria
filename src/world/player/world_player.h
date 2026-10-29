// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Public interface of world/player/.

#ifndef WORLD_PLAYER_H
#define WORLD_PLAYER_H

#include "config.h"
#include "constant.h"
#include "types.h"

// --- food_ops.c ---
void add_food(int);

// --- level_ops.c ---
void prt_experience(void);

// --- player_bonuses.c ---
void change_speed(int);
void py_bonuses(inven_type *, int);
void calc_bonuses(void);

// --- player_move.c ---
void teleport(int);
void move_char(int, bool);

// --- rest_command.c ---
void disturb(int, int);
void search_on(void);
void search_off(void);
bool rest(void);
void rest_off(void);

// --- run_path.c ---
void find_init(int);
void find_run(void);

// --- stat_loss.c ---
// Losing a stat point to poison, bad food or a potion.
// Lowers `stat` (A_STR to A_CHR) by one point unless it is sustained, and says
// which happened. Each stat has its own pair of messages.
void lose_stat(int stat);

// --- stat_ops.c ---
void set_use_stat(int);
bool inc_stat(int);
bool dec_stat(int);
bool res_stat(int);

#endif // WORLD_PLAYER_H
