// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// What the character can do and resist because of what is being worn:
// seventeen yes-or-no answers, worked out again from the equipment every time

#ifndef PLAYER_ABILITIES_H
#define PLAYER_ABILITIES_H

// SEVENTEEN yes-or-no answers about what the character can do and resist.
//
// THE ANSWER IS NOT KEPT, IT IS DERIVED. These seventeen are WORKED OUT FROM THE
// EQUIPMENT, and calc_bonuses() in player_bonuses.c does it from scratch every
// time anything changes: it clears all seventeen and sets them again from the
// flags of what is worn.
// ALL THIRTY-NINE OF player_bonuses.c'S REFERENCES ARE THAT ONE FUNCTION.
// These seventeen are in the save file, though nothing calls calc_bonuses()
// when a game is loaded.
//
// THE TWO THINGS THAT ARE NOT HERE.
//
//   - confuse_monster ("glowing hands", eight references) is a CHARGE THAT IS
//     SPENT: scrolls.c sets it, the next blow that lands clears it. No equipment
//     grants it and calc_bonuses() does not touch it. It goes with the timed
//     states (player_timed_effects.h).
//   - new_spells (nine references, plus twenty in tests/) is a COUNT, not an
//     ability -- how many spells may still be learnt. It pairs with the study
//     request and spells_known.c, and has its own module (player_spells_to_learn.h).
//
// The seventeen fall into four kinds, and the reading windows are named after
// what the caller wants to know, not after the field:
//
//   1. WHAT CANNOT BE DONE TO THE CHARACTER (seven fields, two windows): never
//      paralyzed (free_act, eleven references -- the most-touched of the
//      seventeen), and the six sustained stats, which are ONE window taking a
//      stat index.
//   2. WHAT DAMAGE IS SHRUGGED OFF (five): fire, cold, acid, light and falling.
//   3. WHAT THE BODY DOES DIFFERENTLY (four): sees invisible creatures,
//      regenerates, digests slowly, teleports at random.
//   4. WHAT THE MONSTERS NOTICE (one): aggravation.
//
// What this module deliberately does NOT do:
//
//   - walk the equipment. player_bonuses.c gathers the flags of what is worn and hands
//     the result over; a window that walked the slots itself would have to know
//     about equipment, identification and curses,
//   - the digestion. calc_bonuses() asks whether the character digests slowly or
//     regenerates BEFORE it clears the seventeen, so that the old answer's effect
//     on the digestion rate can be taken back, and pays the new one in again at
//     the end. The rate is the stomach's question (player_food.c) and the undoing
//     stays in the caller -- WHICH IS WHY CLEARING AND DERIVING ARE TWO WINDOWS
//     AND NOT ONE: work happens between them.
//   - know why. Nothing here announces anything or rolls anything.

// No includes: the callers bring config.h, constant.h and types.h in first.
// types.h has no guard of its own.

// --- what the character can do right now ---------------------------------

// Sees creatures that are invisible. TWO THINGS GRANT THIS, which is why it has
// a window of its own below: a worn item (TR_SEE_INVIS) and the timed "detect
// invisible" potion, whose counter is a different question.
bool player_can_see_invisible(void);

// Cannot be held still: monsters' paralysing attacks, the paralysing trap, the
// sleep potion and the gas trap all ask.
bool player_never_paralyzed(void);

// Shrugs off the four kinds of damage that destroy what is carried, and the
// fall down a trap door or a shaft.
bool player_resists_fire(void);
bool player_resists_cold(void);
bool player_resists_acid(void);
bool player_resists_light(void);
bool player_takes_no_falling_damage(void);

// Heals faster and eats more for it; eats less than usual; is dragged somewhere
// else now and then.
bool player_regenerates(void);
bool player_has_slow_digestion(void);
bool player_teleports_randomly(void);

// Wakes monsters that would have slept.
bool player_aggravates_monsters(void);

// Whether a stat cannot be drained. THE CALLER NAMES THE STAT THE WAY THE REST
// OF THE GAME DOES (A_STR to A_CHR); the item's own numbering, which is not the
// same order, stays inside this module.
bool player_stat_sustained(int stat);

// --- working the seventeen out again -------------------------------------

// Forgets all seventeen. calc_bonuses() starts here, then does the armour and
// the to-hit numbers, then tells this module what is worn.
void player_abilities_forget_all(void);

// Grants whatever the given equipment flags grant: eleven of the seventeen come
// straight from the OR of the flags of everything worn. WHICH FLAG MEANS WHICH
// ABILITY IS THIS MODULE'S BUSINESS -- no TR_* constant appears in a caller.
void player_abilities_note_item_flags(uint32_t item_flags);

// Grants the sustain that one worn item carries. The caller passes the item's
// own p1, which numbers the six stats in the ITEM'S order (1 to 6); a value
// outside that range grants nothing, as before. The six are separate flags
// because an item sustains exactly one stat.
void player_abilities_note_sustain(int item_p1);

// Grants "sees invisible" from somewhere that is not equipment: the timed potion
// while its counter is running.
void player_grant_see_invisible(void);

// --- the save file -------------------------------------------------------

// The seventeen are seventeen bytes in the save file, in one run, in the order
// this module keeps them. THE ORDER IS THE FORMAT, so these two windows are what
// the save file's writer and reader use and nothing else.
#define PLAYER_ABILITIES_SAVED_BYTES 17

uint8_t player_abilities_saved_byte(int position);
void player_abilities_restore_byte(int position, uint8_t value);

#endif
