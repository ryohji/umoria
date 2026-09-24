// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// What is remembered about the character's light: whether one is burning, and
// whether its glow is currently laid down on the map

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

// Whether the glow of that light is currently laid down on the map: the ring of
// cave[][].tl bits around the character.
//
// This is a *different* question from the one above, even though the two are
// read side by side in move_light() (moria1.c). One says "a light is burning",
// the other says "its glow is on the map right now". They come apart in two
// ways: a burning light lays down no glow while the character is running with
// find_prself off (the glow would flicker across the screen), and a character
// who has just gone blind still has last turn's glow on the map to erase.
//
// Only the two halves of move_light() write it, and only they and find_init()
// (moria2.c) read it, which is why it was never worth deriving: the answer is
// "what the screen was left looking like", and nothing else knows that.
bool player_light_is_drawn(void);
void set_player_light_drawn(bool drawn);

#endif // PLAYER_LIGHT_H
