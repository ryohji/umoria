// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Whether the screen has been flushed since somebody last said it had not

#ifndef SCREEN_TOUCHED_H
#define SCREEN_TOUCHED_H

// One bit, and it answers one question: has anything been put on the screen
// since the flag was last forgotten? Nobody cares about the screen in general --
// the only reader is the inventory command (moria1.c), which saves the screen,
// hands control back to the rest of moria for a turn, and on the way back in
// needs to know whether what it saved is still what the player is looking at. If
// it is, the inventory screen can be redrawn silently; if it is not, the player
// is asked whether to carry on, because something happened that they should see.
//
// The screen was flushed. Said by put_qio() (io.c), which is the one place that
// pushes the buffer out, and by creature.c when a monster appears or vanishes --
// that redraws a single spot without going through put_qio().
void note_screen_flushed(void);

// Has it? The inventory command asks this when it resumes.
bool screen_was_flushed(void);

// Start counting again from here. Only the inventory command does this, and only
// as part of suspending itself, so it is not called from outside:
// suspend_inven_command() (inven_command_state.h) calls it. The two have to move
// together -- remembering which command to resume while leaving the flag set
// would make the player answer a prompt about a screen that nothing had touched.
void forget_screen_flushed(void);

#endif // SCREEN_TOUCHED_H
