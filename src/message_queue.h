// Copyright (c) 2025 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Message queue system for non-blocking message display
// Replaces the old blocking msg_print() system with a queue-based approach

#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include "types.h"
#include <stdbool.h>

// Maximum number of messages that can be queued
#define MESSAGE_QUEUE_SIZE 64

// Message queue state
typedef struct MessageQueue {
    vtype messages[MESSAGE_QUEUE_SIZE];  // Circular buffer of messages
    int head;                             // Index of first message
    int tail;                             // Index after last message
    int count;                            // Number of messages in queue
    int display_index;                    // Currently displayed message index
    bool waiting_for_more;                // Waiting for user to press space/-more-
} MessageQueue;

// Initialize message queue
void message_queue_init(MessageQueue *queue);

// Add a message to the queue (non-blocking)
bool message_queue_push(MessageQueue *queue, const char *msg);

// Clear all messages from queue
void message_queue_clear(MessageQueue *queue);

// Update queue state based on input (handle -more- prompt)
void message_queue_update(MessageQueue *queue, int key);

// Check if queue has messages to display
bool message_queue_has_pending(const MessageQueue *queue);

// Check if waiting for user input
bool message_queue_is_waiting(const MessageQueue *queue);

#endif  // MESSAGE_QUEUE_H
