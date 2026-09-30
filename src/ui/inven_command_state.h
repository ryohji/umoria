// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Which inventory command is waiting to be resumed, if any

#ifndef INVEN_COMMAND_STATE_H
#define INVEN_COMMAND_STATE_H

// The inventory commands (wear, exchange, take off, drop, inventory, equipment)
// run in a command input mode of their own, and some of them cost a turn. So
// inven_command() (inven_menu.c) is meant to be called again and again, with the
// rest of moria running in between: it returns after spending the turn and says
// "call me again with this character". That is what this module remembers.
//
// The character to resume with, or 0 for "nothing is waiting". The main loop
// (dungeon.c) asks every turn, and the store (store_ui.c) loops on it so that
// inventory commands work while haggling.
char pending_inven_command(void);

// Stop for now, and resume with this command next time. ' ' is a real answer,
// not "nothing": it means "call me again just to put the inventory screen back",
// which is why the test for "waiting" is against 0 and not against blank.
//
// This also forgets that the screen was flushed (screen_touched.h). The two are
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

#endif // INVEN_COMMAND_STATE_H
