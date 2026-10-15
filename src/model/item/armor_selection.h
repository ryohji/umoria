// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Selecting a random worn armor piece, or the first cursed one.

#ifndef ARMOR_SELECTION_H
#define ARMOR_SELECTION_H

// Returns the equipment slot index of a random worn armor piece (one of
// INVEN_BODY, INVEN_ARM, INVEN_OUTER, INVEN_HANDS, INVEN_HEAD, INVEN_FEET),
// or 0 if no armor is worn. Each worn piece has equal probability.
//
// Calls randint() exactly once if at least one armor piece is worn, and does
// not call it if none are worn.
int pick_random_worn_armor(void);

// Returns the equipment slot index of the first cursed armor piece in the
// order INVEN_BODY, INVEN_ARM, INVEN_OUTER, INVEN_HEAD, INVEN_HANDS,
// INVEN_FEET, or 0 if no cursed armor is worn.
//
// Does not call randint().
int pick_first_cursed_armor(void);

#endif // ARMOR_SELECTION_H
