// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The player's mana: what is left, what it can reach, and the part of a point
// that has not finished coming back yet

#ifndef PLAYER_MANA_H
#define PLAYER_MANA_H

// One question, one module -- the fourth question to leave `py`, after the
// purse (player_gold.c), the stomach (player_food.c) and the four numbers the
// character sheet shows (player_display_numbers.c). Three numbers answer it
// together: what is left, what it can reach and the part of a point still on
// its way back. They were py.misc.cmana, py.misc.mana and py.misc.cmana_frac,
// touched from seventy-one places in ten files; since #18-12-4C they are
// statics in player_mana.c and these windows are the only way in.
//
// THE FRACTION IS THE REASON THIS MODULE HAS RULES IN IT. Regeneration gives
// back a tiny amount each turn -- the whole maximum multiplied by a factor of
// a few hundred 65536ths (PLAYER_REGEN_NORMAL is 197), plus a floor of
// PLAYER_REGEN_MNBASE. For a character with ten mana that is well under a
// hundredth of a point a turn, so without somewhere to keep the remainder
// nothing would ever come back. The fraction is that remainder: the integer
// part goes onto what is left, the sixteen low bits are carried, and when the
// carry fills up it becomes one more point. Nobody outside wants to see it --
// only the save file does, because a character put away mid-recovery should not
// lose the progress -- so the arithmetic that touches it belongs here.
//
// Three rules live inside, and each of them is a rule about the fraction:
//
//   - regeneration (player_regenerate_mana) is the fixed-point sum above, plus
//     the guard the old code had for a maximum so large that the short wraps
//     round to negative, plus the stop at the top, WHERE THE FRACTION IS
//     CLEARED ("must set frac to zero even if equal" -- a full store has
//     nothing on its way back),
//   - spending (player_spend_mana) empties the fraction ONLY when it cannot
//     pay in full. An ordinary cast takes the cost off and LEAVES THE FRACTION
//     ALONE, so progress towards the next point survives being spent. Three
//     places did this with the same six lines: casting a spell (magic.c),
//     saying a prayer (prayer.c) and a monster drinking the mana (creature.c),
//   - a new maximum (player_change_max_mana) carries what is left across in
//     proportion, fraction included, because the old maximum and the new one
//     are rarely a whole multiple apart. Going up a level does this.
//
// What the rules deliberately do NOT do is decide anything outside the mana:
//
//   - how fast regeneration runs is the caller's number (hunger, resting and
//     the regeneration flag all move it -- dungeon.c),
//   - what happens when a cast cannot be paid for is the caller's: fainting,
//     the paralysis that lasts five turns per point missing, and the one in
//     three chance of hurting the constitution (magic.c, prayer.c),
//   - what the maximum SHOULD be is worked out from the stat, the class and
//     the level (calc_mana in misc3.c), and raising PY_MANA afterwards belongs
//     to the status flags, which are another question,
//   - printing. prt_cmana() cannot be called from inside a store, so the
//     callers watch for a change themselves (the same arrangement
//     player_display_numbers.c has with PY_ARMOR).

// What is left. Read by the status line, the character sheet, the character
// dump, the save file, the two "can I afford this spell?" questions
// (moria3.c and the casting itself) and the rest-until-recovered test.
int16_t player_mana(void);

// What it can reach. Zero for a character who knows no spells at all, which is
// how misc3.c asks whether a first spell has just been learned.
int16_t player_max_mana(void);

// The part of a point still on its way back. Only the save file asks.
uint16_t player_mana_fraction(void);

// Spends what a spell costs and says how much was actually spent -- the whole
// cost when there was enough, less when there was not. Coming up short empties
// the store (and the fraction with it) rather than going negative; the caller
// decides what being short costs, and can tell from the return value.
int player_spend_mana(int cost);

// One turn of coming back, at the caller's rate (a factor in 65536ths).
// Everything the fraction needs is in here.
void player_regenerate_mana(int percent);

// Fills the store to the top and says whether that changed anything -- a
// potion of restore mana on a character who is already full does nothing and
// is not even identified by it.
bool player_restore_mana(void);

// Sets a new maximum, carrying what is left across in proportion, and says
// whether the maximum moved. A character who had no mana at all starts full.
bool player_change_max_mana(int16_t new_max);

// Leaves the character with no mana at all, maximum included, and says whether
// that changed anything. This is forgetting the last spell, not spending.
bool player_lose_all_mana(void);

// The three parts set outright, one at a time, because the save file's reader
// meets them one at a time and its order cannot move. The wizard command sets
// all three at once.
void player_set_mana(int16_t value);
void player_set_max_mana(int16_t value);
void player_set_mana_fraction(uint16_t value);

#endif
