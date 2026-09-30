// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How fast the character is moving right now

#ifndef PLAYER_SPEED_H
#define PLAYER_SPEED_H

// One question, one module -- the eleventh question to leave `py`, after the
// purse (player_gold.c), the stomach (player_food.c), the four numbers the
// character sheet shows (player_display_numbers.c), the mana (player_mana.c),
// the hit points (player_hp.c), how far the character has come
// (player_level.c), the status word (player_status_flags.c), what the equipment
// grants (player_abilities.c), the eighteen clocks (player_timed_effects.c) and
// the rest (player_resting.c). ONE field answered it -- py.flags.speed, touched
// from nine places in five files (dungeon.c three of them, misc3.c two,
// save.c two, misc1.c one, moria1.c one).
//
// THE QUESTION IS "HOW MANY STEPS FROM NORMAL SPEED IS THE CHARACTER". Zero is
// normal. It is NOT one of the eighteen clocks and not a count of turns: nothing
// ticks it down, it is ADDED TO AND SUBTRACTED FROM while potions, items and
// traps come and go.
//
// THE SIGN IS BACKWARDS FROM WHAT THE NAME SUGGESTS: POSITIVE IS SLOW.
// prt_speed() (status_line.c) shows "Slow" at 1 and "Very Slow" above it, blank at 0,
// "Fast" at -1 and "Very Fast" below it. A potion of speed calls
// change_speed(-1), an item with TR_SPEED calls change_speed(-amount), and a
// potion of slowness calls change_speed(1). Get the sign wrong and fast and
// slow trade places, so the tests nail the direction from both ends.
//
// What this module deliberately does NOT do:
//
//   - THE MONSTERS. change_speed() (moria1.c) adds the same number to the
//     cspeed of every monster on the level, and monster_place.c mixes it into cspeed
//     when a monster is placed. That is the engine's way of saying "a slowed player is handled by
//     moving the monsters faster instead" (creature.c), so the loop and the sum
//     stay with the callers -- this module holds the player's number only.
//   - THE FOOD. A sped-up character burns extra food, the square of the number
//     of steps (dungeon.c). Squaring it is how speed is turned into hunger,
//     which is a different question (player_food.c).
//   - SEARCH MODE'S ONE STEP. The state line subtracts one step while the
//     character is searching (twice, in prt_speed() and prt_stat_block()).
//     Searching is family 3 of the status word, a separate question, and it is
//     not the other half of this one -- the speed means the same thing whether
//     or not the character is searching.
//   - THE PACK. burden.c remembers how many steps the pack is costing, because
//     check_strength() passes change_speed() the DIFFERENCE (new steps minus
//     remembered steps). The total lives here; what part of it the pack is
//     answerable for lives there.
//   - ANY LIMIT. Nothing in the game checks this number against a range, so
//     neither does the window (the same rule as the mana and the hit points).

// No includes: the callers bring config.h, constant.h and types.h in first, the
// same as player_resting.h and the nine before it.

// How many steps from normal, SIGN AND ALL -- positive is slow, negative is
// fast. Five of the nine places wanted the number itself: the extra food a fast
// character burns (dungeon.c), the state line and whether it is worth drawing
// (status_line.c twice), the number mixed into a new monster's speed
// (monster_place.c) and the save file's writer.
int player_speed(void);

// Faster or slower by this many steps. NEGATIVE MAKES THE CHARACTER FASTER.
// change_speed() (moria1.c) is the only caller -- it adds the same number to
// every monster afterwards, which is why this window does not.
void player_speed_adjust(int num_steps);

// How many steps from normal, as an outright answer rather than a change. The
// save file's reader is the only caller: it puts back whatever the file holds.
void player_speed_set(int num_steps);

#endif
