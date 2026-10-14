// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#include <string.h>
#include "config.h"
#include "constant.h"
#include "types.h"
#include "player_identity.h"
#include "externs.h"

// Not one line here reaches out to anything. The six answers start life in the race
// table and the background table, but this file reads neither: creation rolls them
// and hands the answers in, and player_bio.h lists the three rules (king or queen,
// the starting purse, which height table) that stay out there with them.

// THE PLACE THE SIX ANSWERS LIVE. These six statics are the only place the answers
// live, and the thirteen windows below are the only way to reach them.
//
// THE SIX ARE SIX STORES AND NOT ONE STRUCT. They are set in the same breath at
// creation, but nothing ever reads two of them together, so there is nothing for
// a struct to say here.
//
// THE SEX IS A bool NOW, where the field was a uint8_t. The byte's width was the
// save file's business ("anything but zero is male"), and the save file still reads
// and writes a byte -- rd_byte() hands `raw != 0` to player_set_male() and
// wr_byte() writes 1 or 0 back. NOTHING ELSE EVER WANTED THE NUMBER: all nine
// readers asked a yes-or-no question, so the store answers one.
//
// ZERO IS THE START OF A CHARACTER WHO HAS NOT BEEN ROLLED, for all six at once: no
// name, not male, no age, no height, no class, no story. The name and the story are
// arrays, and their promise -- EVERY BYTE PAST THE TERMINATOR IS ZERO -- does not
// rest on that start: the two setters below empty the store before they copy, which
// is what the name prompt's ESC path reads (see player_name_set()).
static char the_name[PLAYER_NAME_SIZE];
static bool the_sex;
static uint16_t the_age;
static uint16_t the_height;
static int16_t the_social_class;
static char the_history[PLAYER_HISTORY_LINES][PLAYER_HISTORY_LINE_SIZE];

const char *player_name(void) {
    // const, and the seven callers only read. The four places that WRITE a name go
    // through player_name_set() -- see the header.
    return the_name;
}

bool player_is_male(void) {
    // "Anything but zero is male" is how all nine callers read the byte while it was
    // a byte, and the only place a value other than 1 or 0 could come from is a
    // hand-edited save file. save.c turns that byte into `raw != 0` on the way in,
    // so the store answers the question the callers actually ask.
    return the_sex;
}

int player_age(void) {
    return the_age;
}

int player_height(void) {
    return the_height;
}

int player_social_class(void) {
    // Signed, and the 1..100 clamp stays in get_history(): this window hands back
    // whatever was put in, including the 0 of a character who has not been rolled.
    return the_social_class;
}

const char *player_history_line(int line) {
    // No bounds check (findings.md 24). The three callers all loop to
    // PLAYER_HISTORY_LINES, which is the array's own bound.
    return the_history[line];
}

void player_name_set(const char *name) {
    // The name prompt and a saved name put back. Cut at PLAYER_NAME_SIZE - 1, which
    // is where the record stopped as well.
    //
    // THE STORE IS EMPTIED FIRST so that this window can promise EVERY BYTE PAST
    // THE TERMINATOR IS ZERO. The promise is worth having because get_string() does
    // NOT write a terminator when the player presses ESC: the bytes after a name
    // are read after all, since get_name() copies all PLAYER_NAME_SIZE bytes out,
    // lets get_string() write into the copy and then looks at `name[0] == 0`. Zero
    // is what those bytes held before any name was ever set, so no reader can tell
    // the difference.
    //
    // STRICTLY, strncpy() DOES MOST OF THIS ITSELF -- it pads to its bound -- so
    // the only byte this line reaches that strncpy() would not is the last one,
    // which matters for a name of exactly PLAYER_NAME_SIZE - 1 characters. Taking
    // the line out drops no test, measured: the store starts zeroed and nothing can
    // put anything else in that byte. IT STAYS BECAUSE THE PROMISE SHOULD NOT REST
    // ON THE STORE'S HISTORY -- the store is a static array of its own now, and the
    // line reads the same whatever the store turns out to be next.
    char *store = the_name;
    memset(store, 0, PLAYER_NAME_SIZE);
    (void)strncpy(store, name, PLAYER_NAME_SIZE - 1);
}

void player_set_male(bool male) {
    // Creation's two branches and a saved byte put back. THE STORE IS A bool, so
    // there is nothing left to narrow: see the header for the one round trip that
    // changes (a hand-edited 2 comes back out of the save file as 1).
    the_sex = male;
}

void player_age_set(int age) {
    // The cast is the store's own width (uint16_t), not a rule this window adds.
    the_age = (uint16_t)age;
}

void player_height_set(int height) {
    // Creation's two height tables (male and female) and a saved short put back.
    // Same width, same truncation as the field: randnor() can return a negative
    // number and the field wrapped it, so the static wraps it too.
    the_height = (uint16_t)height;
}

void player_social_class_set(int social_class) {
    the_social_class = (int16_t)social_class;
}

void player_history_line_set(int line, const char *text) {
    // Creation handing over one wrapped line, and a saved line put back. A line
    // arrives as a string and is kept as a string -- the two-step write that put the
    // terminator one byte past a sixty-character line is gone (bug candidate B21,
    // which never reached anything; the note is in types.h).
    //
    // Emptied first for the same reason as the name, and with the same small print:
    // strncpy() pads to its bound, so this line is what makes the sixty-first byte a
    // terminator when a line really is sixty characters long.
    char *store = the_history[line];
    memset(store, 0, PLAYER_HISTORY_LINE_SIZE);
    (void)strncpy(store, text, PLAYER_HISTORY_LINE_SIZE - 1);
}

void player_history_clear(void) {
    // get_history() emptying the four lines before it fills them. The field's own
    // loop wrote only the leading '\0' of each line; this empties the whole line,
    // which no reader can tell apart (they all stop at the first '\0', and that
    // first '\0' is where it was). One call where the field needed a loop.
    for (int line = 0; line < PLAYER_HISTORY_LINES; line++) {
        memset(the_history[line], 0, PLAYER_HISTORY_LINE_SIZE);
    }
}

// THE ROW NUMBER ITSELF. This one byte is the only place the answer lives, and
// the windows below are the only way to reach it.
//
// ZERO IS WHERE A CHARACTER STARTS AND ALSO A REAL ANSWER: nothing is chosen yet
// until the race menu is answered, and row zero happens to be Human. The byte is
// NOT A QUANTITY -- it is a row, so there is nothing here to add to and no
// `_adjust` window to go with the setter.
static uint8_t race_the_row;

int player_race(void) {
    return race_the_row;
}

void player_race_set(int row) {
    // The race menu and the saved file's byte put back. One sentence for both,
    // because both are a plain replacement -- and there is no `_adjust` to go with
    // it, because a character does not become more of a Dwarf.
    //
    // The cast is the store's own width (uint8_t), not a rule this window adds.
    race_the_row = (uint8_t)row;
}

const char *player_race_name(void) {
    // Not bounds-checked, deliberately (findings.md 24). The menu cannot produce a
    // bad row and nothing else writes this but a saved file.
    return race[race_the_row].trace;
}

// THE PLACE THE ANSWER LIVES. A byte of its own. Nothing outside this file can
// reach the row.
//
// No initialiser, so the row starts at 0: a Warrior, and also "no class picked yet".
// Those are the same byte, which is why get_class() can write a 0 before its
// menu loop without meaning anything by it.
static uint8_t class_the_row;

int player_class(void) {
    return class_the_row;
}

void player_class_set(int row) {
    // The class menu, the zero it writes before the menu loop, and a saved byte put
    // back. One sentence for all three, because all three are a plain replacement.
    //
    // The cast is the store's own width (uint8_t), not a rule this window adds.
    class_the_row = (uint8_t)row;
}

const char *player_class_title(void) {
    // No bounds check on the row (findings.md 24). A row past the end of the table
    // is a row past the end of the table.
    return class[class_the_row].title;
}

int player_class_spell_type(void) {
    // NONE, MAGE or PRIEST. The fifteen callers compare the answer themselves; this
    // window does not turn the number into a yes or a no (see the header).
    return class[class_the_row].spell;
}
