// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How quietly this character moves

#ifndef PLAYER_STEALTH_H
#define PLAYER_STEALTH_H

// THE QUESTION. How quietly does this character walk past something asleep? The
// unit is NOT a chance and NOT a percentage -- it is A NUMBER OF HALVINGS. The one
// place in the game that uses it spells
//
//     notice = randint(1024);
//     if (notice * notice * notice <= (1L << (29 - stl))) { ... the sleeper stirs }
//
// so EVERY POINT HALVES THE CHANCE OF BEING NOTICED and BIGGER MEANS QUIETER. That
// makes this the coarsest of the character's numbers: the disarming skill runs 0
// through 200, this one runs -1 through 18, and one point of it is worth as much as
// the whole of that range.
//
// THE TWELFTH QUESTION OUT OF struct misc, after the purse, how far the character
// has come, how deep they have been, the hit die, the armour class, the base to-hit,
// the disarming skill, the saving throw, the race, the body's weight, the attack
// bonuses, the searching skill and the six answers of the bio. ONE FIELD WITH ONE
// ANSWER -- the simplest shape there is, and the same shape as the hit die and the
// disarming skill. `pclass` WAS THE ONLY FIELD LEFT AFTER THIS ONE, and #18-12-28
// took it and the struct together.
//
// WHERE THE NUMBER COMES FROM -- two tables at creation and then the gear:
//
//   - THE RACE gives the base (race_type.stl): Halfling +4, Gnome +3, Half-Elf and
//     Elf +1, Human 0, Dwarf and Half-Orc -1, Half-Troll -2. SMALL FOLK ARE QUIET
//     AND BIG FOLK ARE LOUD, and this is the only one of the racial bases that is
//     NEGATIVE for three of the eight races.
//   - THE CLASS adds to it (class_type.mstl): Rogue +5, Ranger +3, Mage and Priest
//     +2, Warrior and Paladin +1. EVERY CLASS ADDS SOMETHING, so the lowest a new
//     character can be is -1 (a Half-Troll Warrior) and the highest is 9 (a
//     Halfling Rogue).
//   - WORN GEAR adds and takes back (TR_STEALTH). Three slots can carry it, each
//     worth 1 to 3 points: the Defender weapon, boots of stealth and a cloak of
//     stealth (item_enchant.c:125, :264, :685 -- `t_ptr->p1 = randint(3)` in all three).
//     NINE POINTS OF GEAR ON TOP OF THE NINE FROM CREATION, which is where the
//     wizard screen's "-1-18" comes from: THE PROMPT ADMITS TO EXACTLY THE RANGE
//     THE GAME CAN REACH, which is not true of the other prompts on that screen
//     (searching and disarming both say 0-200 and neither can get near 200).
//
// THE SHIFT HAS EXACTLY AS MUCH ROOM AS IT NEEDS, and that is worth writing down
// because it looks like an overflow waiting to happen. At the bottom of the range
// `1L << (29 - -1)` is `1L << 30`, which is 1,073,741,824 -- and 1024 cubed is
// 1,073,741,824 as well, so a character at -1 IS ALWAYS NOTICED and the cube never
// passes the largest number the shift can make. At the top, `1L << 11` is 2048, so
// only a roll of 1 through 12 stirs the sleeper (12 cubed is 1728 and 13 cubed is
// 2197): 12 chances in 1024. ONE STEP FURTHER DOWN WOULD BE `1L << 31`, which is
// undefined for a signed long -- AND NOTHING CAN TAKE THAT STEP, because the two
// tables cannot add up to less than -1 and no item in the game lowers stealth (the
// noisy curse sets TR_AGGRAVATE instead; see below). This module keeps the number
// as it is; the checking is not its business (ledger observation 24).
//
// WHO ASKS, once #18-12-27B has moved the callers: creation (create.c, race then
// class), the gear (moria1.c), the sleeping monster (creature.c), the character
// sheet and the dumped file by way of the ratings (abilities.c), the wizard screen
// and the saved file. NINE CALLS, against the forty-four of the unit before this
// one -- a whole question can be this small.

// The number. FOUR CALLERS: the ratings block that feeds the character sheet and
// the dump (abilities.c), the sleeping monster (creature.c), the wizard screen's
// prompt and the saved file.
//
// An int, as `p_ptr->stl` always was once C had widened it: two of the four readers
// drop it straight into a larger sum and the other two print it.
int player_stealth(void);

// The number outright. THREE CALLERS: the race's base (create.c), the saved file's
// short put back, and the wizard screen's prompt.
//
// THE SAME SENTENCE FOR ALL THREE, because all three are a plain replacement -- the
// fifth answer to the question the fifteenth unit raised, and it agrees with the hit
// die, the base to-hit and the disarming skill rather than with the armour class.
//
// THE -1 TO 18 RANGE IS THE WIZARD SCREEN'S OWN and stays there. So is the saved
// file's cast: save.c writes `(uint16_t)stl` and reads it back through a
// `(uint16_t *)`, so A NEGATIVE STEALTH SURVIVES THE ROUND TRIP as the same negative
// number, and this window is handed the value that comes out.
void player_stealth_set(int stealth);

// This much quieter -- or louder, when the sign is negative. TWO CALLERS, and they
// are the two that made this unit need a third window at all:
//
//   - THE CLASS at creation (`m_ptr->stl += c_ptr->mstl;`), which is always positive.
//   - THE GEAR (`py.misc.stl += amount;` in py_bonuses()), where `amount` is
//     `t_ptr->p1 * factor` and THE CALLER'S factor IS -1 WHEN THE THING COMES OFF.
//     One line covers putting it on and taking it off, the same as the searching
//     gear and the infra-vision gear.
//
// A window of its own rather than read-add-write, so the store is touched once.
void player_stealth_adjust(int amount);

// WHAT THIS MODULE DOES NOT ANSWER -- five things, all still in the callers:
//
//   1. WHETHER THE SLEEPER STIRS. `notice * notice * notice <= (1L << (29 - stl))`
//      is creature.c's (creature.c:1576), and SO IS THE WHOLE SHAPE OF THE RULE --
//      the cube, the 1024, the 29. This is the ONLY PLACE IN THE GAME that turns
//      stealth into anything, which is why the number itself can be so coarse.
//   2. THE PLUS ONE ON THE CHARACTER SHEET. `a.stl = p_ptr->stl + 1` (abilities.c)
//      so that the quietest sheet word is never the one for zero, and likert()
//      divides by ONE -- the finest divisor on that screen, so EVERY POINT MOVES THE
//      WORD. Both the plus one and the word are the sheet's question (abilities.h).
//   3. THE SIGN THE GEAR PASSES IN. `t_ptr->p1 * factor` is moria1.c's, exactly as
//      it is for the searching gear -- and NOTHING IN THE GAME EVER PASSES A
//      NEGATIVE p1 HERE: the curse that makes a character noisy sets TR_AGGRAVATE,
//      a flag of its own, rather than a negative number of halvings (item_enchant.c:277).
//      THE OPPOSITE OF QUIET IS A DIFFERENT ROAD, not a smaller number on this one.
//   4. THE -1 TO 18 FENCE. wizard.c's prompt, and the only fence there is.
//   5. WHAT TR_AGGRAVATE DOES. `player_aggravates_monsters()` already has a window
//      of its own (#18-7-3), and creature.c asks it FIRST: an aggravating character
//      wakes everything outright and the halvings never get looked at.

#endif // PLAYER_STEALTH_H
