// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Substituting a template inside a string, and joining strings.

#ifndef STR_INSERT_H
#define STR_INSERT_H

#include <stdint.h>

// Both functions rewrite the string given as the first argument in place, and
// both do nothing at all when the template is not found. They are the only
// place in the game where a name is built by substitution.
//
// The name is not strings.h on purpose: -Isrc would let src/strings.h shadow
// the standard <strings.h> for every translation unit (src/platform/curses.h already
// shadows a system header that way).

#include <stdbool.h>

#include "text_buffers.h"

// Returns true if ch is a vowel (lowercase or uppercase).
bool is_a_vowel(char ch);

// Inserts a string into a string
// Replaces the first occurrence of `mtc_str` in `object_str` with `insert`
// (a NULL `insert` just removes the template).
void insert_str(char *object_str, const char *mtc_str, const char *insert);

// Replaces the first occurrence of `mtc_str` in `object_str` with `number`
// written out in decimal, prefixed with '+' when `show_sign` is true and the
// number is not negative.
void insert_lnum(char *object_str, const char *mtc_str, int32_t number, int show_sign);

// Copies the strings after `buffer`, up to the first NULL, one after another
// into `buffer` and returns `buffer`. Only NULL gives the empty string. The
// length is not checked; every caller goes through the CONCAT macro, which
// passes a fresh vtype, so the result must fit in 79 characters.
char *concat(char *buffer, ...);

#define CONCAT(...) concat((vtype){0}, __VA_ARGS__, NULL)

#endif // STR_INSERT_H
