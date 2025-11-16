// Copyright (c) 2025 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Game state encapsulation
// Centralizes game state that was previously scattered in global variables

#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "constant.h"
#include "types.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// Forward declaration
typedef struct GameState GameState;

// Game state structure
// This encapsulates the main game state that was previously global
struct GameState {
    // Player state
    player_type *player;  // Player data (points to existing py global for now)

    // World state
    cave_type (*cave)[MAX_WIDTH];  // Dungeon map (points to existing cave global)
    monster_type *monsters;         // Monster list (points to existing m_list)
    inven_type *treasure;           // Treasure list (points to existing t_list)
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
    vtype *old_messages;   // Message history (points to old_msg global)
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

    // Dungeon dimensions
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

static inline player_type *game_state_get_player(GameState *state) {
    return state->player;
}

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

#endif // GAME_STATE_H
