// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Who this character is: the name, the sex, the age, the height, the social
// class and the four lines of life story

#ifndef PLAYER_BIO_H
#define PLAYER_BIO_H

// THE QUESTION. Who is this character? Six answers that creation works out once
// and nothing in the game ever changes again: WHAT THEY ARE CALLED, WHETHER THEY
// ARE MALE, HOW OLD THEY ARE, HOW TALL THEY ARE, WHAT THEY WERE BORN INTO, and
// THE FOUR LINES OF LIFE STORY the background table spun for them.
//
// THE ELEVENTH QUESTION OUT OF struct misc, after the purse, how far the character
// has come, how deep they have been, the hit die, the armour class, the base
// to-hit, the disarming skill, the saving throw, the race, the body's weight, the
// attack bonuses and the searching skill. SIX FIELDS LEAVE AT ONCE, which is more
// than any unit before this one.
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
//      (The body's weight was the first question shaped like that; here the shape
//      arrives six times over.) The one exception is the name, which change_name()
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
// symbol help (help.c), and the save file (save.c). FORTY-FOUR CALLS, the most of
// any unit on this road.
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
// number: the fields refused nothing (ledger observation 24), the three loops all
// run to PLAYER_HISTORY_LINES, and a window that quietly returned "" for a fifth
// line would be inventing an answer the record never had.
const char *player_history_line(int line);

// SETTING THE SIX. TWO CALLERS for the name (the name prompt, the save file), THREE
// for the sex (creation's two branches, the save file), TWO for the age, THREE for
// the height (creation's two height tables, the save file), TWO for the social
// class.
//
// THE SEX'S WINDOW IS SPELLED THE OTHER WAY ROUND on purpose. The eight units
// before this one all end their setters in `_set`, and this one would be
// `player_male_set()` -- a name for a thing called "the male", which this module
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
// Creation used to write the line in two steps -- strncpy() of the characters, then
// the terminator by hand -- and those two steps are what wrote one byte past a
// sixty-character line (bug candidate B21, which never reached anything; see
// types.h). Through this window a line arrives as a string and leaves as a string.
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

#endif // PLAYER_BIO_H
