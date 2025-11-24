// Copyright (c) 2025 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Event queue implementation

#include "event_queue.h"
#include <string.h>

// Initialize event queue to empty state
void event_queue_init(EventQueue *queue) {
    if (queue == NULL) {
        return;
    }

    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;

    // Clear all events
    memset(queue->events, 0, sizeof(queue->events));
}

// Add an event to the queue
bool event_queue_push(EventQueue *queue, const GameEvent *event) {
    if (queue == NULL || event == NULL) {
        return false;
    }

    // Check if queue is full
    if (queue->count >= EVENT_QUEUE_SIZE) {
        return false;
    }

    // Copy event to tail position
    queue->events[queue->tail] = *event;

    // Advance tail (circular buffer)
    queue->tail = (queue->tail + 1) % EVENT_QUEUE_SIZE;
    queue->count++;

    return true;
}

// Clear all events from queue
void event_queue_clear(EventQueue *queue) {
    if (queue == NULL) {
        return;
    }

    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
}

// Check if queue is empty
bool event_queue_is_empty(const EventQueue *queue) {
    if (queue == NULL) {
        return true;
    }

    return queue->count == 0;
}

// Get number of events in queue
int event_queue_count(const EventQueue *queue) {
    if (queue == NULL) {
        return 0;
    }

    return queue->count;
}

// Get next event from queue (remove it)
bool event_queue_pop(EventQueue *queue, GameEvent *out_event) {
    if (queue == NULL || out_event == NULL) {
        return false;
    }

    // Check if queue is empty
    if (queue->count == 0) {
        return false;
    }

    // Copy event from head position
    *out_event = queue->events[queue->head];

    // Advance head (circular buffer)
    queue->head = (queue->head + 1) % EVENT_QUEUE_SIZE;
    queue->count--;

    return true;
}

// Peek at next event without removing it
bool event_queue_peek(const EventQueue *queue, GameEvent *out_event) {
    if (queue == NULL || out_event == NULL) {
        return false;
    }

    // Check if queue is empty
    if (queue->count == 0) {
        return false;
    }

    // Copy event from head position without removing
    *out_event = queue->events[queue->head];

    return true;
}

