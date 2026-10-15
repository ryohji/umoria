// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Declarations shared inside ui/ only.

#ifndef UI_INTERNAL_H
#define UI_INTERNAL_H

#include "config.h"
#include "constant.h"
#include "types.h"
#include <stdbool.h>
#include "backend.h"
#include <stdint.h>

// --- messages.c ---
// Adds a message as the newest one, dropping the oldest. Truncates to the slot
// size.
void msg_history_push(const char *message);

// Joins a message onto the newest one, two spaces between them, the way
// msg_print() shows two short messages on one line. Truncates rather than
// writing past the slot; the caller (io.c) only appends what it knows fits.
void msg_history_append(const char *message);
void msg_set_at_more_prompt(bool waiting);

// --- panel.c ---
// Two indexes say which panel we are looking at; six dungeon coordinates say
// what that panel covers. The six are derived from the two, and only this
// module knows how the six follow from the two.

// The map does not fill the terminal: the message line is above it and the
// status column to its left. prt_map() draws from here, and the two print
// offsets convert dungeon coordinates into that area, so both have to agree.
#define PANEL_MAP_TOP_ROW 1

int panel_screen_col(int dungeon_col);

// --- screen_touched.c ---
// Start counting again from here. Only the inventory command does this, and only
// as part of suspending itself, so it is not called from outside:
// suspend_inven_command() (ui_input.h) calls it. The two have to move
// together -- remembering which command to resume while leaving the flag set
// would make the player answer a prompt about a screen that nothing had touched.
void forget_screen_flushed(void);

// --- input.c ---
// High-level input functions

// Get a key (blocking)
// Returns the key code, or KEY_NONE if no input
int input_get_key(void);

// Check if input is available
// microsec: timeout in microseconds (0 = immediate, -1 = wait forever)
bool input_check_available(int microsec);

// Flush input buffer
void input_flush(void);

// --- input_ended.c ---
// The player's keystrokes can stop coming: the terminal is gone (HANGUP), or the
// input was a file or a pipe that has been read to its end. Every read then
// returns EOF at once, so the game can no longer ask anybody anything -- it has
// to save what it can and leave. That is one fact about the whole game, not about
// any one prompt, which is why it is kept here and read in four other places: the
// main loop and the turn loop stop (app/dungeon.c), the top level saves the character
// and ends the game (app/main.c), the save file is written with the death flag
// cleared so the tombstone can still be seen on restart (save.c), and flush()
// stops throwing away input there is none of (io.c).
//
// There is no way back. Nothing in the game clears this, because input that has
// ended does not come back -- so this module has no window for "input is fine
// again" and never had a reason to.
//
// The old global was called eof_flag but was an int that counted, and the count
// mattered in exactly one place (see input_end_is_hopeless below). The name said
// less than the thing did, so the count is named here and kept inside.

// Another read came back EOF. Said by inkey() and by check_input() (io.c), every
// time it happens, not only the first time -- the count is what tells a single
// hangup from an input that keeps handing back EOF while the game tries to carry
// on.
void note_input_ended(void);

// --- render.c ---
// High-level rendering functions (used by game code)
// These queue rendering commands to be executed at end_frame

void render_clear(void);
void render_erase_line(int row, int col);
void render_clear_from(int row);
void render_refresh(void);  // Force immediate screen update

void render_present(void);  // Flush all queued commands and present frame

void render_bell(void);  // Audio or visual bell

// Screen save/restore
void render_save_screen(void);
void render_restore_screen(void);

// Utility functions
void render_get_size(int *rows, int *cols);

// --- view_observer.c ---
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

#endif // UI_INTERNAL_H
