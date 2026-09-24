// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Which of the 31 spells (or prayers) of the character's class they know: the
// ones they have learned, the ones they have made work at least once, the ones
// they knew and have since forgotten, and the order in which they learned them.

#ifndef SPELLS_KNOWN_H
#define SPELLS_KNOWN_H

// This is storage and only storage, the same as progress.h, score_death.h,
// hp_table.h, player_light.h and burden.h. Nothing here is derived: what the
// character has learned is history, and the only way to find out is to have
// kept it.
//
// Four globals used to hold it -- three bit fields over the 31 spells and one
// list of 32 spell numbers -- and they belong together because they answer one
// question between them ("what does this character know, and in what order did
// they come to know it?"). calc_spells() (misc3.c) moves spells between learned
// and forgotten as the character's level changes, gain_spells() appends to the
// learned list, and magic.c / prayer.c mark a spell as having worked.
//
// The bit arithmetic (1L << spell) is on this side of the windows. The callers
// ask about a spell by its number, which is what they have: the index into
// magic_spell[] and into spell_names[]. Three of them used to spell out the
// shift themselves, and two had to remember that shifting by SPELL_NONE would
// be undefined.

// The value that stands for "no spell learned here yet" in the learned-in order
// (spell_order). The order is filled with it at the start of a game
// (main.c) and it is written to the save file as-is, so it cannot change.
#define SPELL_NONE 99

// --- what the character knows --------------------------------------------

// True while the character can cast the spell right now. Spells above their
// level are moved out of here (into forgotten) by calc_spells(); learning one
// (gain_spells) or remembering one (calc_spells) moves it back in.
//
// SPELL_NONE answers false, which is what lets the callers walk the
// learned-in order without checking for the marker first.
bool spell_is_learned(int spell);

// True while the character knew the spell and has lost it to a level or a
// statistic change. Shown as " forgotten" in the spell list (misc3.c).
bool spell_is_forgotten(int spell);

// True once the spell has been cast successfully. The first success is worth
// experience (magic.c, prayer.c), so this is what stops it being paid twice;
// until then the spell list shows " untried".
bool spell_has_worked(int spell);

// --- what the character has been through ---------------------------------

// The character knows at least one spell. calc_mana() (misc3.c) uses it to
// decide whether they have any mana at all, and the forgetting loop in
// calc_spells() uses it to stop once nothing is left to forget.
bool any_spell_learned(void);

// The character has forgotten at least one spell, so there is something to
// remember when their level allows it again (calc_spells).
bool any_spell_forgotten(void);

// How many spells the character knows. calc_spells() compares it against how
// many they are allowed to know.
int learned_spell_count(void);

// --- moving spells in and out --------------------------------------------

// Learn a spell for the first time: mark it known *and* append it to the
// learned-in order. The two always happened together (gain_spells did it
// twice, once for a chosen spell and once for a granted prayer), and keeping
// them together is what guarantees the order stays usable.
void spell_learn(int spell);

// Lose a spell the character knew: known goes off, forgotten goes on. The
// learned-in order is left alone, which is how calc_spells() knows which
// spell to give back first.
void spell_forget(int spell);

// Give back a spell the character had forgotten: forgotten goes off, known
// goes on. The mirror image of spell_forget().
void spell_remember(int spell);

// Record that the spell has worked, so the experience for it is only paid once.
void spell_mark_worked(int spell);

// --- the order the spells were learned in --------------------------------

// The spell the character learned nth (0 is the first one they ever learned),
// or SPELL_NONE if they have not learned that many. Forgotten spells stay in
// the order, so this is a history and not a list of what they can cast.
int spell_learned_nth(int n);

// Forget the whole history: no spells learned yet. Called once, when a
// character starts out (main.c).
void spell_order_forget_all(void);

// --- matching against a set of spells ------------------------------------

// The spells in the given set that the character knows. The set comes from a
// spell book (inven_type.flags holds one bit per spell), so this answers "which
// of the spells in this book can I cast?" (moria3.c).
uint32_t spells_learned_among(uint32_t spells);

// The spells in the given set that the character does not know yet -- the
// candidates for learning (misc3.c, twice: from a book for a mage, from all
// spells for a priest).
uint32_t spells_not_learned_among(uint32_t spells);

// --- the save file -------------------------------------------------------

// The three bit fields and the learned-in order as they are written to and
// read from the save file (save.c). These are the only windows that hand over
// the raw representation, and they exist because the file format is fixed: the
// three words and then the 32 bytes, in that order.
uint32_t spells_learned_bits(void);
uint32_t spells_worked_bits(void);
uint32_t spells_forgotten_bits(void);
void spells_set_learned_bits(uint32_t bits);
void spells_set_worked_bits(uint32_t bits);
void spells_set_forgotten_bits(uint32_t bits);

// The learned-in order as 32 bytes, for wr_bytes() and rd_bytes(). A pointer
// is handed out because that is what those two take; nothing else should use
// it (spell_learned_nth() answers the question this array exists for).
uint8_t *spell_order_bytes(void);

#endif // SPELLS_KNOWN_H
