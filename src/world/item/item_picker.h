// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Item picker and display: showing inventory/equipment and choosing an item

#ifndef ITEM_PICKER_H
#define ITEM_PICKER_H

#include <stdbool.h>

// Display inventory items from r1 to r2.
// Returns the left column used.
int show_inven(int r1, int r2, bool weight, int col, const char *mask);

// Return a string describing how a given equipment item is carried.
const char *describe_use(int i);

// Display equipment items.
// Returns the left column used.
int show_equip(bool weight, int col);

// Remove item from equipment list.
void takeoff(int item_val, int posn);

// Get the ID of an item and return whether an item was chosen.
// Sets *com_val to the chosen item index.
// Asks "<prompt> <item description>?" and returns the answer.
int confirm_item(const char *prompt, int item);

int get_item(int *com_val, const char *pmt, int i, int j, const char *mask, const char *message);

#endif // ITEM_PICKER_H
