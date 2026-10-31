// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Declarations shared inside app/ only.

#ifndef APP_INTERNAL_H
#define APP_INTERNAL_H

#include "config.h"
#include "constant.h"
#include "types.h"

// 64 bytes for this structure
typedef struct high_scores {
    int32_t points;
    int32_t birth_date;
    int16_t uid;
    int16_t mhp;
    int16_t chp;
    uint8_t dun_level;
    uint8_t lev;
    uint8_t max_dlv;
    uint8_t sex;
    uint8_t race;
    uint8_t class;
    char name[PLAYER_NAME_SIZE];
    char died_from[25];
} high_scores;
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// --- death.c ---
void display_scores(int);
bool duplicate_character(void);
int32_t total_points(void);

// Does not return (calls exit). Declared _Noreturn so the compiler knows.
_Noreturn void exit_game(void);

// --- dungeon.c ---
void dungeon(void);

// --- seeds.c ---
void init_seeds(uint32_t);

// --- input_end_handling.c ---
void handle_input_end(void);
void handle_quit(void);

// --- platform_init.c ---
// Initialize platform services (rendering + input)
// Returns true on success, false on failure
bool platform_init(void);

// Shutdown platform services
void platform_shutdown(void);

// --- save.c ---
bool get_char(bool *);
bool save_char(void);
bool _save_char(char *);
void set_fileptr(FILE *);
void wr_highscore(high_scores *);
void rd_highscore(high_scores *);

// --- signal_handling.c ---
// Handle pending signals in a safe context (called from main loop)
// This performs all the I/O operations that were deferred from the signal handler
void handle_pending_signals(void);

#endif // APP_INTERNAL_H
