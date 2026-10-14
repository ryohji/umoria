// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// What this character adds to a blow's aim and to its force

#ifndef PLAYER_ATTACK_BONUSES_H
#define PLAYER_ATTACK_BONUSES_H

// THE QUESTION. What does this character add to every attempt to hit, and what to
// every blow that lands? Two small numbers, typically between -6 and +9: they come
// from the strength and dexterity tables (stats.c's tohit_adj() and todam_adj())
// and then grow or shrink by whatever is worn and wielded. NEGATIVE IS ORDINARY
// HERE, not an edge case -- a weak or clumsy character starts at -3, and a cursed
// weapon takes more away.
//
// TWO NUMBERS, AND BOTH OF THEM ARE ANSWERS. NOT ONE READER ADDS THEM UP: a
// reader always knows whether it is aiming or hurting.
//
//   - THE AIM is multiplied by BTH_PLUS_ADJ on the character sheet, added whole to
//     a swing's to-hit roll, and added whole to a throw's.
//   - THE FORCE is added to the damage of a blow that has already landed. Bows
//     never get it: an arrow's damage comes from the arrow.
//
// SO WHY ONE MODULE AND NOT TWO? Because the WRITERS never separate. All four
// places that write set both, in two adjacent lines: creation twice (a guess and
// then the real values), the recalculation in player_bonuses.c, and the saved file's two
// adjacent shorts. Two questions that share every way of being replaced are one
// question with two answers.
//
// WHO ASKS: the character sheet's two ratings (abilities.c), a swing (player_melee.c,
// once for the aim and once for the force), a throw (throw.c, the aim only), the
// copy the sheet shows while equipment is being counted
// (player_display_numbers.c), and the saved file.
//
// THE DISPLAYED PLUSSES ARE A DIFFERENT QUESTION and they already have their own
// module: player_display_numbers.c holds what the sheet shows, which counts only
// the items the character KNOWS the worth of. These two numbers are the real ones,
// and they count everything.

// The two answers. SIX CALLERS FOR THE AIM (the sheet's two ratings, folded into
// one read; a swing; a throw; the sheet's starting copy, twice; the saved file) and
// FOUR FOR THE FORCE (a blow that lands, the sheet's starting copy twice, the saved
// file).
//
// Both are ints: every caller drops the number straight into a wider sum
// (`tot_tohit += …`, `k += …`, `bth + … * BTH_PLUS_ADJ`).
int player_to_hit_bonus(void);
int player_to_damage_bonus(void);

// Both numbers outright. FOUR CALLERS: creation's first guess (create.c:122), the
// real values it settles on (create.c:397), the recalculation that runs whenever
// equipment changes (player_bonuses.c), and the saved file's two shorts put back.
//
// ONE WINDOW FOR THE PAIR, because no caller has ever set one alone -- every writer
// is two adjacent lines, and the saved file holds the pair in two adjacent shorts
// (the aim first, and that order cannot move). Loading a saved game uses this same
// window.
//
// IT REFUSES NOTHING (findings.md 24). WIDTH IS STILL A SHORT: 32768 lands on
// -32768.
void player_attack_bonuses_set(int to_hit, int to_damage);

// This much more from one piece of equipment, or less if it is negative. ONE CALLER
// EACH, both of them the same loop in player_bonuses.c's calc_bonuses(), which walks the
// worn and wielded items and adds each one's tohit and todam.
//
// TWO WINDOWS AND NOT ONE PAIRED WINDOW, unlike the setter, because THE LOOP DOES
// NOT ALWAYS ADD BOTH: a bow's todam is skipped ("Bows can't damage. -CJS-"), and
// an item's two amounts are different numbers anyway.
//
// THE `_adjust` WINDOWS EXIST because somebody adds to these numbers (findings.md
// 45). The condition that skips bows stays in the caller: it is a fact
// about bows, not about this character.
void player_to_hit_bonus_adjust(int amount);
void player_to_damage_bonus_adjust(int amount);

// WHAT THIS MODULE DOES NOT ANSWER -- four things, all still in the callers:
//
//   1. WHERE THE NUMBERS COME FROM. stats.c's tohit_adj() and todam_adj() read the
//      strength and dexterity tables; player_bonuses.c reads each item's own tohit and
//      todam. This module is told the answers, it does not derive them.
//   2. WHAT THE CHARACTER SHEET SAYS. abilities.c multiplies the aim by
//      BTH_PLUS_ADJ and adds the class's per-level column before likert() turns the
//      total into words. That sum is the sheet's question (player_base_to_hit.h).
//   3. WHAT THE SHEET SHOWS WHILE EQUIPMENT IS COUNTED. player_display_numbers.c
//      answers that, and it deliberately differs from the real numbers: it counts
//      only what the character knows the worth of.
//   4. WHEN THE NUMBERS ARE REBUILT. calc_bonuses() and py_bonuses() in player_bonuses.c
//      decide when equipment must be counted again; this module only holds what
//      they work out.

#endif // PLAYER_ATTACK_BONUSES_H
