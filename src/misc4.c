// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Misc code for maintaining the dungeon, printing player info

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

#include <stdarg.h>

// concatenate var length string arguments (last should be NULL) into buffer.
// returns buffer.
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
