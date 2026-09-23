// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Whether the character is carrying a light that is still burning

#ifndef PLAYER_LIGHT_H
#define PLAYER_LIGHT_H

// This is storage and only storage, the same as progress.h, score_death.h and
// hp_table.h. In particular this is NOT derived from the light source in the
// equipment slot, even though that is where its value comes from.
//
// The main loop (dungeon.c) recomputes it once per turn and the *transitions*
// are what matter: going dark prints "Your light has gone out!", disturbs the
// character and relights every creature on the level; coming back into light
// does the same in reverse. A derived function would read the slot fresh at
// every caller and those transitions would never be noticed, so the answer
// stays a remembered one.
//
// dungeon.c also sets it before the loop starts, from the slot, so the first
// turn does not count as a transition.
bool player_has_light(void);
void set_player_has_light(bool lit);

#endif // PLAYER_LIGHT_H
