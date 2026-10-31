// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Substituting a template inside a string, and joining strings

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "text_buffers.h"

#include "str_insert.h"

// This module uses only the standard library and text_buffers.h.

// Returns true if ch is a vowel (lowercase or uppercase).
bool is_a_vowel(char ch) {
    switch (ch) {
    case 'a': case 'e': case 'i': case 'o': case 'u':
    case 'A': case 'E': case 'I': case 'O': case 'U':
        return true;
    default:
        return false;
    }
}

// Replaces the len bytes at `at` in `str` (a buffer of `size` bytes) with
// `insert`, cutting what does not fit.
static void replace_at(char *str, size_t size, char *at, size_t len, const char *insert) {
    size_t prefix = (size_t)(at - str);
    size_t ins_len = insert ? strlen(insert) : 0;
    size_t tail_len = strlen(at + len);

    if (prefix + ins_len >= size) {
        ins_len = size - 1 - prefix;
    }
    if (prefix + ins_len + tail_len >= size) {
        tail_len = size - 1 - prefix - ins_len;
    }
    (void)memmove(at + ins_len, at + len, tail_len);
    if (ins_len > 0) {
        (void)memcpy(at, insert, ins_len);
    }
    at[ins_len + tail_len] = '\0';
}

// Inserts a string into a string
void insert_str(char *object_str, size_t size, const char *mtc_str, const char *insert) {
    size_t mtc_len = strlen(mtc_str);
    size_t obj_len = strlen(object_str);
    if (mtc_len > obj_len) {
        return; // too long to be found
    }
    char *bound = object_str + obj_len - mtc_len;

    char *found = NULL;
    for (char *pc = object_str; pc <= bound; pc++) {
        char *temp_obj = pc;
        const char *temp_mtc = mtc_str;

        size_t i;
        for (i = 0; i < mtc_len; i++) {
            if (*temp_obj++ != *temp_mtc++) {
                break;
            }
        }
        if (i == mtc_len) {
            found = pc;
            break;
        }
    }

    if (found) {
        replace_at(object_str, size, found, mtc_len, insert);
    }
}

void insert_lnum(char *object_str, size_t size, const char *mtc_str, int32_t number, int show_sign) {
    size_t mlen = strlen(mtc_str);
    char *tmp_str = object_str;

    int flag = 1;
    char *string;
    do {
        string = strchr(tmp_str, mtc_str[0]);
        if (string == 0) {
            flag = 0;
        } else {
            flag = strncmp(string, mtc_str, mlen);
            if (flag) {
                tmp_str = string + 1;
            }
        }
    } while (flag);

    if (string) {
        char number_str[16];

        if ((number >= 0) && (show_sign)) {
            (void)snprintf(number_str, sizeof(number_str), "+%d", number);
        } else {
            (void)snprintf(number_str, sizeof(number_str), "%d", number);
        }
        replace_at(object_str, size, string, mlen, number_str);
    }
}

// Concatenate var length string arguments (last should be NULL) into buffer.
// Returns buffer.
//
// Callers reach it through the CONCAT macro, which keeps its own
// prototype of this function.
char *concat(char *const buffer, ...) {
    char *p = buffer;
    const char *s;
    va_list list;

    va_start(list, buffer);
    buffer[0] = '\0';
    while ((s = va_arg(list, const char *))) {
        p = strcpy(p, s) + strlen(s);
    }
    va_end(list);

    return buffer;
}
