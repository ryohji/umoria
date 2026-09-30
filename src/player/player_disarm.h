// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How good this character is at getting traps and locks open

#ifndef PLAYER_DISARM_H
#define PLAYER_DISARM_H

// THE QUESTION. How good is this character at taking a trap apart and at
// picking a lock, before the intelligence, the level and the class are counted?
// The unit is a PERCENTAGE-LIKE CHANCE compared against `randint(100)` once the
// trap's own difficulty has been taken off it, so BIGGER MEANS MORE LIKELY TO
// SUCCEED. The wizard screen admits to 0 through 200; a Human Warrior starts
// near 25 and a Halfling Rogue near 60.
//
// THE FIFTH QUESTION OUT OF struct misc, after how deep the character has been,
// how many faces the hit die has, the armour class and the base to-hit. Unlike
// the two before it this one is A SINGLE FIELD WITH A SINGLE ANSWER -- the
// simplest shape there is, and the same shape as the hit die.
//
// ONE NUMBER, TWO USES, AND THE GAME NEVER SEPARATES THEM: a trap and a lock are
// the same skill here. traps.c uses it on traps and chests, moria3.c on locked
// doors and closed chests, and no caller has ever wanted one without the other.
//
// WHERE THE STARTING NUMBER COMES FROM -- two tables, both at creation:
//
//   - THE RACE gives the base (race_type.b_dis): Human 0, Half-Elf 2, Elf 5,
//     Gnome 10, Halfling 15, and so on -- small folk are better at it.
//   - THE CLASS adds to it (class_type.mdis): Paladin 20, Warrior 25,
//     Priest 25, Mage 30, Ranger 30, Rogue 45.
//
// THE DEXTERITY BONUS IS BAKED IN ONCE AND NEVER AGAIN. create.c stores
// `race[i].b_dis + todis_adj()`, so ONE COPY OF THE CREATION-TIME DEXTERITY
// BONUS IS FROZEN INSIDE THIS NUMBER, while every reader adds two more copies of
// the CURRENT bonus on top. If a potion changes the character's dexterity later,
// the two current copies move and the frozen one does not. That is how the game
// has always behaved; this module keeps the number, it does not tidy the rule.
//
// WHO ASKS: the three places a lock or a trap is worked on (moria3.c twice,
// traps.c once), the character sheet, the wizard screen and the saved file.

// The number. SIX CALLERS: the character sheet's "Disarming" rating
// (abilities.c), the locked door and the closed chest (moria3.c), the trap and
// the trapped chest (traps.c), the wizard screen and the saved file.
//
// An int, as `p_ptr->disarm` always was once C had widened it: all four of the
// game's readers drop it straight into a larger sum.
int player_disarm(void);

// The number outright. THREE CALLERS: the race's base plus the creation-time
// dexterity bonus (create.c), the saved file's short put back, and the wizard
// screen's prompt.
//
// THE SAME SENTENCE FOR ALL THREE, because all three are a plain replacement.
// That is the fourth answer to the question the fifteenth unit raised, and it
// agrees with the hit die and the base to-hit rather than with the armour class.
//
// THE 0 TO 200 RANGE IS THE WIZARD SCREEN'S OWN and stays there: this window
// refuses nothing, because the field refused nothing (ledger observation 24).
void player_disarm_set(int chance);

// This much better -- the class's mdis, added to whatever the race left here.
// ONE CALLER, create.c, which used to spell it `m_ptr->disarm += c_ptr->mdis;`.
// A window of its own rather than read-add-write, so the store is touched once.
void player_disarm_adjust(int chance);

// WHAT THIS MODULE DOES NOT ANSWER -- four things, all still in the callers:
//
//   1. THE TOTAL THAT IS ACTUALLY ROLLED AGAINST. Four places spell out
//
//        disarm + 2 * todis_adj() + stat_adj(A_INT)
//                + class_level_adj[pclass][CLA_DISARM] * player_level() / 3
//
//      and they are identical down to the last character (abilities.c:52,
//      moria3.c:876, moria3.c:901, traps.c). FOLDING THE FOUR INTO ONE
//      WINDOW WAITED FOR #18-12-28 and that wait is over: the subscript is
//      player_class() now, so the fold no longer means reaching `py`. IT IS STILL
//      NOT MADE -- making it means player_disarm.c calling player_class.c, which
//      is a unit of its own. #18-12-28 unlocked the fold and deliberately left it
//      (ledger observation 42).
//   2. WHETHER THE ATTEMPT SUCCEEDS. `(i - t_ptr->p1) > randint(100)` is the
//      caller's, and so is the trap's own difficulty.
//   3. BEING BLIND, CONFUSED OR HALLUCINATING. traps.c divides the total by ten
//      for each of those, up to three times over. One reader, one place.
//   4. WHAT THE CHARACTER SHEET SAYS. likert() turns the total into a word with
//      a divisor of eight; that is the sheet's question (abilities.h).

#endif // PLAYER_DISARM_H
