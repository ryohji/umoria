// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#ifndef MAP_VIEW_H
#define MAP_VIEW_H

// panel_bounds is static in panel.c; panel_contains is in panel.h.
int get_panel(int, int, int);
uint8_t loc_symbol(int, int);
bool test_light(int, int);
void prt_map(void);

void check_view(void);

#endif // MAP_VIEW_H
