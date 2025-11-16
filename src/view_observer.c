// Copyright (c) 2025 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// View Observer implementation

#include "view_observer.h"

#include <stddef.h>

// Maximum number of simultaneous observers
// In practice, we'll typically have 1-2 (main view + maybe debug view)
#define MAX_OBSERVERS 8

// Observer registry
static ViewObserver *observers[MAX_OBSERVERS];
static int observer_count = 0;

// Register an observer
void view_observer_register(ViewObserver *observer) {
    if (observer == NULL) {
        return;
    }

    // Check if already registered
    for (int i = 0; i < observer_count; i++) {
        if (observers[i] == observer) {
            return;  // Already registered
        }
    }

    // Add to registry if space available
    if (observer_count < MAX_OBSERVERS) {
        observers[observer_count++] = observer;
    }
}

// Unregister an observer
void view_observer_unregister(ViewObserver *observer) {
    if (observer == NULL) {
        return;
    }

    // Find and remove
    for (int i = 0; i < observer_count; i++) {
        if (observers[i] == observer) {
            // Shift remaining observers down
            for (int j = i; j < observer_count - 1; j++) {
                observers[j] = observers[j + 1];
            }
            observer_count--;
            return;
        }
    }
}

// Notification implementations
// Each function notifies all registered observers

void view_notify_player_hp_changed(int current_hp, int max_hp) {
    for (int i = 0; i < observer_count; i++) {
        ViewObserver *obs = observers[i];
        if (obs->on_player_hp_changed) {
            obs->on_player_hp_changed(obs->context, current_hp, max_hp);
        }
    }
}

void view_notify_player_mana_changed(int current_mana, int max_mana) {
    for (int i = 0; i < observer_count; i++) {
        ViewObserver *obs = observers[i];
        if (obs->on_player_mana_changed) {
            obs->on_player_mana_changed(obs->context, current_mana, max_mana);
        }
    }
}

void view_notify_player_gold_changed(int32_t gold) {
    for (int i = 0; i < observer_count; i++) {
        ViewObserver *obs = observers[i];
        if (obs->on_player_gold_changed) {
            obs->on_player_gold_changed(obs->context, gold);
        }
    }
}

void view_notify_player_exp_changed(int32_t exp) {
    for (int i = 0; i < observer_count; i++) {
        ViewObserver *obs = observers[i];
        if (obs->on_player_exp_changed) {
            obs->on_player_exp_changed(obs->context, exp);
        }
    }
}

void view_notify_player_level_changed(int level) {
    for (int i = 0; i < observer_count; i++) {
        ViewObserver *obs = observers[i];
        if (obs->on_player_level_changed) {
            obs->on_player_level_changed(obs->context, level);
        }
    }
}

void view_notify_player_depth_changed(int depth) {
    for (int i = 0; i < observer_count; i++) {
        ViewObserver *obs = observers[i];
        if (obs->on_player_depth_changed) {
            obs->on_player_depth_changed(obs->context, depth);
        }
    }
}

void view_notify_message_added(const char *message) {
    for (int i = 0; i < observer_count; i++) {
        ViewObserver *obs = observers[i];
        if (obs->on_message_added) {
            obs->on_message_added(obs->context, message);
        }
    }
}

void view_notify_map_cell_changed(int row, int col, char character) {
    for (int i = 0; i < observer_count; i++) {
        ViewObserver *obs = observers[i];
        if (obs->on_map_cell_changed) {
            obs->on_map_cell_changed(obs->context, row, col, character);
        }
    }
}

void view_notify_map_refresh(void) {
    for (int i = 0; i < observer_count; i++) {
        ViewObserver *obs = observers[i];
        if (obs->on_map_refresh) {
            obs->on_map_refresh(obs->context);
        }
    }
}

void view_notify_mode_changed(int mode) {
    for (int i = 0; i < observer_count; i++) {
        ViewObserver *obs = observers[i];
        if (obs->on_mode_changed) {
            obs->on_mode_changed(obs->context, mode);
        }
    }
}
