// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Whether the input has run out

#ifndef INPUT_ENDED_H
#define INPUT_ENDED_H

// The player's keystrokes can stop coming: the terminal is gone (HANGUP), or the
// input was a file or a pipe that has been read to its end. Every read then
// returns EOF at once, so the game can no longer ask anybody anything -- it has
// to save what it can and leave. That is one fact about the whole game, not about
// any one prompt, which is why it is kept here and read in four other places: the
// main loop and the turn loop stop (dungeon.c), the top level saves the character
// and ends the game (main.c), the save file is written with the death flag
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

// How many EOFs, for the game state snapshot only (game_state.c copies the old
// global's value). Nothing decides anything from this -- the two questions above
// are what the game asks.
int input_end_count(void);

#endif // INPUT_ENDED_H
