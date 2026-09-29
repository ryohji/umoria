// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How much gold the player is carrying: where it is kept

#ifndef PLAYER_GOLD_H
#define PLAYER_GOLD_H

// One question, one module. `py` is a struct of eighty-odd fields answering
// dozens of unrelated questions, so it is not being replaced by one module;
// each question leaves it separately, and this is the first. The purse is the
// smallest and the most independent of them: twenty-two places read or write
// it, none of them alongside another field of the struct.

// This is storage and only storage. The windows do not clamp, do not refuse a
// negative, and do not guard against overflow, because the old code did none
// of those things here and doing them here would change behaviour rather than
// preserve it. Every rule about the purse has exactly one reader and stays
// with it:
//
//   - a shop refuses a purchase you cannot afford (store2.c asks first),
//   - a thief takes a tenth and cannot take more than you have (creature.c
//     compares first),
//   - a new character is given at least 80 gold (create.c),
//   - the wizard command refuses a negative (wizard.c).

// What is in the purse. Shown on the status line, in the character sheet, in
// the shop, on the tombstone, and divided by 100 for the death score; the
// dividing stays at that one call site.
int32_t player_gold(void);

// Gold comes in: found on the floor, paid by a shopkeeper for an item, or
// handed over by the "die of old age" wizard command.
void player_gain_gold(int32_t amount);

// Gold goes out: the price of a purchase, or what a thief made off with. The
// caller has already made sure there is that much to take.
void player_pay_gold(int32_t amount);

// Puts an amount in the purse outright, replacing what was there. Used where
// the number does not come from the old one: character creation, the wizard
// command, the save file's reader, and the thief who empties it.
void player_set_gold(int32_t amount);

#endif
