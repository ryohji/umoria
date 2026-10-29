// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Public interface of command/: commands.

#ifndef COMMANDS_H
#define COMMANDS_H

#include "config.h"
#include "constant.h"
#include "types.h"

// --- char_screen.c ---
void display_char(void);
void change_name(void);

// --- magic.c ---
void cast(void);

// --- create.c ---
void create_character(void);

// --- eat.c ---
bool eat(void);

// --- files.c ---
void init_scorefile(void);
void read_times(void);
void helpfile(const char *);
bool file_character(char *);

// --- help.c ---
void ident_char(void);

// --- inven_menu.c ---
void inven_command(char);

// --- look.c ---
void look(void);

// --- options_menu.c ---
void set_options(void);

// --- potions.c ---
bool quaff(void);

// --- prayer.c ---
void pray(void);

// --- scrolls.c ---
void read_scroll(void);

// --- staffs.c ---
bool use(void);

// --- store_ui.c ---
void enter_store(int);

// --- wands.c ---
bool aim(void);

// --- wizard.c ---
bool enter_wiz_mode(void);
void wizard_command(char);

#endif // COMMANDS_H
