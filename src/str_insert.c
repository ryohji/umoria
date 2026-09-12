// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Substituting a template inside a string

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "constant.h"
#include "types.h"

#include "str_insert.h"

// No externs.h here, the same as panel.c, stores.c, options.c and stats.c.
// Nothing outside the standard library is called from this file, so the linker
// reports no unresolved project symbol at all; there is not even one
// declaration left to write out by hand, as stats.c had to do for py. The only
// thing these functions borrow from the game is the vtype typedef, and the
// includes above bring that in.

// Inserts a string into a string
void insert_str(char *object_str, const char *mtc_str, const char *insert) {
    int mtc_len = (int)strlen(mtc_str);
    int obj_len = (int)strlen(object_str);
    char *bound = object_str + obj_len - mtc_len;

    char *pc;
    for (pc = object_str; pc <= bound; pc++) {
        char *temp_obj = pc;
        const char *temp_mtc = mtc_str;

        int i;
        for (i = 0; i < mtc_len; i++) {
            if (*temp_obj++ != *temp_mtc++) {
                break;
            }
        }
        if (i == mtc_len) {
            break;
        }
    }

    if (pc <= bound) {
        char out_val[80];

        (void)strncpy(out_val, object_str, (pc - object_str));
        // Turbo C needs int for array index.
        out_val[(int)(pc - object_str)] = '\0';
        if (insert) {
            (void)strcat(out_val, insert);
        }
        (void)strcat(out_val, (pc + mtc_len));
        (void)strcpy(object_str, out_val);
    }
}

void insert_lnum(char *object_str, const char *mtc_str, int32_t number, int show_sign) {
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
        vtype str1, str2;

        (void)strncpy(str1, object_str, string - object_str);
        str1[string - object_str] = '\0';
        (void)strcpy(str2, string + mlen);

        if ((number >= 0) && (show_sign)) {
            (void)sprintf(object_str, "%s+%d%s", str1, number, str2);
        } else {
            (void)sprintf(object_str, "%s%d%s", str1, number, str2);
        }
    }
}
