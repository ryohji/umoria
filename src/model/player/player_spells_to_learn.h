// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How many more spells (or prayers) the character may still learn

#ifndef PLAYER_SPELLS_TO_LEARN_H
#define PLAYER_SPELLS_TO_LEARN_H

// THE NAME. It is a COUNT, not a list of spells -- which spells are known lives
// elsewhere (spells_known.h).
//
// THE QUESTION. How many more spells may be learned right now? calc_spells()
// works out how many the character's class, level and stat allow, subtracts the
// ones already known, and stores the remainder here. gain_spells() spends it,
// one spell at a time, when the player asks to study.
//
// It is a CHARGE, like the glowing hands (player_glowing_hands.h), but it can be
// more than one, and unlike the hands it is RECOMPUTED every time the character
// gains a level -- calc_spells() may raise it, lower it, or set it to zero
// (forgetting spells when a drained stat no longer allows them).

// How many more may be learned. Zero means none: the status line leaves the
// "Study" slot blank, and asking to study says so.
int player_spells_to_learn(void);

// Store a new count. Replacing, not adding -- both callers have already worked
// out the whole number:
//
//   - calc_spells() computes it from the class, the level and the stat.
//   - gain_spells() adds back what could not be learned for want of a book.
//   - loading a saved game puts back the byte it wrote.
void player_spells_to_learn_set(int count);

// WHAT THIS MODULE DOES NOT ANSWER -- five things, all of them still in the
// callers:
//
//   1. HOW MANY ARE ALLOWED. The nine-way table in calc_spells() reads the class
//      table, the stat and the level. One caller, one place.
//   2. WHICH SPELLS ARE KNOWN. That is spells_known.h. calc_spells() subtracts
//      learned_spell_count() to make this number, but the two are not halves of
//      one fact: this count is remembered (it is in the saved file) and spent by
//      gain_spells(), not recomputed from the known spells whenever it is read.
//   3. THE BOOK THAT IS MISSING. gain_spells() can be allowed more spells than
//      the books at hand contain; it learns what it can and adds the difference
//      back. "Which spells are in this book" is the book's business.
//   4. THE THREE MESSAGES. "You can learn some new %ss now.", "You can't learn
//      any new %ss!" and "You seem to be missing a book." belong to msg_print(),
//      which lives behind externs.h, and this module does not include it.
//   5. REDRAWING THE STATUS LINE. PY_STUDY is already its own question
//      (player_status_flags.h). The two move together, but whether the screen
//      needs repainting is about showing, not about the count.

#endif // PLAYER_SPELLS_TO_LEARN_H
