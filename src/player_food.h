// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How full the player's stomach is, and how fast it empties

#ifndef PLAYER_FOOD_H
#define PLAYER_FOOD_H

// One question, one module -- the second question to leave `py`, after the
// purse (player_gold.c). Two fields answer it together and only together:
// py.flags.food is the counter, py.flags.food_digested is how much of it a
// single turn costs, and the one place that reads the rate subtracts it from
// the counter. Thirty-eight places touch the pair.

// Two rules live inside, because each has exactly one reader and each is about
// nothing but the stomach:
//
//   - a turn's digestion is the rate taken off the counter (player_digest),
//   - the first bite forgives a starvation debt: a negative counter becomes
//     empty before the food is added (player_gain_food). The old code did this
//     at the top of add_food().
//
// Everything else stays with its caller, because the reason is outside the
// stomach:
//
//   - the thresholds (PLAYER_FOOD_ALERT / WEAK / FAINT and zero) pick a
//     regeneration rate, raise PY_HUNGRY / PY_WEAK and print, all in
//     dungeon.c, and the flags belong to another question,
//   - eating past PLAYER_FOOD_MAX costs speed and says so (misc1.c): it
//     prints, and it writes another field,
//   - starvation damage is the damage system's (dungeon.c),
//   - a resurrected character is lifted off a negative counter (save.c),
//   - vomiting cuts the counter down to 150 (potions.c).

// How full the stomach is. Compared against the thresholds each turn, written
// to the save file, and shown as "Hungry" / "Weak" by way of the status flags.
int16_t player_food(void);

// Food goes in: a ration eaten, or the nourishment in a potion. Both arrive
// through add_food(), which keeps the overeating penalty to itself.
void player_gain_food(int amount);

// Food goes out for a reason other than the passing of a turn -- being slowed
// burns extra. The counter may go below zero; the caller decides what that
// costs.
void player_burn_food(int amount);

// Puts a number in the stomach outright, replacing what was there: the 7500 a
// new character starts with, the save file's reader, the 150 left after
// vomiting, and the zero a resurrection needs.
void player_set_food(int16_t amount);

// One turn passes: the rate comes off the counter. Nothing is checked here.
void player_digest(void);

// How much one turn costs. Only the save file asks.
int16_t player_digestion(void);

// Sets the rate outright: the 2 a new character starts with, and the save
// file's reader.
void player_set_digestion(int16_t amount);

// Changes the rate by a step, in either direction. Wearing a ring of slow
// digestion or of regeneration moves it (and calc_bonuses() undoes the move
// before recalculating), and so do searching and resting.
void player_adjust_digestion(int delta);

#endif
