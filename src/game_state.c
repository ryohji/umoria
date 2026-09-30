// Copyright (c) 2025-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Game state implementation

#include "game_state.h"

#include "burden.h"
#include "command_state.h"
#include "dungeon_level.h"
#include "dungeon_map.h"
#include "dungeon_size.h"
#include "constant.h"
#include "externs.h"
#include "floor_items.h"
#include "input_ended.h"
#include "inven_command_state.h"
#include "inventory.h"
#include "level_exit.h"
#include "missile_serial.h"
#include "monster_list.h"
#include "monster_turn.h"
#include "panel.h"
#include "pending_teleport.h"
#include "player_light.h"
#include "progress.h"
#include "running.h"
#include "save_state.h"
#include "screen_touched.h"
#include "score_death.h"
#include "stores.h"
#include "messages.h"

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
    state->cave = square_at(0, 0);
    state->monsters = monster_list_at(0);
    state->treasure = floor_item_at(0);
    state->inventory = inventory_and_equipment_at(0);
    state->stores = store_at(0);
    state->old_messages = msg_history_slots();

    // Initialize game metadata from existing globals
    state->dungeon_level = dungeon_level();
    state->turn = progress_turn();
    state->death = player_is_dead();
    state->wizard_mode = progress_wizard_mode();

    // Flags
    state->new_level_flag = level_is_over();
    state->teleport_flag = teleport_is_pending();
    state->character_generated = character_is_generated();
    state->character_saved = character_is_saved();

    // Command state
    state->command_count = command_count_remaining();
    state->default_dir = direction_is_remembered();

    // Message system
    state->msg_flag = msg_pending();
    state->last_msg_index = (int16_t)msg_history_newest_slot();

    // Seeds
    state->rng_seed = progress_color_seed();
    state->town_seed = progress_town_seed();

    // File paths
    // strncpy は上限まで詰まったとき終端の '\0' を書かない。ここでやりたい
    // のは「収まらなければ切り詰め、必ず終端する」なので、それをそのまま
    // 表す snprintf を使う。戻り値は切り詰めが起きたかを示すが、状態の
    // 写しとりに失敗の扱いはないので捨てる。
    (void)snprintf(state->save_file_path, sizeof(vtype), "%s", save_file_path());
    (void)snprintf(state->died_from, sizeof(vtype), "%s", death_cause());
    state->birth_date = character_birth_date();
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
    state->player_light = player_has_light();
    state->find_flag = running_steps();
    state->free_turn_flag = free_turn_flag;
    state->weapon_heavy = weapon_is_too_heavy();
    state->pack_heavy = pack_speed_penalty();
    state->doing_inven = pending_inven_command();
    state->screen_change = screen_was_flushed();
    state->eof_flag = input_end_count();
    state->noscore = score_disqualifications();
    state->panic_save = is_panic_save();
    state->wait_for_more = msg_at_more_prompt();
    state->closing_flag = closing_flag;

    // Dungeon dimensions
    state->cur_height = dungeon_height();
    state->cur_width = dungeon_width();
    state->max_panel_rows = (int16_t)panel_max_row_index();
    state->max_panel_cols = (int16_t)panel_max_col_index();

    // Temporary
    state->hack_monptr = monster_turn_index();
    state->missile_ctr = missile_serial_value();

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

// Not in core/rnd.c with the rest of the seeding, because it also seeds the
// colors of unknown items and the town (progress.c), and reads the clock.
// gets a new random seed for the random number generator
void init_seeds(uint32_t seed) {
    uint32_t clock_var;

    if (seed == 0) {
        clock_var = (uint32_t)time((time_t *)0);
    } else {
        clock_var = seed;
    }
    progress_set_color_seed(clock_var);

    clock_var += 8762;
    progress_set_town_seed(clock_var);

    clock_var += 113452L;
    set_rnd_seed(clock_var);
    // make it a little more random
    for (clock_var = (uint32_t)randint(100); clock_var != 0; clock_var--) {
        (void)rnd();
    }
}
