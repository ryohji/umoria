// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#ifndef PLAYER_IDENTITY_H
#define PLAYER_IDENTITY_H

// THE QUESTION. Who is this character? Six answers that creation works out once
// and nothing in the game ever changes again: WHAT THEY ARE CALLED, WHETHER THEY
// ARE MALE, HOW OLD THEY ARE, HOW TALL THEY ARE, WHAT THEY WERE BORN INTO, and
// THE FOUR LINES OF LIFE STORY the background table spun for them.
//
// SIX ANSWERS AND ONE MODULE, and the usual rule for splitting says nothing here.
// The rule is "does anything read one without the other" -- and all six are read
// apart, every time. NOTHING ADDS TWO OF THEM UP, nothing compares two of them,
// nothing needs two at once. By that rule alone this would be six modules. Two
// other facts put them together instead:
//
//   1. CREATION SETS EACH ONE ONCE AND NOBODY EVER MOVES IT. There is no place in
//      the game that adds to an age or a height, so THERE IS NO `_adjust` WINDOW
//      IN THIS HEADER AT ALL -- six answers, and not one of them can be nudged.
//      The one exception is the name, which change_name()
//      SETS AGAIN -- replacing it, not moving it.
//   2. THE SAME FOUR PLACES ASK FOR THEM. The character sheet (put_character() and
//      put_misc1()), the file the player dumps their character to
//      (file_character()), and the save file's writer and reader. Split into six
//      modules, every one of the six would be read from those same four places and
//      none of them would stand on its own.
//
// WHO ASKS: creation (create.c, which sets all six and reads three of them back),
// the character sheet and the name prompt (char_screen.c), the character dump
// (files.c), the tomb and the high score entry (death.c), the '@' line of the
// symbol help (help.c), and the save file (save.c).
//
// THE SEX IS THE ONLY ONE OF THE SIX WITH RULES ATTACHED, and all three of them
// stay with their callers:
//
//   - KING OR QUEEN. status_line.c's title_string() says **KING** / **QUEEN**,
//     death.c's tomb says *King* / *Queen*, and death.c's kingly() says "All Hail
//     the Mighty King!" -- THE SAME FORK WRITTEN THREE TIMES WITH THREE DIFFERENT
//     PAIRS OF WORDS. None of them is this question's answer.
//   - THE PURSE A NEW CHARACTER STARTS WITH. create.c's get_money() works it out
//     from the social class (`sc * 6`) and adds fifty for a woman ("She charmed
//     the banker into it! -CJS-"). The class and the sex are TERMS IN A SUM ABOUT
//     MONEY, and the sum belongs where the money is.
//   - WHICH HEIGHT TABLE TO ROLL AGAINST. create.c's get_ahw() picks m_b_ht/m_m_ht
//     or f_b_ht/f_m_ht. The body's weight left the same shape with its caller.

// The four lines of life story, said once. The lines are four because
// get_history() clears four, fills at most four and prints four, and the enumerated
// 330,984 stories the background table can make never need a fifth. A LINE IS
// SIXTY CHARACTERS AND A TERMINATOR: get_history() wraps at sixty, and 8,057 of
// those stories really do have a line of exactly sixty.
//
// player_bio.c holds a _Static_assert so these two numbers and the field in types.h
// cannot drift apart. THE NAME'S WIDTH IS NOT HERE: PLAYER_NAME_SIZE stays in
// types.h because the high score table's own name field is that wide too, and that
// field is not this question.
#define PLAYER_HISTORY_LINES 4
#define PLAYER_HISTORY_LINE_SIZE 61

// WHAT THE CHARACTER IS CALLED. SEVEN CALLERS (the character sheet, the name
// prompt copying the old name out before the player edits it, the character dump,
// the '@' help line, the tomb, the high score entry, the save file's writer).
//
// `const char *`, AND THE const IS THE POINT. Four of the places that name this
// field today WRITE THROUGH THE ADDRESS they are handed -- get_string() and
// user_name() fill it, rd_string() reads a saved one back into it. Handing out a
// writable `char *` would leave those four writing into the store behind the
// window's back, and the window would be an ornament. (msg_history_slot() and
// death_cause() do return `char *`; THIS MODULE DELIBERATELY DOES NOT COPY THEM.)
// Each of those four takes a local buffer instead and hands the finished name to
// player_name_set() -- which is why save.c's wr_string() takes a `const char *`.
const char *player_name(void);

// WHETHER THIS CHARACTER IS MALE. NINE CALLERS: the three king-or-queen forks, the
// two "Male"/"Female" lines (the sheet and the dump), the 'M'/'F' of the high score
// entry, the height table's choice, the extra fifty gold, and the save file.
//
// A bool, though the field was a uint8_t, because NOT ONE OF THE NINE USES IT AS A
// NUMBER -- three are `? :` and six are `if`. ONE THING DOES CHANGE BY A HAIR: a
// hand-edited save file's 2 comes back as 1 through a bool. Nothing in the game can
// tell (2 was already true, and only creation ever writes this byte, with true or
// false), so the round trip is not worth two raw windows.
bool player_is_male(void);

// HOW OLD AND HOW TALL. THREE CALLERS EACH (the sheet, the dump, the save file).
// Both were uint16_t fields and both come back as int, because every caller either
// printed them through an `(int)` cast already or hands them to prt_num(), which
// takes an int.
int player_age(void);
int player_height(void);

// WHAT THE CHARACTER WAS BORN INTO -- 1 to 100, worked out from the life story.
// FOUR CALLERS (the sheet, the dump, the starting purse, the save file). Signed,
// like the field: get_history() clamps it into 1..100 before it ever arrives, but
// THE CLAMP IS CREATION'S RULE and this window does not repeat it.
int player_social_class(void);

// ONE LINE OF THE LIFE STORY. THREE CALLERS, and all three ask for all four lines
// in a loop (creation printing them, the dump writing them, the save file).
//
// `const char *` for the same reason the name is, and NO BOUNDS CHECK on the line
// number: the three loops all run to PLAYER_HISTORY_LINES, and a window that
// quietly returned "" for a fifth line would be inventing an answer the record
// never had (findings.md 24).
const char *player_history_line(int line);

// SETTING THE SIX. TWO CALLERS for the name (the name prompt, the save file), THREE
// for the sex (creation's two branches, the save file), TWO for the age, THREE for
// the height (creation's two height tables, the save file), TWO for the social
// class.
//
// THE SEX'S WINDOW IS SPELLED THE OTHER WAY ROUND on purpose. The other setters
// end in `_set`, and this one would be `player_male_set()` -- a name for a thing called "the male", which this module
// never says. Its reader is a question (`player_is_male()`), so its writer is an
// instruction (`player_set_male()`).
//
// THEY REFUSE NOTHING, because the fields refused nothing. A name longer than
// PLAYER_NAME_SIZE - 1 is cut (the field could not hold it either; writing past it
// is what user_name() does today with a login name of twenty-seven characters or
// more), and a story line longer than sixty characters is cut the same way.
void player_name_set(const char *name);
void player_set_male(bool male);
void player_age_set(int age);
void player_height_set(int height);
void player_social_class_set(int social_class);

// ONE LINE OF THE LIFE STORY, PUT THERE. TWO CALLERS: creation, which wraps the
// block of story text and hands over one finished line at a time, and the save
// file's reader.
//
// Through this window a line arrives as a string and leaves as a string (bugs.md
// B21: writing past a sixty-character line; see types.h).
void player_history_line_set(int line, const char *text);

// NO LIFE STORY AT ALL. ONE CALLER: get_history(), which empties the four lines
// before it fills them. Four lines emptied by one call, where the field needed a
// loop.
void player_history_clear(void);

// WHAT THIS MODULE DOES NOT ANSWER -- five things, all still in the callers:
//
//   1. WHERE THE ANSWERS COME FROM. create.c rolls the age and the height against
//      the race table, walks the background table for the story, and adds up the
//      social class from it. This module is told the answers.
//   2. HOW THE STORY IS WRAPPED. The sixty-character fold, the trailing blanks and
//      the line count are all get_history()'s arithmetic. This window takes a line
//      that is already a line.
//   3. KING OR QUEEN, THE STARTING PURSE, AND WHICH HEIGHT TABLE -- the three
//      rules above, in status_line.c, death.c and create.c.
//   4. WHAT THE CHARACTER SHEET LOOKS LIKE. put_character() and put_misc1() own
//      the rows, the columns and the words "Male" and "Female"; file_character()
//      owns the dump's layout.
//   5. WHAT CLASS THIS CHARACTER IS. It is read beside the name and the sex in
//      three of these places and it is still not this question: player_class.h
//      answers that, the last of the thirteen.

// THE QUESTION. Which race did the player pick? The answer is A ROW NUMBER into
// race[] (src/model/data/player.c:107), nothing more:
//
//   0 Human   1 Half-Elf   2 Elf        3 Halfling
//   4 Gnome   5 Dwarf      6 Half-Orc   7 Half-Troll
//
// THIS ONE IS NOT A QUANTITY. NOTHING IN THE GAME DOES ARITHMETIC ON THIS ONE
// except to find a row. There is no
// `_adjust` window here and there never will be -- A CHARACTER DOES NOT BECOME
// MORE OF A DWARF.
//
// WRITTEN EXACTLY TWICE IN A CHARACTER'S LIFE: once when the race menu is
// answered (create.c) and once when a saved game is read back (save.c). After
// that it never changes, which is why the three readers below can cache nothing
// and need nothing.
//
// BEWARE THE NAME. WHAT IS CROWDED IS `race`: the constant table race[], the shop
// owner's own owner_race, race_type.trace, and high_scores.race in death.c are
// four different things spelled with the same word.

// The row number. EIGHT CALLERS: the race's own row at creation three times
// (create.c), the start of the history chart (create.c), the class menu's mask
// (create.c), the high-score entry (death.c), and the shop prices twice
// (store_price.c, store_haggle.c).
//
// An int, though the field was a uint8_t: every caller immediately uses it as an
// array subscript, where an int is what C wants anyway.
int player_race(void);

// The row outright. TWO CALLERS: the race menu (create.c) and the saved file's
// byte put back (save.c).
//
// THAT IS THE SIXTH ANSWER to the question the fifteenth unit raised -- "does
// loading a saved game use the same window the race does?" -- and it is "the
// same", agreeing with the hit die, the base to-hit, the disarming skill and the
// saving throw rather than with the armour class.
//
// IT REFUSES NOTHING (findings.md 24).
// The menu can only produce 0..MAX_RACES-1 and a saved file can only hold a byte,
// but neither end was ever checked and this window does not start checking.
// WIDTH IS STILL A BYTE, so 256 lands on 0 and -1 lands on 255.
void player_race_set(int row);

// The race's name, as the game spells it -- "Human", "Half-Elf", ... THREE
// CALLERS, all of them display: the character sheet's side panel
// (status_line.c), the name/race/sex/class block (char_screen.c) and the dumped
// character file (files.c).
//
// THIS IS THE ONE PLACE THE MODULE REACHES OUT: race[] is one of the read-only
// constant tables declared in externs.h.
//
// WHY THE NAME IS IN HERE AND THE REST OF THE ROW IS NOT: what the callers want
// is the race's name. The other readers of race[] want the race's *stats*
// (the age spread, the height, the classes it may take), which is a different
// question and stays in create.c where the character is built.
//
// NOTE: this does not fold three calls into one. Each of the three sites still
// calls once, so the call count does not move -- the gain is that none of the
// three spells out a subscript.
//
// const char *, which is what race_type.trace already is (types.h:331) and what
// both receivers -- prt_field() and put_buffer() -- already take, so not one cast
// is needed anywhere.
const char *player_race_name(void);

// WHAT THIS MODULE DOES NOT ANSWER -- three things, all still in the callers:
//
//   1. WHAT THE SHOPS CHARGE. store_price.c:147 and store_haggle.c:499 index
//      rgold_adj[owner's race][this race]. That table is about A PAIR, not about
//      one race, and the two expressions differ anyway (one is `x / 100`, the
//      other `(200 - x) / 100`), so there is nothing to fold and the pricing
//      stays the shop's business.
//   2. WHERE THE LIFE STORY STARTS. create.c:194 computes `prace * 3 + 1` to find
//      the first row of background[]. That is knowledge about how background[] is
//      laid out -- the comment above it (create.c:185) says so -- not about the
//      race, and there is only one such site.
//   3. WHAT THE RACE IS LIKE. create.c reads the row whole three times, for the
//      stat adjustments, the age/height/weight spreads and the mask of classes
//      the race may take. "Which race is this?" and "what does that race give?"
//      are two questions, and only the first one is in here.

// THE QUESTION. Which class did the player pick? The answer is A ROW NUMBER into
// class[] (src/model/data/player.c:283), nothing more:
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
//   - magic_spell[row - 1][spell] TEN TIMES (app/dungeon.c, magic.c, spellbook.c seven times,
//     prayer.c).
//   - class[row] for what the class GIVES: the whole row at creation (create.c) and
//     first_spell_lev twice (spellbook.c).
//   - player_title[row][level - 1] once (status_line.c) and player_init[row][i] once
//     (app/main.c, the starting pack).
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
// all three at once (app/main.c, app/dungeon.c, stat_ops.c, level_ops.c) where a pair of predicates
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
//      (src/model/data/player.c:307). With ten sites, if these ever fold it should be into a
//      module for the spell table, not this one.
//   3. WHAT THE CLASS GIVES. create.c reads the row whole for the six stat
//      adjustments, the hit die's bonus, the stealth, the experience factor and the
//      title, and spellbook.c reads first_spell_lev twice. "Which class is this?" and
//      "what does that class give?" are two questions, and only the first is here.
//   4. THE TITLE FOR THIS LEVEL. player_title[row][level - 1] (status_line.c) is
//      a table about A PAIR again, and the other half of the pair is a question
//      that already has a module of its own (player_level.h).
//   5. THE STARTING PACK. player_init[row][i] (app/main.c) is what a new character of
//      this class carries, which is creation's business and is read once.
//   6. THE SCORE FILE'S BYTE. high_scores.class holds this row number and IS NOT
//      THIS QUESTION -- it is a record of a character who may be dead and gone
//      (death.c, save.c). The high score table is its own unit's problem.

#endif // PLAYER_IDENTITY_H
