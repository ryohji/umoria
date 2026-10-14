// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#ifndef UI_IO_H
#define UI_IO_H

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

#endif // UI_IO_H
