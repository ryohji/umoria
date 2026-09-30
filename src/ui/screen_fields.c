// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Small writes into fields of the screen, shared by the status line, the
// character screen and the experience display
//
// The two tables are static; the other files reach them through erase_field()
// and prt_stat_name().

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"
#include "screen_fields.h"

static const char *stat_names[] = {
    "STR : ",
    "INT : ",
    "WIS : ",
    "DEX : ",
    "CON : ",
    "CHR : ",
};
#define BLANK_LENGTH 24
static char blank_string[] = "                        ";

// Writes width blanks at row, column, to wipe a field before or instead of
// writing into it. width is at most BLANK_LENGTH.
void erase_field(int width, int row, int column) {
    put_buffer(&blank_string[BLANK_LENGTH - width], row, column);
}

// Writes the label of a stat ("STR : " and so on) at row, column.
void prt_stat_name(int stat, int row, int column) {
    put_buffer(stat_names[stat], row, column);
}

// Print number with header at given row, column -RAK-
void prt_num(const char *header, int num, int row, int column) {
    vtype out_val;
    (void)sprintf(out_val, "%s: %6d", header, num);
    put_buffer(out_val, row, column);
}

// Print long number at given row, column
void prt_long(int32_t num, int row, int column) {
    vtype out_val;
    (void)sprintf(out_val, "%6d", num);
    put_buffer(out_val, row, column);
}
