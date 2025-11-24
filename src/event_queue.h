// Copyright (c) 2025 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Event queue system for game events (combat, damage, effects, etc.)
// Decouples event generation from message display and rendering

#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include "constant.h"
#include "types.h"
#include <stdbool.h>
#include <stdint.h>

// Maximum number of events that can be queued
#define EVENT_QUEUE_SIZE 128

// Event types
typedef enum {
    EVENT_NONE = 0,
    EVENT_ATTACK_HIT,      // Monster/player hit attack
    EVENT_ATTACK_MISS,     // Monster/player miss attack
    EVENT_DAMAGE,          // Damage dealt
    EVENT_HEAL,            // Healing applied
    EVENT_DEATH,           // Monster/player death
    EVENT_LEVEL_UP,        // Player level up
    EVENT_GAIN_EXP,        // Experience gained
    EVENT_STATUS_EFFECT,   // Status effect applied (poison, blind, etc.)
} EventType;

// Game event structure
typedef struct GameEvent {
    EventType type;
    int32_t source_id;    // Entity that caused the event (-1 for player)
    int32_t target_id;    // Entity affected by the event (-1 for player)
    int32_t value;        // Numeric value (damage, healing, exp, etc.)
    vtype message;        // Associated message text
} GameEvent;

// Event queue state
typedef struct EventQueue {
    GameEvent events[EVENT_QUEUE_SIZE];  // Circular buffer
    int head;                             // Index of first event
    int tail;                             // Index after last event
    int count;                            // Number of events in queue
} EventQueue;

// Initialize event queue
void event_queue_init(EventQueue *queue);

// Add an event to the queue
bool event_queue_push(EventQueue *queue, const GameEvent *event);

// Get next event from queue (remove it)
bool event_queue_pop(EventQueue *queue, GameEvent *out_event);

// Peek at next event without removing it
bool event_queue_peek(const EventQueue *queue, GameEvent *out_event);

// Clear all events from queue
void event_queue_clear(EventQueue *queue);

// Check if queue is empty
bool event_queue_is_empty(const EventQueue *queue);

// Get number of events in queue
int event_queue_count(const EventQueue *queue);

#endif  // EVENT_QUEUE_H
