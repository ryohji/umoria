// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#include <string.h>
#include "config.h"
#include "constant.h"
#include "types.h"
#include "player_identity.h"
#include "player.h"

// Six identity stores. All start at zero (empty character).
static char the_name[PLAYER_NAME_SIZE];
static bool the_sex;
static uint16_t the_age;
static uint16_t the_height;
static int16_t the_social_class;
static char the_history[PLAYER_HISTORY_LINES][PLAYER_HISTORY_LINE_SIZE];

const char *player_name(void) {
    return the_name;
}

bool player_is_male(void) {
    return the_sex;
}

int player_age(void) {
    return the_age;
}

int player_height(void) {
    return the_height;
}

int player_social_class(void) {
    return the_social_class;
}

const char *player_history_line(int line) {
    return the_history[line];
}

void player_name_set(const char *name) {
    // Emptied first to ensure all bytes past terminator are zero.
    char *store = the_name;
    memset(store, 0, PLAYER_NAME_SIZE);
    (void)strncpy(store, name, PLAYER_NAME_SIZE - 1);
}

void player_set_male(bool male) {
    the_sex = male;
}

void player_age_set(int age) {
    the_age = (uint16_t)age;
}

void player_height_set(int height) {
    the_height = (uint16_t)height;
}

void player_social_class_set(int social_class) {
    the_social_class = (int16_t)social_class;
}

void player_history_line_set(int line, const char *text) {
    // Emptied first so strncpy() pads to make the terminator at position 60.
    char *store = the_history[line];
    memset(store, 0, PLAYER_HISTORY_LINE_SIZE);
    (void)strncpy(store, text, PLAYER_HISTORY_LINE_SIZE - 1);
}

void player_history_clear(void) {
    for (int line = 0; line < PLAYER_HISTORY_LINES; line++) {
        memset(the_history[line], 0, PLAYER_HISTORY_LINE_SIZE);
    }
}

// Race row. Starts at 0 (Human).
static uint8_t race_the_row;

int player_race(void) {
    return race_the_row;
}

void player_race_set(int row) {
    race_the_row = (uint8_t)row;
}

const char *player_race_name(void) {
    return race[race_the_row].trace;
}

// Class row. Starts at 0 (Warrior).
static uint8_t class_the_row;

int player_class(void) {
    return class_the_row;
}

void player_class_set(int row) {
    class_the_row = (uint8_t)row;
}

const char *player_class_title(void) {
    return class[class_the_row].title;
}

int player_class_spell_type(void) {
    return class[class_the_row].spell;
}
