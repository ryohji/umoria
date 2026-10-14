// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Whether the input has run out: where the count lives

#include "config.h"
#include "constant.h"
#include "types.h"

#include "ui_input.h"
#include "ui_internal.h"

// This module uses nothing outside itself.

// EOF counter. Starts at 0.
static int eof_count = 0;

// The number of EOFs io.c has put up with before it panic-saves and dies. Stated
// once, here, and nowhere else (see ui_internal.h for why it is not at the call site).
#define EOF_TRIES_ALLOWED 100

void note_input_ended(void) {
    eof_count++;
}

bool input_has_ended(void) {
    return eof_count != 0;
}

bool input_end_is_hopeless(void) {
    return eof_count > EOF_TRIES_ALLOWED;
}

int input_end_count(void) {
    return eof_count;
}
