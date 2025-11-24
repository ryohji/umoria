// Copyright (c) 2025 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Game loop with Update/Draw separation
// Modern game engine architecture for Umoria

#ifndef GAME_LOOP_H
#define GAME_LOOP_H

#include "game_state.h"
#include "message_queue.h"
#include "event_queue.h"
#include <stdbool.h>

// Game loop state
typedef struct GameLoop {
    GameState *state;                    // Global game state
    MessageQueue message_queue;          // Message display queue
    EventQueue event_queue;              // Game event queue
    bool awaiting_player_input;          // Waiting for player command
    int last_command;                    // Last player command
} GameLoop;

// Initialize game loop
void game_loop_init(GameLoop *loop, GameState *state);

// Update game state (no rendering)
void game_loop_update(GameLoop *loop, int input_key);

// Draw game state (no logic updates)
void game_loop_draw(const GameLoop *loop);

// Check if game loop should continue
bool game_loop_should_continue(const GameLoop *loop);

#endif  // GAME_LOOP_H
