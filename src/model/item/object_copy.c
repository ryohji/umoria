// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Copying an object template from object_list into an inven_type

#include "config.h"
#include "constant.h"
#include "types.h"
#include "externs.h"

#include "object_copy.h"

// Copy fields from an object_list template into an inven_type.
// Used to initialize inventory slots, floor items, and store items from
// the definition table.
void invcopy(inven_type *to, int from_index) {
    treasure_type *from = &object_list[from_index];

    to->index       = from_index;
    to->name2       = SN_NULL;
    to->inscrip[0]  = '\0';
    to->flags       = from->flags;
    to->tval        = from->tval;
    to->tchar       = from->tchar;
    to->p1          = from->p1;
    to->cost        = from->cost;
    to->subval      = from->subval;
    to->number      = from->number;
    to->weight      = from->weight;
    to->tohit       = from->tohit;
    to->todam       = from->todam;
    to->ac          = from->ac;
    to->toac        = from->toac;
    to->damage[0]   = from->damage[0];
    to->damage[1]   = from->damage[1];
    to->level       = from->level;
    to->ident       = 0;
}
