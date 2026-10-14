// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Declarations shared inside command/ only.

#ifndef COMMAND_INTERNAL_H
#define COMMAND_INTERNAL_H

#include "config.h"
#include "constant.h"
#include "types.h"

// --- char_screen.c ---
void put_character(void);
void put_stats(void);
const char *likert(int, int);
void put_misc1(void);
void put_misc2(void);
void put_misc3(void);
void get_name(void);

// --- files.c ---
void print_objects(void);

// --- recall.c ---
bool bool_roff_recall(creature_type *);
int roff_recall(creature_type *);

// --- wizard.c ---
void wizard_light(void);
void change_character(void);
void wizard_create(void);

#endif // COMMAND_INTERNAL_H
