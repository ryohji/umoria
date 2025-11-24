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

