// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// What the game remembers about the command being typed

#ifndef COMMAND_STATE_H
#define COMMAND_STATE_H

// Three things are remembered between commands, and all three exist because a
// command can be given a repeat count ("#" or a digit, then the command).
//
//   1. How many more times the command is to be repeated.
//   2. Whether the direction should be taken from memory instead of asked for.
//   3. Which command was typed last.
//
// They were three globals (command_count, default_dir, last_command) sitting in
// different parts of externs.h. Only the main loop in dungeon.c drives all
// three; the other files ask one question each.

// --- The repeat count ------------------------------------------------------

// A count has been typed and the command is about to run that many times. Said
// by dungeon.c once it knows the command accepts a count.
void begin_command_count(int count);

// Is a repeat in progress? Asked in five places that have nothing to do with
// counting: the main loop (an interruption is looked for), the status line (it
// shows "Repeat N"), a stuck door (it stays silent rather than saying "The door
// holds firm." once per try), the "^" prefix, and resting.
bool command_is_repeating(void);

// How many times are left. For showing the number ("Repeat %-3d"), for the game
// state snapshot, and for the one caller that compares it with something other
// than zero (dungeon.c asks for more than one when '.' turns into a rest).
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
// (terrain_commands.c), the turn was free or the run took the count over (dungeon.c), and
// **a new level has begun** (dungeon.c). The last one is a real cancelling, not
// a fresh start: a count can outlive a staircase, and this is what stops it.
// That is why there is no separate "forget" window here, unlike running.h.
void cancel_command_count(void);

// Hold the count across something that would cancel it, then put it back. Three
// places do this, and the original said why in a comment each time: printing a
// message must not "interrupt a counted command" (io.c), and asking for a
// direction must not "end a counted command" (direction.c, dungeon.c).
//
// The held value is handed back to the caller rather than kept here, because
// **the two outer callers nest**: dungeon.c holds the count while it turns '-'
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

// Remember the command that has just run. Said by dungeon.c at the end of every
// command.
void note_command(char command);

// Was the command before this one the given key? The only two readers both ask
// it about themselves: pressing ^P twice in a row shows the whole message
// history rather than one line, and pressing 'V' twice keeps the score screen
// from being redrawn. **The character itself never leaves the module** -- nobody
// needs to know what the previous command was, only whether it was this one.
bool previous_command_was(char command);

#endif // COMMAND_STATE_H
