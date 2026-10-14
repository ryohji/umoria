// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How many faces this character's hit die has

#ifndef PLAYER_HIT_DIE_H
#define PLAYER_HIT_DIE_H

// THE QUESTION. How many faces does the die have that this character's hit
// points are rolled from? THE UNIT IS FACES, not hit points and not a die --
// `randint(faces)` is rolled once per level, so the number is the largest a
// single level can add.
//
// Not a body part or a skill: the SHAPE OF ONE ROLL.
//
// TWO TABLES DECIDE IT, both at character creation and never again:
//
//   - THE RACE gives the base (race_type.bhitdie): Halfling 6, Gnome 7, Elf 8,
//     Half-Elf 9, Dwarf 9, Human 10, Half-Orc 10, Half-Troll 12.
//   - THE CLASS adds to it (class_type.adj_hd): Mage 0, Priest 2, Ranger 4,
//     Rogue 6, Paladin 6, Warrior 9.
//
// So the number lands between 6 (a Halfling Mage) and 21 (a Half-Troll
// Warrior), and after create.c has rolled the hit point table NOTHING MOVES IT
// AGAIN for the rest of the game.
//
// NOBODY READS IT WHILE THE GAME IS BEING PLAYED. All five readers sit in the
// twenty lines of create.c that roll the hit points, and the only other two
// places are the saved file. THAT STILL MAKES IT A QUESTION rather than a byte
// the save file happens to need (findings.md 37). Here two tables decide it, and
// the whole hit point table is built out of it.

// How many faces. Five callers, all of them in create.c's hit point rolling:
// the level-one hit points, the two bounds on the total, the level-one row of
// the table, and the per-level `randint()`.
int player_hit_die(void);

// How many faces, as an outright answer. TWO CALLERS, the same shape as
// player_infra_range_set(): the race's base when the character is made
// (create.c), and the saved file's reader.
//
// The save file's restore is THE SAME SENTENCE as the race's base -- both are a
// plain replacement. (player_max_depth_set() had to be a window of its own
// because the other writer there kept the deeper of two numbers; there is no
// such rule here.)
void player_hit_die_set(int faces);

// More faces by this many -- the class's adjustment, one caller (create.c),
// applied straight after the race's base is in place. A window of its own
// rather than read-add-write, so the store is touched once.
//
// NOTHING REFUSES A NEGATIVE: every adj_hd in the class table is zero or more.
void player_hit_die_adjust(int faces);

// WHAT THIS MODULE DOES NOT ANSWER -- four things, all still in the callers:
//
//   1. THE HIT POINTS THEMSELVES. create.c hands con_adj() + this number to
//      player_reset_hp(); the constitution bonus is a different question
//      (player_hp.h).
//   2. THE BOUNDS ON THE TOTAL. "only succeed if it is within 1/8 of average
//      value" is a rule about REROLLING, and its one reader is create.c.
//   3. THE HIT POINT TABLE. hp_table.c already keeps the total at each level.
//      The die is what the table is rolled FROM, not the table.
//   4. THE TWO TABLES. race_type.bhitdie and class_type.adj_hd are globals of
//      their own (`race` and `class`, read-only data). This module keeps the
//      answer, not where the answer came from.

#endif // PLAYER_HIT_DIE_H
