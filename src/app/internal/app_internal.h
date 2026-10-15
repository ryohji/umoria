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

// --- game_state.c ---
// Forward declaration
typedef struct GameState GameState;

// Game state structure
// This encapsulates the main game state that was previously global
struct GameState {
    // World state
    cave_type *cave;                // Dungeon map (square (0,0) of dungeon_map.c's table)
    monster_type *monsters;         // Monster list (row 0 of monster_list.c's table)
    inven_type *treasure;           // Floor items (row 0 of floor_items.c's table)
    inven_type *inventory;          // Player inventory
    store_type *stores;             // Store data

    // Game metadata
    int16_t dungeon_level;  // Current dungeon depth
    int32_t turn;           // Current game turn
    bool death;             // Player death flag
    bool wizard_mode;       // Wizard mode active

    // Flags
    bool new_level_flag;  // Trigger level generation
    bool teleport_flag;   // Handle teleport
    bool character_generated;
    bool character_saved;

    // Command state
    int command_count;     // Command repetition counter
    bool default_dir;      // Use last direction

    // Message system
    bool msg_flag;
    vtype *old_messages;   // Message history (points into messages.c)
    int16_t last_msg_index;

    // Seeds
    uint32_t rng_seed;
    uint32_t town_seed;

    // File I/O
    vtype save_file_path;  // Path to save file
    vtype died_from;       // Death reason
    int32_t birth_date;    // Character creation time
    FILE *highscore_fp;    // High score file

    // Options (user preferences)
    bool rogue_like_commands;
    bool find_cut;
    bool find_examine;
    bool find_prself;
    bool find_bound;
    bool prompt_carry_flag;
    bool show_weight_flag;
    bool highlight_seams;
    bool find_ignore_doors;
    bool sound_beep_flag;
    bool display_counts;

    // Runtime state
    bool player_light;     // Player has light
    int find_flag;         // Running mode
    bool free_turn_flag;   // Free action
    bool weapon_heavy;     // Weapon too heavy
    int pack_heavy;        // Pack too heavy
    char doing_inven;      // Inventory UI state
    bool screen_change;    // Screen update needed
    int eof_flag;          // EOF detected
    int16_t noscore;       // Don't record score
    bool panic_save;       // Emergency save
    bool wait_for_more;    // Waiting for user
    int closing_flag;      // Game closing

    // Dungeon dimensions (a copy of dungeon_size.c's pair)
    int16_t cur_height;
    int16_t cur_width;
    int16_t max_panel_rows;
    int16_t max_panel_cols;

    // Temporary/utility
    int hack_monptr;       // Compact monster hack
    int16_t missile_ctr;   // Missile counter
};

// Global game state instance
// Eventually this will be passed as a parameter instead of being global
extern GameState *g_game_state;

// Initialize game state
// Allocates and initializes the global game state
GameState *game_state_init(void);

// Free game state
// Cleans up allocated game state
void game_state_free(GameState *state);

// Accessor functions (for gradual migration)
// These provide access to state through the GameState structure

static inline int16_t game_state_get_dungeon_level(GameState *state) {
    return state->dungeon_level;
}

static inline void game_state_set_dungeon_level(GameState *state, int16_t level) {
    state->dungeon_level = level;
}

static inline int32_t game_state_get_turn(GameState *state) {
    return state->turn;
}

static inline void game_state_increment_turn(GameState *state) {
    state->turn++;
}

static inline bool game_state_is_dead(GameState *state) {
    return state->death;
}

static inline void game_state_set_death(GameState *state, bool dead) {
    state->death = dead;
}

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
