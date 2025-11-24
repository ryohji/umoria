// Copyright (c) 2025 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Game loop implementation

#include "game_loop.h"
#include "externs.h"
#include "render.h"
#include <string.h>

// Initialize game loop
void game_loop_init(GameLoop *loop, GameState *state) {
    if (loop == NULL || state == NULL) {
        return;
    }

    memset(loop, 0, sizeof(GameLoop));

    loop->state = state;
    message_queue_init(&loop->message_queue);
    event_queue_init(&loop->event_queue);
    loop->awaiting_player_input = true;
    loop->last_command = 0;
}

// Check if game loop should continue
bool game_loop_should_continue(const GameLoop *loop) {
    if (loop == NULL || loop->state == NULL) {
        return false;
    }

    // Continue if not dead and no new level requested
    return !loop->state->death && !loop->state->new_level_flag;
}

// Update game state (no rendering)
void game_loop_update(GameLoop *loop, int input_key) {
    if (loop == NULL) {
        return;
    }

    // Handle message queue updates first (process -more- prompts)
    if (message_queue_is_waiting(&loop->message_queue)) {
        message_queue_update(&loop->message_queue, input_key);
        return;
    }

    // If awaiting player input and we have input, process it
    if (loop->awaiting_player_input && input_key != 0) {
        loop->last_command = input_key;
        loop->awaiting_player_input = false;
        // TODO: Execute player command (will call existing do_command)
        // This will generate events in event_queue
    }

    // Process queued events into messages
    // TODO: Convert events to messages

    // Update turn counter
    // TODO: Increment turn and update game state

    // Process creature AI and actions
    // TODO: Call creatures() which will generate events

    // Set awaiting input flag for next turn
    loop->awaiting_player_input = true;
}

// Draw game state (no logic updates)
void game_loop_draw(const GameLoop *loop) {
    if (loop == NULL) {
        return;
    }

    // Begin frame
    render_begin_frame();

    // Draw dungeon map (uses existing draw_cave from misc1.c)
    draw_cave();

    // Draw player stats (uses existing put_stats from misc1.c)
    put_stats();

    // Draw message queue current message
    if (message_queue_has_pending(&loop->message_queue)) {
        // TODO: Implement message rendering
    }

    // End frame and present
    render_end_frame();
}

