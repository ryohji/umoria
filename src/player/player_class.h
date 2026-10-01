// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Which of the six classes this character is

#ifndef PLAYER_CLASS_H
#define PLAYER_CLASS_H

// THE QUESTION. Which class did the player pick? The answer is A ROW NUMBER into
// class[] (src/data/player.c:283), nothing more:
//
//   0 Warrior   1 Mage   2 Priest   3 Rogue   4 Ranger   5 Paladin
//
// THIS ONE IS NOT A QUANTITY. Every caller indexes a table with it or hands the
// byte on unchanged -- not one compares it against a number and not one adds to it.
// So there is no `_adjust` window here and there never will be: A CHARACTER DOES NOT
// SLOWLY BECOME MORE OF A ROGUE.
//
// FIFTY-FIVE CALLS. The reason is not that the answer is complicated -- it is one
// byte written twice -- but that FOUR CONSTANT TABLES ARE INDEXED BY IT: class[]
// itself, class_level_adj[][], magic_spell[][] and player_title[][], plus
// player_init[][] at the very start of a game.
//
// WRITTEN EXACTLY TWICE IN A CHARACTER'S LIFE: once when the class menu is answered
// (create.c) and once when a saved game is read back (save.c). There is a third write
// in the source and it is a zero -- get_class() clears the byte before the menu loop,
// and the loop cannot be left without an answer, so the zero is never the answer a
// character keeps.
//
// BEWARE THE NAME, and this time it is the table's name rather than the field's.
// `grep -w pclass` finds sixty-two lines and fifty-five of them are this field (the
// other seven are prose). `grep -w class` finds SIXTY-NINE and only TWENTY-TWO are
// this table: thirty-five are prose, THREE are the table under another subscript,
// and EIGHT are high_scores.class -- a byte in the score file that happens to hold
// the same row number and is not this question (death.c, save.c).

// The row number. THIRTY-THREE CALLERS, every one of them a subscript or a byte
// handed on:
//
//   - class_level_adj[row][column] SIXTEEN TIMES -- the to-hit columns seven times
//     (abilities.c, hit_rolls.c, player_melee.c, throw.c), the disarming
//     column four, the device column three and the saving column twice.
//   - magic_spell[row - 1][spell] TEN TIMES (dungeon.c, magic.c, spellbook.c seven times,
//     prayer.c).
//   - class[row] for what the class GIVES: the whole row at creation (create.c) and
//     first_spell_lev twice (spellbook.c).
//   - player_title[row][level - 1] once (status_line.c) and player_init[row][i] once
//     (main.c, the starting pack).
//   - the high score entry (death.c) and the saved file's byte (save.c).
//
// An int, though the field was a uint8_t: every caller immediately uses it as an
// array subscript, where an int is what C wants anyway.
int player_class(void);

// The row outright. THREE CALLERS: the class menu (create.c), the zero it writes
// before the menu loop, and the saved file's byte put back (save.c).
//
// IT REFUSES NOTHING (findings.md 24). The menu can only produce 0..MAX_CLASS-1 and
// a saved file can only hold a byte, but neither end was ever checked and this window
// does not start checking. WIDTH IS STILL A BYTE, so 256 lands on 0 and -1 lands on
// 255.
void player_class_set(int row);

// The class's name, as the game spells it -- "Warrior", "Mage", ... FOUR CALLERS:
// the character sheet's side panel (status_line.c), the name/race/sex/class block
// (char_screen.c), the dumped character file (files.c) and the tomb (death.c).
//
// THE MIRROR OF player_race_name(), and the two stand side by side at two of those
// four sites -- status_line.c:297 asks for the race's name on the line above and
// char_screen.c:54 on the line two above. The four callers want the class's name, not
// a table lookup.
//
// THIS AND THE NEXT WINDOW ARE THE ONLY LINES THAT REACH OUT: class[] is one of
// the read-only constant tables declared in externs.h.
//
// const char *, which is what class_type.title already is (types.h:501) and what
// both receivers -- prt_field() and put_buffer() -- already take.
const char *player_class_title(void);

// Which school of magic this class uses, if any: NONE, MAGE or PRIEST as
// constant.h spells them. FIFTEEN CALLERS, the most of any window in this header.
// All fifteen ask which school: compared against MAGE or PRIEST.
//
// THE VALUE AND NOT A PAIR OF YES-OR-NO WINDOWS. `player_class_casts_spells()` and
// `player_class_says_prayers()` would read better at eleven of the fifteen sites,
// and they are wrong: THE THREE ANSWERS ARE ONE NUMBER, and four sites branch on
// all three at once (main.c, dungeon.c, stat_ops.c, level_ops.c) where a pair of predicates
// would have to be asked twice and could disagree. One number, one window (findings.md
// 43).
//
// TWO CALLERS THAT READ ONLY THIS. gain_level() (level_ops.c:45) and lose_exp()
// (spells.c) read nothing but this window, twice each.
//
// WHAT THE SCHOOL DECIDES stays with the callers, all of it: which stat the spells
// hang off (A_INT or A_WIS), which word to print ("spell" or "prayer"), which half
// of spell_names[] to index (SPELL_OFFSET or PRAYER_OFFSET), which book can be read
// and whether the book is needed at all (a priest gets prayers from their god and
// can learn them blind). The school is a fact about the class; what follows from it
// is the spell code's business.
int player_class_spell_type(void);

// WHAT THIS MODULE DOES NOT ANSWER -- SIX things: fifty-seven calls (on fifty-five
// lines) ask six different questions:
//
//   1. WHAT THE CLASS IS WORTH IN A SKILL. class_level_adj[row][column] is read at
//      sixteen sites and stays at all sixteen. It is a table about A PAIR (a class
//      and a skill), and the expressions around it differ at every site. NINE OF THE
//      SIXTEEN ARE COPIES OF EACH OTHER (findings.md 42): four spell out the disarming
//      total and five the saving throw and the device chance. Folding them means
//      player_disarm.c calling player_class.c, which is a unit of its own.
//   2. WHERE THIS CLASS'S SPELLS ARE. magic_spell[row - 1][spell] is read at ten
//      sites and THE MINUS ONE IS THE TABLE'S OWN LAYOUT, not a fact about the
//      class: the table has MAX_CLASS - 1 rows because a Warrior has no spells
//      (src/data/player.c:307). With ten sites, if these ever fold it should be into a
//      module for the spell table, not this one.
//   3. WHAT THE CLASS GIVES. create.c reads the row whole for the six stat
//      adjustments, the hit die's bonus, the stealth, the experience factor and the
//      title, and spellbook.c reads first_spell_lev twice. "Which class is this?" and
//      "what does that class give?" are two questions, and only the first is here.
//   4. THE TITLE FOR THIS LEVEL. player_title[row][level - 1] (status_line.c) is
//      a table about A PAIR again, and the other half of the pair is a question
//      that already has a module of its own (player_level.h).
//   5. THE STARTING PACK. player_init[row][i] (main.c) is what a new character of
//      this class carries, which is creation's business and is read once.
//   6. THE SCORE FILE'S BYTE. high_scores.class holds this row number and IS NOT
//      THIS QUESTION -- it is a record of a character who may be dead and gone
//      (death.c, save.c). The high score table is its own unit's problem.

#endif // PLAYER_CLASS_H
