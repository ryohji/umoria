// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Which of the six classes this character is

#include "config.h"
#include "constant.h"
#include "types.h"
#include "externs.h"

#include "player_class.h"

// THE PLACE THE ANSWER LIVES. A byte of its own. Nothing outside this file can
// reach the row.
//
// No initialiser, so the row starts at 0: a Warrior, and also "no class picked yet".
// Those are the same byte and always were (`py` was zeroed whole), which is why
// get_class() can write a 0 before its menu loop without meaning anything by it.
static uint8_t the_row;

int player_class(void) {
    return the_row;
}

void player_class_set(int row) {
    // The class menu, the zero it writes before the menu loop, and a saved byte put
    // back. One sentence for all three, because all three are a plain replacement.
    //
    // The cast is the store's own width (uint8_t), not a rule this window adds.
    the_row = (uint8_t)row;
}

const char *player_class_title(void) {
    // No bounds check on the row (findings.md 24). A row past the end of the table
    // is a row past the end of the table.
    return class[the_row].title;
}

int player_class_spell_type(void) {
    // NONE, MAGE or PRIEST. The fifteen callers compare the answer themselves; this
    // window does not turn the number into a yes or a no (see the header).
    return class[the_row].spell;
}
