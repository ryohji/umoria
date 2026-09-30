// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Small writes into fields of the screen, shared by the status line, the
// character screen and the experience display

#ifndef SCREEN_FIELDS_H
#define SCREEN_FIELDS_H

#include <stdint.h>

// Writes width blanks at row, column. width is at most 24.
void erase_field(int width, int row, int column);

// Writes the label of a stat ("STR : " and so on) at row, column. stat is one
// of A_STR .. A_CHR.
void prt_stat_name(int stat, int row, int column);

// Writes "header: " and num in six places at row, column.
void prt_num(const char *header, int num, int row, int column);

// Writes num in six places at row, column.
void prt_long(int32_t num, int row, int column);

#endif // SCREEN_FIELDS_H
