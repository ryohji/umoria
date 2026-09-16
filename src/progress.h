// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How far the game has got: the turn counter, the two seeds that let a run be
// reproduced, and whether this run is a wizard run.

#ifndef PROGRESS_H
#define PROGRESS_H

// This is storage and only storage. The window does not range-check or clamp,
// and it does not decide anything on the caller's behalf, because the old code
// did neither and doing it here would change behaviour rather than preserve it.

// --- the turn counter ----------------------------------------------------

// The current turn. Starts at -1, which is not a turn at all: a negative turn
// means no character is in play. The main loop raises it to 0 on its first
// pass, so 0 is a real turn and `> 0` and `>= 0` are not the same test. The
// callers that ask about the sign do their own comparing for that reason --
// see save_state.h, which is where the sign is given a name.
int32_t progress_turn(void);

// Puts the counter anywhere, including back to a negative. Used when a run
// ends or is loaded, and by the save file's reader.
void progress_set_turn(int32_t turn);

// One turn passes. The main loop is the only caller.
void progress_advance_turn(void);

// --- the seeds -----------------------------------------------------------

// The seed the object colours and the town layout are generated from. Kept so
// that a saved game rebuilds the same world it was saved from.
uint32_t progress_color_seed(void);
void progress_set_color_seed(uint32_t seed);

uint32_t progress_town_seed(void);
void progress_set_town_seed(uint32_t seed);

// --- wizard mode ---------------------------------------------------------

// True once the player has actually entered wizard mode.
bool progress_wizard_mode(void);
void progress_set_wizard_mode(bool on);

// True when wizard mode was asked for on the command line but has not been
// granted yet. Startup turns this into progress_set_wizard_mode().
bool progress_wizard_requested(void);
void progress_set_wizard_requested(bool requested);

#endif // PROGRESS_H
