// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Public interface of world/view/.

#ifndef WORLD_VIEW_H
#define WORLD_VIEW_H

#include "config.h"
#include "constant.h"
#include "types.h"

// --- map_view.c ---
// panel_bounds is static in panel.c; panel_contains is in panel.h.
int get_panel(int, int, int);
void prt_map(void);
void check_view(void);

// --- status_line.c ---
void cnv_stat(uint8_t, char *);
void prt_stat(int);
const char *title_string(void);
void prt_cmana(void);
void prt_mhp(void);
void prt_chp(void);
void prt_pac(void);
void prt_gold(void);
void prt_depth(void);
void prt_hunger(void);
void prt_blind(void);
void prt_confused(void);
void prt_afraid(void);
void prt_poisoned(void);
void prt_state(void);
void prt_speed(void);
void prt_study(void);
void prt_winner(void);
void prt_stat_block(void);
void draw_cave(void);

#endif // WORLD_VIEW_H
