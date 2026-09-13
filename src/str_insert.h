// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Substituting a template inside a string.

#ifndef STR_INSERT_H
#define STR_INSERT_H

#include <stdint.h>

// Two functions moved out of misc3.c unchanged. Both rewrite the string given
// as the first argument in place, and both do nothing at all when the template
// is not found. They are the only place in the game where a name is built by
// substitution, so they were never related to the ten other jobs misc3.c does.
//
// The name is not strings.h on purpose: -Isrc would let src/strings.h shadow
// the standard <strings.h> for every translation unit (src/curses.h already
// shadows a system header that way).
//
// This header deliberately does not include types.h, which has no include
// guard and so cannot be pulled in twice. Include config.h, constant.h and
// types.h before this file, the same as externs.h expects.

// Inserts a string into a string
// Replaces the first occurrence of `mtc_str` in `object_str` with `insert`
// (a NULL `insert` just removes the template).
void insert_str(char *object_str, const char *mtc_str, const char *insert);

// Replaces the first occurrence of `mtc_str` in `object_str` with `number`
// written out in decimal, prefixed with '+' when `show_sign` is true and the
// number is not negative.
void insert_lnum(char *object_str, const char *mtc_str, int32_t number, int show_sign);

#endif // STR_INSERT_H
