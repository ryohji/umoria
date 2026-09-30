// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How well this character shrugs off a spell, a trap or a curse

#ifndef PLAYER_SAVING_THROW_H
#define PLAYER_SAVING_THROW_H

// THE QUESTION. How well does this character resist something being done to
// them, before the wisdom, the level and the class are counted? The unit is a
// PERCENTAGE-LIKE CHANCE compared against `randint(100)`, so BIGGER MEANS MORE
// LIKELY TO SHRUG IT OFF. A Halfling Warrior starts at thirty-six, a Human
// Warrior at eighteen, and a Half-Troll Warrior -- the worst pair the class
// table allows that race -- at ten.
//
// THE SIXTH QUESTION OUT OF struct misc, after how deep the character has been,
// the hit die, the armour class, the base to-hit and the disarming skill. ONE
// FIELD WITH ONE ANSWER, the same simple shape as the hit die and the disarming
// skill.
//
// ONE NUMBER, BUT THE CHARACTER SHEET TURNS IT INTO TWO RATINGS:
//
//   - "Saving Throw" adds stat_adj(A_WIS) and the CLA_SAVE column,
//   - "Magic Device" adds stat_adj(A_INT) and the CLA_DEVICE column.
//
// So HOW WELL A WAND OR A STAFF IS HANDLED IS THIS SAME NUMBER, read with the
// intelligence instead of the wisdom. tests/put_misc3_test.c:238 has called that
// SUSPICIOUS since it was written, because a device would more naturally hang off
// the disarming skill; whether it was meant is unknowable now, and it is the
// game's behaviour, so it is kept. THIS MODULE ONLY KEEPS THE NUMBER -- neither
// rating is worked out here.
//
// WHERE THE STARTING NUMBER COMES FROM -- two tables, both at creation, and
// NOTHING IS BAKED IN ON THE WAY (unlike the disarming skill, which freezes a
// copy of the creation-time dexterity bonus inside itself):
//
//   - THE RACE gives the base (race_type.bsav): Human 0, Half-Orc -3,
//     Half-Troll -8, Half-Elf 3, Elf 6, Dwarf 9, Gnome 12, Halfling 18.
//     NEGATIVE IS A REAL ANSWER, as it is for the disarming skill.
//   - THE CLASS adds to it (class_type.msav): Warrior 18, Paladin 24,
//     Priest 30, Rogue 30, Ranger 30, Mage 36 -- the spell-casters resist best.
//
// BEWARE THE NAME. `save` is the most crowded word in this source tree: src/save/save.c
// and save_char() are about writing the game out, save_screen() is about the
// display, device_use_chance()'s first parameter happens to be this very number,
// direction.c has a local `int save` holding a command count, and race_type.bsav,
// class_type.msav, CLA_SAVE and player_abilities.save are four more spellings of
// nearby things. `grep -w save` finds eighty-three lines in src/*.c; eleven of
// them were this question.
//
// AND BEWARE player_saves(). THAT function (player_damage.c) is THE ROLL -- it answers
// "did they resist this time?" with a bool. THIS window answers "how good are
// they at resisting?" with a number. The roll stays where it is, because it needs
// randint() and class_level_adj[pclass] as well.

// The number. SEVEN CALLERS: the roll itself (player_damage.c), the sheet's two ratings
// (abilities.c twice), the chance of using a staff and of using a wand
// (staffs.c, wands.c), the wizard screen and the saved file.
//
// An int, as `p_ptr->save` always was once C had widened it: every reader drops
// it straight into a larger sum.
int player_saving_throw(void);

// The number outright. THREE CALLERS: the race's base (create.c), the saved
// file's short put back, and the wizard screen's prompt.
//
// THE SAME SENTENCE FOR ALL THREE, because all three are a plain replacement.
// That is the fifth answer to the question the fifteenth unit raised, and it
// agrees with the hit die, the base to-hit and the disarming skill rather than
// with the armour class.
//
// THE WIZARD SCREEN'S RANGE STAYS THERE, and it is worth knowing that the screen
// contradicts itself: the prompt says "(0-100)" while the check that follows
// admits anything from 0 to 200 -- the same check the disarming prompt uses, with
// a different sentence above it. This window refuses nothing either way, because
// the field refused nothing (ledger observation 24).
void player_saving_throw_set(int chance);

// This much better -- the class's msav, added to whatever the race left here.
// ONE CALLER, create.c, which used to spell it `m_ptr->save += c_ptr->msav;`.
// A window of its own rather than read-add-write, so the store is touched once.
void player_saving_throw_adjust(int chance);

// WHAT THIS MODULE DOES NOT ANSWER -- four things, all still in the callers:
//
//   1. WHETHER THIS PARTICULAR ATTEMPT IS RESISTED. player_saves() (player_damage.c)
//      rolls it, and every caller in the game asks that function rather than
//      this one.
//   2. THE TWO TOTALS. The saving throw's total is spelled out in two places
//      (player_damage.c's player_saves() and abilities.c:54) and the device's in three
//      (abilities.c:58, staffs.c:43, wands.c:50), always as
//
//        save + stat_adj(A_WIS or A_INT)
//             + class_level_adj[pclass][CLA_SAVE or CLA_DEVICE] * player_level() / 3
//
//      FOLDING THOSE FIVE WAITED FOR #18-12-28 and that wait is over: the
//      subscript is player_class() now, so the fold no longer means reaching `py`.
//      IT IS STILL NOT MADE, for the same reason as the disarming skill's four --
//      it means player_saving_throw.c calling player_class.c, which is a unit of
//      its own. This was the second of the two folds waiting on the class, and
//      #18-12-28 unlocked both without making either (ledger observation 42).
//   3. THE REST OF THE DEVICE CHANCE. device_use_chance() (device.c) takes this
//      number as its first argument and then subtracts the item's level and a
//      penalty, and halves what is left when the character is confused. That
//      function is already on its own with its own tests.
//   4. WHAT THE CHARACTER SHEET SAYS. likert() turns either total into a word
//      with a divisor of six; that is the sheet's question (abilities.h).

#endif // PLAYER_SAVING_THROW_H
