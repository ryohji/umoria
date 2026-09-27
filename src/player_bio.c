// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Who this character is: the name, the sex, the age, the height, the social
// class and the four lines of life story

// memset() and strncpy(), for the two answers that are strings. str_insert.c is
// the precedent for asking for one standard header by name rather than pulling in
// headers.h, which would bring windows.h with it on one of the three platforms.
#include <string.h>

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_bio.h"

// No externs.h here, the same as the twenty-five questions before this one. The six
// answers come from the race table and the background table, but THIS FILE READS
// NEITHER: creation rolls them and hands the answers in, and player_bio.h lists the
// three rules (king or queen, the starting purse, which height table) that stay out
// there with them.

// The width of a story line is said in player_bio.h and again, one byte at a time,
// in types.h. While both exist they have to agree: a line of sixty characters and
// its terminator has to fit, or player_history_line_set() would write past the
// record. AFTER #18-12-26C THIS CHECK GOES AWAY with the field, and the array below
// becomes the only place the number is spent.
_Static_assert(sizeof(((player_type *)0)->misc.history) ==
                   PLAYER_HISTORY_LINES * PLAYER_HISTORY_LINE_SIZE,
               "four story lines of sixty characters and a terminator");

// THE PLACE THE SIX ANSWERS LIVE, for the length of step A only: still the fields of
// the character's record, reached through one door each so that #18-12-26C can move
// them by changing these six functions and nothing else.
//
// SIX DOORS AND NOT ONE DOOR ONTO struct misc, even though all six fields sit in the
// same struct: the six become six separate statics in step C, so the seam has to be
// at the same grain as the move.
extern player_type py;

static char *the_name(void) {
    return py.misc.name;
}

static uint8_t *the_sex(void) {
    return &py.misc.male;
}

static uint16_t *the_age(void) {
    return &py.misc.age;
}

static uint16_t *the_height(void) {
    return &py.misc.ht;
}

static int16_t *the_social_class(void) {
    return &py.misc.sc;
}

static char *the_history_line(int line) {
    return py.misc.history[line];
}

const char *player_name(void) {
    // const, and the seven callers only read. The four places that WRITE a name go
    // through player_name_set() from #18-12-26B on -- see the header.
    return the_name();
}

bool player_is_male(void) {
    // Anything but zero is male, which is how all nine callers read the byte
    // (`if (py.misc.male)` and `? "Male" : "Female"`). A byte of 2 out of a
    // hand-edited save file was true before and is true here.
    return *the_sex() != 0;
}

int player_age(void) {
    return *the_age();
}

int player_height(void) {
    return *the_height();
}

int player_social_class(void) {
    // Signed, and the 1..100 clamp stays in get_history(): this window hands back
    // whatever was put in, including the 0 of a character who has not been rolled.
    return *the_social_class();
}

const char *player_history_line(int line) {
    // No bounds check, the same as the field had none. The three callers all loop to
    // PLAYER_HISTORY_LINES.
    return the_history_line(line);
}

void player_name_set(const char *name) {
    // The name prompt and a saved name put back. Cut at PLAYER_NAME_SIZE - 1, which
    // is where the record stopped as well.
    //
    // THE STORE IS EMPTIED FIRST so that this window can promise EVERY BYTE PAST
    // THE TERMINATOR IS ZERO. The promise is worth having because get_string() does
    // NOT write a terminator when the player presses ESC: the bytes after a name
    // are read after all, since get_name() goes on to look at `name[0] == 0` with
    // only the typed characters in place. Zero is what those bytes held before any
    // name was ever set, so no reader can tell the difference.
    //
    // STRICTLY, strncpy() DOES MOST OF THIS ITSELF -- it pads to its bound -- so
    // the only byte this line reaches that strncpy() would not is the last one,
    // which matters for a name of exactly PLAYER_NAME_SIZE - 1 characters. Taking
    // the line out drops no test, measured: the store starts zeroed and nothing can
    // put anything else in that byte. IT STAYS BECAUSE THE PROMISE SHOULD NOT REST
    // ON THE STORE'S HISTORY, and step C gives the store a different history.
    char *store = the_name();
    memset(store, 0, PLAYER_NAME_SIZE);
    (void)strncpy(store, name, PLAYER_NAME_SIZE - 1);
}

void player_set_male(bool male) {
    // Creation's two branches and a saved byte put back. Written as 1 or 0: see the
    // header for the one round trip that changes (a hand-edited 2 comes back as 1).
    *the_sex() = (uint8_t)(male ? 1 : 0);
}

void player_age_set(int age) {
    // The cast is the store's own width, not a rule this window adds: the field was
    // uint16_t and `py.misc.age = race[i].b_age + randint(...)` truncated exactly
    // like this.
    *the_age() = (uint16_t)age;
}

void player_height_set(int height) {
    // Creation's two height tables (male and female) and a saved short put back.
    // Same width, same truncation as the field: randnor() can return a negative
    // number and the field wrapped it, so this one does too.
    *the_height() = (uint16_t)height;
}

void player_social_class_set(int social_class) {
    *the_social_class() = (int16_t)social_class;
}

void player_history_line_set(int line, const char *text) {
    // Creation handing over one wrapped line, and a saved line put back. A line
    // arrives as a string and is kept as a string -- the two-step write that put the
    // terminator one byte past a sixty-character line is gone (see types.h).
    //
    // Emptied first for the same reason as the name, and with the same small print:
    // strncpy() pads to its bound, so this line is what makes the sixty-first byte a
    // terminator when a line really is sixty characters long.
    char *store = the_history_line(line);
    memset(store, 0, PLAYER_HISTORY_LINE_SIZE);
    (void)strncpy(store, text, PLAYER_HISTORY_LINE_SIZE - 1);
}

void player_history_clear(void) {
    // get_history() emptying the four lines before it fills them. The field's own
    // loop wrote only the leading '\0' of each line; this empties the whole line,
    // which no reader can tell apart (they all stop at the first '\0', and that
    // first '\0' is where it was).
    for (int line = 0; line < PLAYER_HISTORY_LINES; line++) {
        memset(the_history_line(line), 0, PLAYER_HISTORY_LINE_SIZE);
    }
}
