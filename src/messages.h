// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The messages shown on the top line: their rolling history -CJS-, and whether
// the one up there now has been seen by the player.

#ifndef MESSAGES_H
#define MESSAGES_H

// The history is a ring buffer. Three files used to walk it, each with its own
// copy of the wrap-around rule: msg_print() advanced the index, the ^P command
// stepped backwards through it, and the save file dumped the raw slots. The
// rule now lives here only.

// --- newest-first view: for whoever wants to read the messages back ---

// `back` counts backwards from the newest message: 0 is the one on screen now,
// 1 the one before it. Wraps around, so asking further back than the history
// holds returns an old message rather than reading outside the ring.
const char *msg_history_recent(int back);

// Adds a message as the newest one, dropping the oldest. Truncates to the slot
// size.
void msg_history_push(const char *message);

// Joins a message onto the newest one, two spaces between them, the way
// msg_print() shows two short messages on one line. Truncates rather than
// writing past the slot; the caller (io.c) only appends what it knows fits.
void msg_history_append(const char *message);

// --- storage view: for the save file, whose format is the raw ring ---

// Number of slots, and which slot currently holds the newest message. The save
// file stores every slot in storage order plus this index, so both are needed
// to write and to read one.
int msg_history_slot_count(void);
char *msg_history_slot(int slot);
int msg_history_newest_slot(void);
void msg_history_set_newest_slot(int slot);

// The whole block at once. Only for game_state.c, whose snapshot holds a
// pointer to the history rather than a copy of it. Walking the result by hand
// is what this module exists to stop -- use msg_history_recent() to read.
vtype *msg_history_slots(void);

// --- the top line itself, and its -more- prompt ---

// True while the top line holds a message the player has not acknowledged.
// msg_print() asks for -more- before letting anything overwrite one, and
// erase_line()/clear_screen() flush it rather than wiping it unseen.
bool msg_pending(void);

// msg_print() decides this for the message it prints. Others only ever clear
// it, meaning "the player has seen what is up there": both command loops
// (dungeon.c, store2.c) do so before reading a key, and inkey() does so on EOF,
// where nobody is left to answer a -more- prompt.
void msg_set_pending(bool pending);

// True while msg_print() is stopped at a -more- prompt. The interrupt handler
// reads it: if the player cancels a *Suicide* the prompt has to be put back.
bool msg_at_more_prompt(void);
void msg_set_at_more_prompt(bool waiting);

#endif // MESSAGES_H
