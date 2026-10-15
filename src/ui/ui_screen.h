// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Public interface of ui/: ui_screen.

#ifndef UI_SCREEN_H
#define UI_SCREEN_H

#include "config.h"
#include "constant.h"
#include "types.h"
#include <stdbool.h>
#include <stdint.h>

// --- messages.c ---
// The messages shown on the top line: their rolling history -CJS-, and whether
// the one up there now has been seen by the player.
// The history is a ring buffer. The wrap-around rule lives here only:
// msg_print(), the ^P command and the save file go through the windows below.

// --- newest-first view: for whoever wants to read the messages back ---

// `back` counts backwards from the newest message: 0 is the one on screen now,
// 1 the one before it. Wraps around, so asking further back than the history
// holds returns an old message rather than reading outside the ring.
const char *msg_history_recent(int back);

// --- storage view: for the save file, whose format is the raw ring ---

// Number of slots, and which slot currently holds the newest message. The save
// file stores every slot in storage order plus this index, so both are needed
// to write and to read one.
int msg_history_slot_count(void);
char *msg_history_slot(int slot);
int msg_history_newest_slot(void);
void msg_history_set_newest_slot(int slot);

// The whole block at once. Only for app/game_state.c, whose snapshot holds a
// pointer to the history rather than a copy of it. Walking the result by hand
// is what this module exists to stop -- use msg_history_recent() to read.
vtype *msg_history_slots(void);

// --- the top line itself, and its -more- prompt ---

// True while the top line holds a message the player has not acknowledged.
// msg_print() asks for -more- before letting anything overwrite one, and
// erase_line()/clear_screen() flush it rather than wiping it unseen.
bool msg_pending(void);

// msg_print() decides this for the message it prints. Others only ever clear
// it, meaning "the player has seen what is up there": both command loops
// (app/dungeon.c, store_ui.c) do so before reading a key, and inkey() does so on EOF,
// where nobody is left to answer a -more- prompt.
void msg_set_pending(bool pending);

// True while msg_print() is stopped at a -more- prompt. The interrupt handler
// reads it: if the player cancels a *Suicide* the prompt has to be put back.
bool msg_at_more_prompt(void);

// --- panel.c ---
// The panel: the part of the dungeon the screen is showing.
#define PANEL_MAP_LEFT_COL 13

// --- which panel we are looking at ---

int panel_row_index(void);
int panel_col_index(void);

// Moves the window if (y, x) has come within two rows or three columns of an
// edge, and returns true if it moved. `force` recalculates regardless, which
// is what the 'W'here command wants. -RAK-
bool panel_move_to(int y, int x, bool force);

// "We are not looking at any panel yet." Both are set outside the valid range
// so that the next panel_move_to() is bound to see a change and redraw. The
// two halves are apart because the code that used them is apart: dungeon()
// forgets the position when a level starts, generate_cave() forgets the bounds
// while it builds one. Each on its own guarantees the redraw; the game does
// both.
void panel_forget_position(void);
void panel_forget_bounds(void);

// --- the dungeon coordinates that panel covers ---

int panel_top_row(void);
int panel_bottom_row(void);
int panel_left_col(void);
int panel_right_col(void);

// Tests a given point to see if it is within the screen boundaries. -RAK-
bool panel_contains(int y, int x);

// --- dungeon coordinates to screen coordinates ---

int panel_screen_row(int dungeon_row);

// --- how many panels the dungeon is worth ---

// Also parks the window on the last panel, which is what generate_cave() has
// always done right before it builds the level. Nothing reads the window
// before dungeon() forgets it again, but keep the two together: they were one
// block of code, written twice.
void panel_set_dungeon_size(int height, int width);
int panel_max_row_index(void);
int panel_max_col_index(void);

// Only for the save file, which stores the two counts rather than the dungeon
// size they come from.
void panel_set_max_indexes(int max_row, int max_col);

// --- screen_fields.c ---
// Small writes into fields of the screen, shared by the status line, the
// character screen and the experience display
// Writes width blanks at row, column. width is at most 24.
void erase_field(int width, int row, int column);

// Writes the label of a stat ("STR : " and so on) at row, column. stat is one
// of A_STR .. A_CHR.
void prt_stat_name(int stat, int row, int column);

// Writes "header: " and num in six places at row, column.
void prt_num(const char *header, int num, int row, int column);

// Writes num in six places at row, column.
void prt_long(int32_t num, int row, int column);

// --- screen_touched.c ---
// Whether the screen has been flushed since somebody last said it had not
// One bit, and it answers one question: has anything been put on the screen
// since the flag was last forgotten? Nobody cares about the screen in general --
// the only reader is the inventory command (inven_menu.c), which saves the screen,
// hands control back to the rest of moria for a turn, and on the way back in
// needs to know whether what it saved is still what the player is looking at. If
// it is, the inventory screen can be redrawn silently; if it is not, the player
// is asked whether to carry on, because something happened that they should see.
//
// The screen was flushed. Said by put_qio() (io.c), which is the one place that
// pushes the buffer out, and by creature.c when a monster appears or vanishes --
// that redraws a single spot without going through put_qio().
void note_screen_flushed(void);

// Has it? The inventory command asks this when it resumes.
bool screen_was_flushed(void);

// --- io.c ---
void put_buffer(const char *, int, int);
void put_qio(void);
void shell_out(void);
void io_set_input_end_handler(void (*handler)(void));
void io_set_quit_handler(void (*handler)(void));
char inkey(void);
void flush(void);
void erase_line(int, int);
void clear_screen(void);
void clear_from(int);
void print(char, int, int);
void move_cursor_relative(int, int);
void count_msg_print(const char *);
void prt(const char *, int, int);
void move_cursor(int, int);
void msg_print(const char *);
bool get_check(const char *);
int get_com(const char *, char *);
bool get_string(char *, int, int, int);
void pause_line(int);
void pause_exit(int, int);
void save_screen(void);
void restore_screen(void);
void bell(void);
void screen_map(void);
bool check_input(int);
void user_name(char *);

// Open files with a leading ~ expanded to the home directory.
#ifndef _WIN32
// call functions which expand tilde before calling open/fopen
#define open topen
#define fopen tfopen

FILE *tfopen(const char *, const char *);
int topen(char *, int, int);
#endif

#endif // UI_SCREEN_H
