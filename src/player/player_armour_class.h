// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How hard this character is to hit -- the real armour class

#ifndef PLAYER_ARMOUR_CLASS_H
#define PLAYER_ARMOUR_CLASS_H

// THE QUESTION. How much armour class does this character really have? The unit
// is armour class, and BIGGER MEANS HARDER TO HIT: every reader hands the number
// to test_hit() as the amount an attacker has to beat, or takes it as the
// percentage-like amount that shaves a blow down.
//
// THE THIRD QUESTION OUT OF struct misc, after how deep the character has been
// (player_max_depth.c) and how many faces the hit die has (player_hit_die.c).
// This is the first one of the three that is READ WHILE THE GAME IS PLAYED, and
// it is read a great deal: twenty-six places, more than any single question
// taken out of py.misc so far.
//
// TWO FIELDS, ONE QUESTION. The answer used to live in py.misc.pac ("Total AC")
// and py.misc.ptoac ("Magical AC"), and the split is BY WHERE THE POINTS CAME
// FROM, not by what they mean:
//
//   - THE ARMOUR HALF is what is worn: the `ac` of every worn item added up,
//     plus the temporary points a spell lends (invulnerability 100, blessing 2).
//   - THE MAGICAL HALF is the plusses: the character's dexterity bonus
//     (toac_adj()) plus the `toac` of every worn item.
//
// NOT ONE READER LOOKS AT A HALF. All twenty-six read `pac + ptoac`, so the
// split is invisible to everyone who uses the answer; only the four places that
// build it, and the two shorts in the saved file, can tell the halves apart.
// The proof is in create.c, which fills the halves THE OTHER WAY ROUND when the
// race is chosen (dexterity bonus into the armour half) from when the class is
// chosen (dexterity bonus into the magical half) -- and nothing in the game can
// tell the difference, because both spellings add up to the same number.
//
// WHO ASKS: the monsters. Twenty-one of the twenty-six readers are the lines in
// creature.c where one blow of one monster is tested, one for each kind of
// attack; four more are in traps.c (a trap's dart, a thrown thing); and the
// last one is creature.c's `damage -= (ac * damage) / 200`, which is why a
// character in plate takes less from a blow that did land.

// The answer: both halves added up. TWENTY-SIX CALLERS, which is what this
// module is for -- each of them used to spell out `p_ptr->pac + p_ptr->ptoac`.
//
// The sum is an int rather than an int16_t on purpose. The old expression was an
// int too (C widens both halves before adding them), so nothing has changed; it
// is worth saying because 100 points of invulnerability on top of a heavy suit
// is the one place the halves get large.
int player_armour_class(void);

// One half at a time. THREE CALLERS FOR THE MAGICAL HALF -- the character
// sheet's starting numbers in calc_bonuses() and in create.c, and the saved
// file -- and ONE FOR THE ARMOUR HALF, the saved file.
//
// The sheet is a DIFFERENT QUESTION (player_display_numbers.h): prt_pac() shows
// player_display_ac(), not this number, and the two are deliberately allowed to
// disagree -- an unidentified suit protects the character while the sheet still
// says nothing about it. These two windows exist so that the file keeps its two
// shorts and the sheet keeps its starting value; nothing else needs a half.
int player_armour_class_magical(void);
int player_armour_class_armour(void);

// Start again from nothing worn: the armour half becomes zero and the magical
// half becomes what is handed over (always toac_adj(), the dexterity bonus).
// TWO CALLERS, calc_bonuses() and create.c's class table.
//
// ONE WINDOW RATHER THAN TWO ASSIGNMENTS, because "nothing is worn yet" is one
// sentence: every time the equipment changes, calc_bonuses() wipes both halves
// and adds the worn items back one by one.
void player_armour_class_reset(int magical);

// Both halves outright. TWO CALLERS: the saved file's two shorts put back, and
// create.c's race table, which is the one place that puts the dexterity bonus in
// the armour half instead of the magical one.
//
// SEPARATE FROM player_armour_class_reset(), and this is the second answer to
// the question the fifteenth unit raised (does the save file's restore need a
// window of its own?). For the hit die it did not -- the race's base and the
// file's byte were the same sentence. HERE IT DOES: _reset() carries the rule
// "the armour half is zero", which a restore cannot obey, because the number in
// the file already has the worn armour in it.
void player_armour_class_set_parts(int armour, int magical);

// One worn item's worth: its `ac` into the armour half, its `toac` into the
// magical half. ONE CALLER, the loop in calc_bonuses() that walks the worn
// slots.
//
// The two go together because an item gives both at once, and taking them in one
// window keeps the caller from touching the store twice per slot.
void player_armour_class_add_item(int armour, int magical);

// More armour class by this many, or less if it is negative. SIX CALLERS, all of
// them spells, and ALL OF THEM TOUCH THE ARMOUR HALF -- invulnerability is worth
// 100 and a blessing is worth 2:
//
//   - calc_bonuses() adds them whenever it rebuilds, for as long as the spell is
//     in force;
//   - dungeon.c adds them the turn the spell begins and takes them away the turn
//     it runs out.
//
// THE DOUBLE BOOKKEEPING IS THE CALLER'S, not this module's (the same shape as
// the twelfth unit's infravision). Whether a spell is in force, and whether this
// is the turn it began, is a question player_timed_effects.c answers; this
// window only moves the number.
//
// NOTHING IS REFUSED AND NOTHING IS CLAMPED, because nothing was before: the old
// code was a bare `+=` and a bare `-=`.
void player_armour_class_adjust(int armour);

// WHAT THIS MODULE DOES NOT ANSWER -- four things, all still in the callers:
//
//   1. WHETHER A BLOW LANDS. test_hit() in moria1.c weighs the armour class
//      against the attacker; twenty-five of the twenty-six readers only pass the
//      number to it.
//   2. HOW MUCH A BLOW THAT LANDED HURTS. creature.c's `* damage / 200` is a
//      rule about damage, and its one reader is creature.c.
//   3. WHAT THE SHEET SAYS. player_display_numbers.c keeps that, and it is
//      allowed to differ (see above).
//   4. HOW LONG A SPELL LASTS. player_timed_effects.c keeps the clocks; this
//      module is only told to add 100 or take 2 away.

#endif // PLAYER_ARMOUR_CLASS_H
