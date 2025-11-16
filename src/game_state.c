// Copyright (c) 2025 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Game state implementation

#include "game_state.h"

#include "constant.h"
#include "externs.h"

#include <stdlib.h>
#include <string.h>

// Global game state instance
// This is temporary during migration - eventually will be passed as parameter
GameState *g_game_state = NULL;

// Initialize game state
GameState *game_state_init(void) {
    GameState *state = malloc(sizeof(GameState));
    if (state == NULL) {
        return NULL;
    }

    // Initialize to zero
    memset(state, 0, sizeof(GameState));

    // Point to existing global variables (for backward compatibility during migration)
    // These will eventually be moved into the GameState structure itself
    state->player = &py;
    state->cave = cave;
    state->monsters = m_list;
    state->treasure = t_list;
    state->inventory = inventory;
    state->stores = store;
    state->old_messages = old_msg;

    // Initialize game metadata from existing globals
    state->dungeon_level = dun_level;
    state->turn = turn;
    state->death = death;
    state->wizard_mode = wizard;

    // Flags
    state->new_level_flag = new_level_flag;
    state->teleport_flag = teleport_flag;
    state->character_generated = character_generated;
    state->character_saved = character_saved;

    // Command state
    state->command_count = command_count;
    state->default_dir = default_dir;

    // Message system
    state->msg_flag = msg_flag;
    state->last_msg_index = last_msg;

    // Seeds
    state->rng_seed = randes_seed;
    state->town_seed = town_seed;

    // File paths
    strncpy(state->save_file_path, savefile, sizeof(vtype) - 1);
    strncpy(state->died_from, died_from, sizeof(vtype) - 1);
    state->birth_date = birth_date;
    state->highscore_fp = highscore_fp;

    // Options
    state->rogue_like_commands = rogue_like_commands;
    state->find_cut = find_cut;
    state->find_examine = find_examine;
    state->find_prself = find_prself;
    state->find_bound = find_bound;
    state->prompt_carry_flag = prompt_carry_flag;
    state->show_weight_flag = show_weight_flag;
    state->highlight_seams = highlight_seams;
    state->find_ignore_doors = find_ignore_doors;
    state->sound_beep_flag = sound_beep_flag;
    state->display_counts = display_counts;

    // Runtime state
    state->player_light = player_light;
    state->find_flag = find_flag;
    state->free_turn_flag = free_turn_flag;
    state->weapon_heavy = weapon_heavy;
    state->pack_heavy = pack_heavy;
    state->doing_inven = doing_inven;
    state->screen_change = screen_change;
    state->eof_flag = eof_flag;
    state->noscore = noscore;
    state->panic_save = panic_save;
    state->wait_for_more = wait_for_more;
    state->closing_flag = closing_flag;

    // Dungeon dimensions
    state->cur_height = cur_height;
    state->cur_width = cur_width;
    state->max_panel_rows = max_panel_rows;
    state->max_panel_cols = max_panel_cols;

    // Temporary
    state->hack_monptr = hack_monptr;
    state->missile_ctr = missile_ctr;

    // Set global instance
    g_game_state = state;

    return state;
}

// Free game state
void game_state_free(GameState *state) {
    if (state == NULL) {
        return;
    }

    // Note: We don't free the pointers to existing globals
    // (player, cave, monsters, etc.) as they're still owned elsewhere

    // Clear global instance if it matches
    if (g_game_state == state) {
        g_game_state = NULL;
    }

    free(state);
}
