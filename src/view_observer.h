// Copyright (c) 2025 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// View Observer pattern for Model-View separation
// Allows views to observe game state changes and update accordingly

#ifndef VIEW_OBSERVER_H
#define VIEW_OBSERVER_H

#include <stdbool.h>
#include <stdint.h>

// Forward declarations
typedef struct ViewObserver ViewObserver;

// Type-safe data structures for state changes
// These provide clear semantics and allow future extension

// Player hit points
typedef struct {
    int current;
    int maximum;
} HitPoints;

// Player mana points
typedef struct {
    int current;
    int maximum;
} ManaPoints;

// Player gold
typedef struct {
    int32_t amount;
} Gold;

// Player experience
typedef struct {
    int32_t amount;
} Experience;

// Player level
typedef struct {
    int level;
} PlayerLevel;

// Dungeon depth
typedef struct {
    int depth;
} DungeonDepth;

// Map cell position and display
typedef struct {
    int row;
    int col;
    char character;
} MapCell;

// Game message
typedef struct {
    const char *text;
} GameMessage;

// Game mode identifier
typedef struct {
    int mode;
} GameMode;

// Observer callback function types
// These are called when specific game state changes occur

// Player state changes
typedef void (*OnPlayerHpChangedFn)(void *context, HitPoints hp);
typedef void (*OnPlayerManaChangedFn)(void *context, ManaPoints mana);
typedef void (*OnPlayerGoldChangedFn)(void *context, Gold gold);
typedef void (*OnPlayerExpChangedFn)(void *context, Experience exp);
typedef void (*OnPlayerLevelChangedFn)(void *context, PlayerLevel level);
typedef void (*OnPlayerDepthChangedFn)(void *context, DungeonDepth depth);

// Message system
typedef void (*OnMessageAddedFn)(void *context, GameMessage message);

// Map changes
typedef void (*OnMapCellChangedFn)(void *context, MapCell cell);
typedef void (*OnMapRefreshFn)(void *context);

// Game mode changes
typedef void (*OnModeChangedFn)(void *context, GameMode mode);

// Observer interface
// Views implement these callbacks to receive state change notifications
struct ViewObserver {
    void *context;  // User data (e.g., pointer to view state)

    // Player state callbacks (optional - can be NULL if not interested)
    OnPlayerHpChangedFn on_player_hp_changed;
    OnPlayerManaChangedFn on_player_mana_changed;
    OnPlayerGoldChangedFn on_player_gold_changed;
    OnPlayerExpChangedFn on_player_exp_changed;
    OnPlayerLevelChangedFn on_player_level_changed;
    OnPlayerDepthChangedFn on_player_depth_changed;

    // Message system
    OnMessageAddedFn on_message_added;

    // Map updates
    OnMapCellChangedFn on_map_cell_changed;
    OnMapRefreshFn on_map_refresh;

    // Game mode
    OnModeChangedFn on_mode_changed;
};

// Observer registry functions

// Register a view observer
// Multiple observers can be registered simultaneously
void view_observer_register(ViewObserver *observer);

// Unregister a view observer
void view_observer_unregister(ViewObserver *observer);

// Notification functions
// Game logic calls these when state changes occur

// Player state notifications
void view_notify_player_hp_changed(HitPoints hp);
void view_notify_player_mana_changed(ManaPoints mana);
void view_notify_player_gold_changed(Gold gold);
void view_notify_player_exp_changed(Experience exp);
void view_notify_player_level_changed(PlayerLevel level);
void view_notify_player_depth_changed(DungeonDepth depth);

// Message notifications
void view_notify_message_added(GameMessage message);

// Map notifications
void view_notify_map_cell_changed(MapCell cell);
void view_notify_map_refresh(void);

// Game mode notifications
void view_notify_mode_changed(GameMode mode);

#endif // VIEW_OBSERVER_H
