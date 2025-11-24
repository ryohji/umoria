// Copyright (c) 2025 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Message queue implementation

#include "message_queue.h"
#include <string.h>

// Initialize message queue to empty state
void message_queue_init(MessageQueue *queue) {
    if (queue == NULL) {
        return;
    }

    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
    queue->display_index = 0;
    queue->waiting_for_more = false;

    // Clear all message buffers
    for (int i = 0; i < MESSAGE_QUEUE_SIZE; i++) {
        queue->messages[i][0] = '\0';
    }
}

// Add a message to the queue (non-blocking)
bool message_queue_push(MessageQueue *queue, const char *msg) {
    if (queue == NULL || msg == NULL) {
        return false;
    }

    // Check if queue is full
    if (queue->count >= MESSAGE_QUEUE_SIZE) {
        return false;
    }

    // Copy message to tail position
    strncpy(queue->messages[queue->tail], msg, VTYPESIZ - 1);
    queue->messages[queue->tail][VTYPESIZ - 1] = '\0';

    // Advance tail (circular buffer)
    queue->tail = (queue->tail + 1) % MESSAGE_QUEUE_SIZE;
    queue->count++;

    return true;
}

// Clear all messages from queue
void message_queue_clear(MessageQueue *queue) {
    if (queue == NULL) {
        return;
    }

    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
    queue->display_index = 0;
    queue->waiting_for_more = false;
}

// Check if queue has messages to display
bool message_queue_has_pending(const MessageQueue *queue) {
    if (queue == NULL) {
        return false;
    }

    return queue->display_index < queue->count;
}

// Check if waiting for user input
bool message_queue_is_waiting(const MessageQueue *queue) {
    if (queue == NULL) {
        return false;
    }

    return queue->waiting_for_more;
}

// Update queue state based on input (handle -more- prompt)
void message_queue_update(MessageQueue *queue, int key) {
    if (queue == NULL) {
        return;
    }

    // If waiting for more, check if user pressed space/enter
    if (queue->waiting_for_more) {
        if (key == ' ' || key == '\n' || key == '\r') {
            queue->waiting_for_more = false;
            queue->display_index++;
        }
        return;
    }

    // If there are more messages to display, set waiting flag
    if (queue->display_index < queue->count) {
        queue->waiting_for_more = true;
    }
}

