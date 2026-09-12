// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The town's shops: where their records live

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "stores.h"

// Zero at the start of a run, which is what the old-version save file path
// counts on: it fills in the shops it knows about and leaves the rest empty.
// store_init() overwrites all of them at the start of a new game.
static store_type stores[MAX_STORES];

int store_count(void) {
    return MAX_STORES;
}

store_type *store_at(int index) {
    return &stores[index];
}
