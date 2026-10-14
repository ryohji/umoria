// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How well this character swings and shoots before anything is added

#ifndef PLAYER_BASE_TO_HIT_H
#define PLAYER_BASE_TO_HIT_H

// THE QUESTION. How good is this character at hitting things, before the weapon,
// the level and the strength of the arm are counted? The unit is the same one
// test_hit() weighs against a target's armour class, and BIGGER MEANS MORE LIKELY
// TO HIT. The range the game itself admits to is 0 to 200 (wizard.c asks for a
// number in that span); a Human Warrior starts at 70 swinging and 55 shooting, a
// Human Mage at 34 and 20. NEGATIVE IS POSSIBLE AND REAL: an Elf's racial base is
// -5 for swinging and +15 for shooting, so an Elven Mage begins at 29.
//
// TWO NUMBERS, BOTH OF THEM ANSWERS:
//
//   - THE MELEE NUMBER is used when the character swings what is wielded.
//   - THE BOWS NUMBER is used when something is fired or thrown, and it is the
//     one the game leans on more heavily -- throw.c picks it out again for every
//     kind of launcher.
//
// THE MIRROR IMAGE OF THE ARMOUR CLASS (player_armour_class.h). There, two fields
// held one answer: every reader added them up and no reader wanted a half. Here,
// NOT ONE READER ADDS THEM UP -- a reader always knows which of the two it wants,
// because it already knows whether a blow or a shot is being tested.
//
// SO WHY ONE MODULE AND NOT TWO? Because the WRITERS never separate. All eight
// places that write do it in pairs, and six of the eight pairs hand both numbers
// the very same amount: heroism is worth 12 to each, super heroism 24 to each, a
// blessing 5 to each. A race gives both, a class adds to both, the saved file
// carries both, and the wizard screen is the only caller in the game that ever
// changes one without the other. Two questions that share every way of changing
// are one question with two answers.
//
// WHO ASKS: the four places a to-hit roll is made. player_melee.c for a swing (twice,
// because an unlit target is harder to hit), throw.c for a shot or a throw (seven
// times, once per launcher), abilities.c for the two ratings on the character
// sheet, and the saved file.

// The two answers. FIVE CALLERS FOR THE MELEE NUMBER (abilities.c's rating,
// player_melee.c's two branches, the wizard screen, the saved file) and TEN FOR THE BOWS
// NUMBER (abilities.c, throw.c's seven, the wizard screen, the saved file).
//
// Both are ints, as `p_ptr->bth` always was once C had widened it: every caller
// puts the number straight into a larger sum.
int player_base_to_hit(void);
int player_base_to_hit_with_bows(void);

// Both numbers outright. TWO CALLERS: create.c's race table, and the saved file's
// two shorts put back.
//
// ONE WINDOW FOR THE PAIR, because neither caller has ever set one alone -- the
// race hands over both bases in two adjacent lines, and the file holds them in two
// adjacent shorts (melee first, and that order cannot move).
//
// UNLIKE THE ARMOUR CLASS, the file's restore and the race's base ARE the same
// sentence here, so this one window serves both: nothing is wiped and nothing is
// implied, the two numbers are simply replaced. That is the third answer to the
// question the fifteenth unit raised, and it agrees with the hit die rather than
// with the armour class.
void player_base_to_hit_set(int melee, int with_bows);

// One number alone. ONE CALLER EACH, both of them the wizard screen, which shows
// the melee number, takes a new one, and only then moves on to the bows number.
//
// THESE TWO WINDOWS ARE NOT SHORTHAND for player_base_to_hit_set() with the other
// number read back: the old code wrote one field per prompt, so the window writes
// one number per prompt. THE 0 TO 200 RANGE IS THE WIZARD SCREEN'S OWN and stays
// there; these windows refuse nothing, because the fields refused nothing.
void player_base_to_hit_set_melee(int melee);
void player_base_to_hit_set_with_bows(int with_bows);

// The class's worth added to whatever the race left here. ONE CALLER, create.c,
// for the melee and the bow amounts alike.
//
// The two amounts differ (a Warrior's class is worth 70 to swinging and 55 to
// shooting), which is why this window takes two numbers where the next one takes
// one.
void player_base_to_hit_adjust(int melee, int with_bows);

// This many more to BOTH numbers, or fewer if it is negative. SIX CALLERS, all of
// them spells:
//
//   - heroism, 12 the turn it begins and -12 the turn it runs out;
//   - super heroism, 24 and -24;
//   - a blessing, 5 and -5.
//
// The reason this is one window: no spell in the game has ever moved one of the
// two numbers by a different amount than the other.
//
// THE DOUBLE BOOKKEEPING IS THE CALLER'S, the same shape as the armour class and
// the infravision before it: whether a spell is in force, and whether this is the
// turn it began or ended, is what player_timed_effects.c answers. This window only
// moves the numbers.
void player_base_to_hit_adjust_both(int amount);

// WHAT THIS MODULE DOES NOT ANSWER -- four things, all still in the callers:
//
//   1. WHETHER A BLOW LANDS. test_hit(int bth, ...) in hit_rolls.c does that, and ITS
//      FIRST ARGUMENT HAS THE SAME NAME AS THIS QUESTION but is only whatever the
//      caller handed over -- a monster's own to-hit goes through the same door.
//   2. WHAT THE CHARACTER SHEET SAYS. abilities.c adds the weapon bonus times
//      BTH_PLUS_ADJ and the class's per-level column on top of these numbers, and
//      likert() turns the total into words. That sum is the sheet's question.
//   3. THE BARE-HANDED AND UNLIT PENALTIES. player_melee.c halves the melee number and
//      subtracts when the target cannot be seen; throw.c takes 75 percent of the
//      bows number for a thing merely thrown. Each of those rules has exactly one
//      reader, and it stays with that reader.
//   4. HOW LONG A SPELL LASTS. player_timed_effects.c keeps the clocks.

#endif // PLAYER_BASE_TO_HIT_H
