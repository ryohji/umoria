// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#ifndef PLAYER_IDENTITY_H
#define PLAYER_IDENTITY_H

#include "config.h"
#include "constant.h"
#include "types.h"

// Identity answers: name, sex, age, height, social class, life story. Set once
// at creation (except name, which change_name() can replace), never adjusted.

// Life story dimensions: 4 lines, 60 characters plus terminator each.
#define PLAYER_HISTORY_LINES 4
#define PLAYER_HISTORY_LINE_SIZE 61

// Character's name. Returns const to prevent bypassing player_name_set().
const char *player_name(void);

// Whether character is male. bool, not numeric.
bool player_is_male(void);

// Age and height in years and inches.
int player_age(void);
int player_height(void);

// Social class (1..100). Clamping is creation's responsibility.
int player_social_class(void);

// One line of life story. Line number not bounds-checked; callers loop to PLAYER_HISTORY_LINES.
const char *player_history_line(int line);

// Set identity values. Names and lines longer than their limits are truncated.
void player_name_set(const char *name);
void player_set_male(bool male);
void player_age_set(int age);
void player_height_set(int height);
void player_social_class_set(int social_class);
void player_history_line_set(int line, const char *text);

// Clear all four life story lines.
void player_history_clear(void);

// Race: row number into race[] (0=Human, 1=Half-Elf, ... 7=Half-Troll).
// Set twice: creation and save file load. Never adjusted.

// Race row number as array subscript.
int player_race(void);

// Set race row. Not bounds-checked.
void player_race_set(int row);

// Race name from race[] table.
const char *player_race_name(void);

// Class: row number into class[] (0=Warrior, 1=Mage, ... 5=Paladin).
// Set twice: creation and save file load. Never adjusted.

// Class row number as array subscript.
int player_class(void);

// Set class row. Not bounds-checked.
void player_class_set(int row);

// Class name from class[] table.
const char *player_class_title(void);

// Magic school: NONE, MAGE, or PRIEST. Returns the value, not separate predicates,
// because some callers branch on all three.
int player_class_spell_type(void);

#endif // PLAYER_IDENTITY_H
