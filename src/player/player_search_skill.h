// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How well and how often this character looks for what is hidden

#ifndef PLAYER_SEARCH_SKILL_H
#define PLAYER_SEARCH_SKILL_H

// THE QUESTION. When this character looks for a hidden door or a trap, how likely
// is the look to succeed -- and how often does a look happen without being asked
// for? Two small numbers, both of them from the race table and the class table and
// then moved by whatever is worn.
//
// THE TENTH QUESTION OUT OF struct misc, after how deep the character has been, the
// hit die, the armour class, the base to-hit, the disarming skill, the saving
// throw, the race, the body's weight and the attack bonuses.
//
// TWO NUMBERS, AND BOTH OF THEM ARE ANSWERS -- the same shape as the base to-hit
// (player_base_to_hit.h) and the attack bonuses (player_attack_bonuses.h), and the
// opposite of the armour class (where two fields held one answer because every
// reader added them up). NOT ONE READER HERE COMBINES THEM. The two do stand next
// to each other in one place (moria3.c, and again in terrain_commands.c), and even
// there they are asked different things: THE FREQUENCY DECIDES WHETHER TO LOOK AT
// ALL, AND THE CHANCE DECIDES WHETHER THE LOOK FINDS ANYTHING.
//
//   - THE CHANCE is handed straight to search(). Bigger is better.
//   - THE FREQUENCY is a one-in-n: a look happens when randint(n) comes up 1, so
//     SMALLER IS BETTER, and the character sheet has to turn it upside down
//     (40 - frequency) before it can be read as a rating.
//
// SO WHY ONE MODULE AND NOT TWO? Because the two are almost always moved together.
// The class table adds to both, and a piece of searching gear moves both -- in
// OPPOSITE DIRECTIONS, by the same amount. Only one writer in the whole game ever
// touches one alone, and it is the debugging editor.
//
// WHO ASKS: search mode's own turn (dungeon.c), the automatic look while running
// (moria3.c) and while resting or tunnelling (terrain_commands.c), the character
// sheet's two ratings (abilities.c, which is where the upside-down arithmetic
// lives), the debugging editor (wizard.c), and the saved file.
//
// NOT THE SAME QUESTION AS "IS THIS CHARACTER SEARCHING RIGHT NOW". That is a flag,
// it already has a window (player_is_searching() in player_status_flags.h), and
// search_on() / search_off() turn it on and off. This module holds how good the
// looking is, not whether it is happening -- which is why the file is not called
// player_searching.c.

// The two answers. SIX CALLERS FOR THE CHANCE (search mode's turn, the two
// automatic looks, the character sheet, the debugging editor's prompt, the saved
// file) and THREE FOR THE FREQUENCY (the automatic look, which asks twice in one
// condition and has it folded to one read, the character sheet, the saved file).
// Both counts were checked against the callers after #18-12-25B; the step-A note
// said four for the frequency and listed three.
//
// Both are ints, though the fields are int16_t: every caller either passes the
// number to a function taking int (`search(row, col, chance)`, `randint(n)`) or
// drops it into wider arithmetic (`40 - frequency`).
int player_search_chance(void);
int player_search_frequency(void);

// Either number outright. THREE CALLERS FOR THE CHANCE (creation reads the race
// table, the debugging editor, the saved file) and TWO FOR THE FREQUENCY (creation,
// the saved file).
//
// TWO WINDOWS AND NOT ONE PAIRED WINDOW, unlike the attack bonuses, because ONE
// WRITER SETS ONE ALONE: wizard.c walks Gold, then Searching, then Stealth, and it
// never asks for the frequency. Creation's two are three lines apart as well (the
// base to-hit's window sits between them), so nothing here is written as a pair.
//
// THEY REFUSE NOTHING, because the fields refused nothing (ledger observation 24).
// The debugging editor bounds its own prompt at 0..200 before it calls, and that
// bound is the editor's, not this question's.
void player_search_chance_set(int chance);
void player_search_frequency_set(int frequency);

// Both numbers move at once. TWO CALLERS, and each hands in two amounts:
//
//   - creation adds the class's modifiers (msrh and mfos, BOTH POSITIVE),
//   - putting on or taking off a piece of searching gear adds the item's amount to
//     the chance and TAKES THE SAME AMOUNT OFF the frequency.
//
// ONE PAIRED WINDOW, unlike the setters, because NO CALLER EVER ADDS TO ONE ALONE.
// The two amounts are separate parameters and not one, because THE CALLER OWNS THE
// SIGNS: py_bonuses() passes (amount, -amount) and creation passes (msrh, mfos),
// and if this window negated the second amount itself then creation would be
// telling a lie. "Searching gear makes you find more and look more often" is a fact
// about the gear, and it stays where the gear is.
void player_search_skill_adjust(int chance_amount, int frequency_amount);

// WHAT THIS MODULE DOES NOT ANSWER -- four things, all still in the callers:
//
//   1. WHERE THE NUMBERS COME FROM. create.c reads the race table's srh and fos and
//      the class table's msrh and mfos; player_bonuses.c reads the item's own amount. This
//      module is told the answers, it does not derive them.
//   2. WHAT THE CHARACTER SHEET SAYS. abilities.c turns the frequency upside down
//      (40 - frequency) and floors the result at 0 before likert() makes words of
//      it, and it shows the chance unchanged. THE UPSIDE-DOWN ARITHMETIC IS THE
//      SHEET'S QUESTION, and so is the floor -- the same division of labour as the
//      attack bonuses (where BTH_PLUS_ADJ and a damage floor stayed with callers).
//   3. WHETHER A LOOK HAPPENS THIS TURN. randint() and the searching flag decide
//      that (moria3.c, terrain_commands.c, dungeon.c, player_status_flags.h).
//   4. WHAT A LOOK FINDS. search() in moria3.c takes the chance and works against
//      the dungeon; this module never sees a square.

#endif // PLAYER_SEARCH_SKILL_H
