// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Public interface of ui/: ui_input.

#ifndef UI_INPUT_H
#define UI_INPUT_H

#include "config.h"
#include "constant.h"
#include "types.h"
#include <stdbool.h>
#include "backend.h"

// --- command_state.c ---
// What the game remembers about the command being typed
// Three things are remembered between commands, and all three exist because a
// command can be given a repeat count ("#" or a digit, then the command).
//
//   1. How many more times the command is to be repeated.
//   2. Whether the direction should be taken from memory instead of asked for.
//   3. Which command was typed last.
//
// Only the main loop in app/dungeon.c drives all three; the other files ask one
// question each.

// --- The repeat count ------------------------------------------------------

// A count has been typed and the command is about to run that many times. Said
// by app/dungeon.c once it knows the command accepts a count.
void begin_command_count(int count);

// Is a repeat in progress? Asked in five places that have nothing to do with
// counting: the main loop (an interruption is looked for), the status line (it
// shows "Repeat N"), a stuck door (it stays silent rather than saying "The door
// holds firm." once per try), the "^" prefix, and resting.
bool command_is_repeating(void);

// How many times are left. For showing the number ("Repeat %-3d"), for the game
// state snapshot, and for the one caller that compares it with something other
// than zero (app/dungeon.c asks for more than one when '.' turns into a rest).
int command_count_remaining(void);

// Take the whole count and end the repeat. The five commands that do this use
// the count as a number rather than as a number of tries -- how many messages to
// look back through (^P), how many objects to make (^G), which level to go to
// (^D), how much experience to set ('+'), and how many turns to rest. **The
// clamps stay with them**: ^P stops at MAX_SAVE_MSG and ^D treats more than 99
// as zero, and those limits are about messages and dungeon depth, not about
// counting (the same choice as the bounds in level_exit.h).
int take_command_count(void);

// One of the repeats is done. Said by the main loop after the command has run.
void consume_command_count(void);

// Whatever was being repeated, stop. Said by nine places, and they are all the
// same thing: a key was read (io.c), a message was printed (io.c), the player
// was disturbed (rest_command.c), a run could not start (run_path.c), a door came open
// (terrain_commands.c), the turn was free or the run took the count over (app/dungeon.c), and
// **a new level has begun** (app/dungeon.c). The last one is a real cancelling, not
// a fresh start: a count can outlive a staircase, and this is what stops it.
// That is why there is no separate "forget" window here, unlike player_condition.h.
void cancel_command_count(void);

// Hold the count across something that would cancel it, then put it back. Three
// places do this, and the original said why in a comment each time: printing a
// message must not "interrupt a counted command" (io.c), and asking for a
// direction must not "end a counted command" (direction.c, app/dungeon.c).
//
// The held value is handed back to the caller rather than kept here, because
// **the two outer callers nest**: app/dungeon.c holds the count while it turns '-'
// into a movement command, and the get_dir() it calls holds it again.
int hold_command_count(void);
void resume_command_count(int held);

// --- The direction ---------------------------------------------------------

// Should the remembered direction be used instead of asking for one? Asked by
// get_dir() (direction.c), which is the only reader. **The direction itself is not
// kept here** -- it is a static of get_dir(), next to the prompt that reads it,
// the same as find_direction in run_path.c.
bool direction_is_remembered(void);

// The two answers, both said by the main loop. A command that came from the
// count reuses the direction; a command that came from the keyboard asks again.
// **This is not the same question as "is a repeat in progress"**: the first run
// of a counted command has a count but no remembered direction yet, which is
// where the direction gets asked for and remembered.
void reuse_remembered_direction(void);
void ask_for_direction_again(void);

// --- The command before this one -------------------------------------------

// Remember the command that has just run. Said by app/dungeon.c at the end of every
// command.
void note_command(char command);

// Was the command before this one the given key? The only two readers both ask
// it about themselves: pressing ^P twice in a row shows the whole message
// history rather than one line, and pressing 'V' twice keeps the score screen
// from being redrawn. **The character itself never leaves the module** -- nobody
// needs to know what the previous command was, only whether it was this one.
bool previous_command_was(char command);

// --- direction.c ---
bool get_dir(const char *, int *);
bool get_alldir(const char *, int *);

// --- direction_keys.c ---
// Map direction numbers to command keys.
typedef enum {
    DIR_KEY_WALK,
    DIR_KEY_RUN,
    DIR_KEY_TUNNEL
} direction_key_style;

// Returns the command key for the given direction (1-4, 6-9) and style.
// Returns 0 for any other direction (including 5).
char direction_command_key(int dir, direction_key_style style);

// The other way round: the direction (1-4, 6-9) of a walk, run or tunnel
// key, with its style in *style. Returns 0 for any other key.
int command_key_direction(char key, direction_key_style *style);

// --- input.c ---
// Input abstraction layer
// Provides a device-independent interface for user input
// Initialize input system with a specific backend
bool input_init(InputBackend *backend);

// Shutdown input system
void input_shutdown(void);

// --- input_ended.c ---
// Whether the input has run out
// Has the input ended? True from the first EOF onwards. This is the question all
// four readers outside io.c ask.
bool input_has_ended(void);

// Has it gone on so long that the game must stop trying? inkey() returns ESCAPE
// after each EOF, which lets the caller's prompt fall through, and the game may
// well come back and ask again -- so a run of EOFs can loop. After enough of them
// io.c gives up: it panic-saves, and dies even if that save fails.
//
// The number of tries lives here rather than at the call site (the opposite of
// the clamps in level_exit.h) because it is not a rule about what io.c is doing
// at that moment; it is the only reason the ending is counted at all, and there
// is one caller. The original wrote it as "eof_flag > 100", "just in case, to
// make sure that the process eventually dies".
bool input_end_is_hopeless(void);

// How many EOFs, for the game state snapshot only (app/game_state.c copies the old
// global's value). Nothing decides anything from this -- the two questions above
// are what the game asks.
int input_end_count(void);

// --- inven_command_state.c ---
// Which inventory command is waiting to be resumed, if any
// The inventory commands (wear, exchange, take off, drop, inventory, equipment)
// run in a command input mode of their own, and some of them cost a turn. So
// inven_command() (inven_menu.c) is meant to be called again and again, with the
// rest of moria running in between: it returns after spending the turn and says
// "call me again with this character". That is what this module remembers.
//
// The character to resume with, or 0 for "nothing is waiting". The main loop
// (app/dungeon.c) asks every turn, and the store (store_ui.c) loops on it so that
// inventory commands work while haggling.
char pending_inven_command(void);

// Stop for now, and resume with this command next time. ' ' is a real answer,
// not "nothing": it means "call me again just to put the inventory screen back",
// which is why the test for "waiting" is against 0 and not against blank.
//
// This also forgets that the screen was flushed (ui_screen.h). The two are
// one act, not two: the flag means "something has been drawn since we suspended
// ourselves", so it has to start from clear at the moment we suspend. Setting
// one without the other is the bug this window exists to prevent -- leave the
// flag set and the player is asked "Continuing with inventory command?" about a
// screen nothing has touched; forget the flag somewhere else and a monster
// walking into view goes unnoticed.
//
// Call it after the last message has been flushed. msg_print() puts text on the
// screen, which notes a flush of its own, so flushing first and suspending after
// is the order that leaves the flag clear (inven_menu.c does exactly this).
void suspend_inven_command(char command);

// Done -- nothing to resume. Says nothing about the screen.
void finish_inven_command(void);

#endif // UI_INPUT_H
