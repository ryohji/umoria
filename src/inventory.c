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
// place to keep objects needs nothing from the rest of the game. That leaves
// four symbols to declare, so they are declared here rather than dragging in
// the global header (which pulls ncurses along with it) for four lines. The
// declarations therefore appear twice, here and in externs.h; the definitions
// are still the ones in treasure.c, so the two can not drift into different
// objects.
//
// The array is not owned here yet on purpose. Moving the definition out of
// treasure.c and making it static is a separate step: two definitions would
// mean two copies of the state, and the game and the tests would disagree
// about what the player is carrying.
extern inven_type inventory[INVEN_ARRAY_SIZE];
extern int16_t inven_ctr;    // how many pack slots are used
extern int16_t inven_weight; // how heavy the pack is
extern int16_t equip_ctr;    // how many equipment slots are filled

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
