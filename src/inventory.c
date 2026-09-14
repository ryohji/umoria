// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// What the player carries and wears: where the record of it lives

#include "config.h"
#include "constant.h"
#include "types.h"

#include "equipment.h"
#include "inventory.h"

// No externs.h here, the same as panel.c, stores.c, options.c and stats.c: a
// place to keep objects needs nothing from the rest of the game. Nothing at
// all is declared from outside now, so this file compiles on its own.
//
// The state is owned here and is static: the only way in is through the
// windows below. One array holds both, pack in 0..INVEN_WIELD-1 and worn gear
// in INVEN_WIELD..INVEN_ARRAY_SIZE-1, because the save file serialises it as
// one run and splitting the array would change that order.
static inven_type inventory[INVEN_ARRAY_SIZE];
static int16_t inven_ctr    = 0; // how many pack slots are used
static int16_t inven_weight = 0; // how heavy the pack is
static int16_t equip_ctr    = 0; // how many equipment slots are filled

// --- the pack ------------------------------------------------------------

inven_type *inventory_at(int index) {
    return &inventory[index];
}

int inventory_slot_count(void) {
    return INVEN_WIELD;
}

int inventory_count(void) {
    return inven_ctr;
}

void inventory_set_count(int count) {
    inven_ctr = (int16_t)count;
}

int inventory_weight(void) {
    return inven_weight;
}

void inventory_set_weight(int weight) {
    inven_weight = (int16_t)weight;
}

// --- the equipment -------------------------------------------------------

inven_type *equipment_at(int index) {
    return &inventory[index];
}

int equipment_first_slot(void) {
    return INVEN_WIELD;
}

int equipment_end_slot(void) {
    return INVEN_ARRAY_SIZE;
}

int equipment_count(void) {
    return equip_ctr;
}

void equipment_set_count(int count) {
    equip_ctr = (int16_t)count;
}

// --- both at once --------------------------------------------------------

inven_type *inventory_and_equipment_at(int index) {
    return &inventory[index];
}

int inventory_and_equipment_slot_count(void) {
    return INVEN_ARRAY_SIZE;
}
