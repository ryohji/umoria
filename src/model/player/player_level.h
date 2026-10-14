// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How far the character has come: the level, the experience behind it and the
// price of the next level

#ifndef PLAYER_LEVEL_H
#define PLAYER_LEVEL_H

// THE QUESTION. Five numbers answer it together: the level, the experience behind
// it, the most experience ever held, the fraction of a point not yet added up, and
// the factor that scales the table.
//
// ONE OF THE FIVE NUMBERS IS ALSO A CONCLUSION. The
// level is stored (the save file reads and writes it) and yet it is fixed by the
// experience:
//
//     advance_price(level) = player_exp[level - 1] * expfact / 100
//     lev                  = the lowest level whose advance price is still
//                            more than the experience in hand
//
// The table entry is the price of LEAVING a level, not of reaching it -- which is
// why every call site subtracts one from the level before subscripting, and why
// the character sheet prints the entry as "Exp to Adv.".
//
// That one promise was kept in two different ways, and nothing made the two
// agree:
//
//   - prt_experience() (level_ops.c) CLIMBS, one level at a time, calling
//     gain_level() for each step,
//   - lose_exp() (spells.c) COUNTS AGAIN from the bottom, walking the table from
//     the first entry until the price is too high.
//
// The price itself was written out five times (misc3.c three times, files.c and
// spells.c once each), and the spells.c copy starts its subscript at a different
// end from the other four. So the promise is what this module is for: the price
// is computed in one place (player_experience_to_advance_from), the climb asks
// one question (player_deserves_next_level) and the count-again is one window
// (player_recompute_level). Two routes to the same promise, both now going
// through the same arithmetic.
//
// The level being stored is not an accident, and this module does not try to
// turn it into a pure conclusion: the wizard's "never die" trick (death.c) adds
// MAX_PLAYER_LEVEL to the level and five million to the experience on purpose,
// which leaves the promise broken until something recounts. That is why the
// plain setters at the bottom exist and why the wizard commands use them: the
// bare setter is how the code says "this is deliberately not the level the
// experience deserves".
//
// The fraction is NOT the fraction the mana and the hit points keep. Theirs is a
// share of a maximum in 65536ths, handed back by regeneration. This one is the
// REMAINDER OF A DIVISION whose divisor is the current level: killing a monster
// is worth mexp * its level, split by how far the character has already come, so
// what does not divide evenly is kept in 65536ths until it adds up to a point.
// One caller writes it (mon_take_hit in monster_damage.c) and one reads it (the save
// file). The mana's and the hit points' windows cannot be copied here.
//
// Five rules live inside:
//
//   - the price of leaving a level (player_experience_to_advance_from), the one
//     copy of the formula the five call sites shared,
//   - experience arriving whole (player_gain_experience) and experience arriving
//     as a share to be divided (player_gain_shared_experience, which is the
//     fixed-point remainder above),
//   - experience going away (player_lose_experience), which stops at nothing and
//     never goes below it,
//   - the promise between the level and the experience, from both ends
//     (player_deserves_next_level for the climb, player_recompute_level for the
//     count-again, player_trim_surplus_experience for the half of the surplus
//     that gaining a level throws away),
//   - the high-water mark (player_record_max_experience and
//     player_restore_experience): the most experience ever held, which is what
//     restore-level puts back and what drain-life cannot take.
//
// What the rules deliberately do NOT do:
//
//   - climbing. player_deserves_next_level() answers a question; the loop that
//     asks it again belongs to prt_experience(), because each step prints
//     "Welcome to level n", recalculates the hit points, the spells and the
//     mana, and redraws the level and the title. None of that is a question
//     about a number,
//   - what a new level is worth (the hit-point table, the spell counts, the
//     mana) -- other questions, other modules,
//   - printing. prt_level(), prt_title() and prt_experience()'s own drawing stay
//     with the callers,
//   - where the experience comes from: monsters, traps, potions, first castings
//     and identifying an item all work out their own amount and hand over a
//     number.

// How far the character has come. Read from everywhere -- spell strength, device
// chances, the character sheet, the dump, the save file and the high-score
// record -- and by a long way the most-read of the five numbers.
uint16_t player_level(void);

// The experience behind that level.
int32_t player_experience(void);

// The most experience ever held. Restore-level puts the experience back to it;
// nothing else ever lowers it.
int32_t player_max_experience(void);

// The part of a point of experience that has not added up yet. Only mon_take_hit
// writes it and only the save file reads it.
uint16_t player_experience_fraction(void);

// How much dearer this character's levels are than the plain table: the race's
// base plus the class's share, a percentage. Never below a hundred, which is the
// one thing that keeps the count-again below from walking off the end of the
// table (see player_level.c).
uint8_t player_experience_factor(void);

// What leaving the given level costs this character: the table entry scaled by
// the factor. THE ONE COPY of a formula that was written out five times. The
// argument is a level, counting from one, not a subscript.
int32_t player_experience_to_advance_from(uint16_t level);

// What leaving the level the character is on costs -- the number the character
// sheet and the dump print as "Exp to Adv.".
int32_t player_experience_needed_to_advance(void);

// Experience arriving whole: traps, potions, a first casting, an item whose
// effect has just become clear. Nothing is capped here; the cap belongs to the
// climb (player_cap_experience).
void player_gain_experience(int32_t amount);

// Experience arriving as a share to be divided by how far the character has
// already come -- a dead monster is worth mexp * its level, and a character
// twice as far along collects half of it. What does not divide evenly is kept in
// 65536ths until it adds up to a whole point, so that killing weak monsters at a
// high level is not worth exactly nothing.
void player_gain_shared_experience(int32_t total);

// Experience going away, stopping at nothing. The level is NOT recounted here:
// the caller does that with player_recompute_level(), because what follows a
// changed level (hit points, spells, mana, the two redraws) is not a question
// about a number.
void player_lose_experience(int32_t amount);

// The ceiling on experience, and whether it bit. Asked once, at the top of the
// climb.
bool player_cap_experience(void);

// Whether the experience in hand has paid for a level above the one the
// character is on -- the question the climb asks over and over. False at the top
// level, whatever the experience.
bool player_deserves_next_level(void);

// One step of the climb: the level goes up by one. What that is worth is worked
// out by the caller.
void player_advance_level(void);

// Half of anything paid over the odds is thrown away, so that arriving at a
// level several steps up does not carry the whole surplus onward. Called after
// the level has gone up, and does nothing when there is no surplus.
void player_trim_surplus_experience(void);

// Notes a new high-water mark and says whether there was one.
bool player_record_max_experience(void);

// Puts the experience back to the high-water mark and says whether there was
// anything to put back. Restore-level is the only caller.
bool player_restore_experience(void);

// The level this character's experience deserves, counted from the bottom of the
// table. This is lose_exp()'s way of keeping the promise, and it can return a
// level lower OR higher than the one being carried.
uint16_t player_level_deserved_by_experience(void);

// Sets the level to the one the experience deserves and says whether it moved.
// The caller does the rest -- the hit points, the spells, the mana and the two
// redraws all sit behind that bool.
bool player_recompute_level(void);

// The five numbers set outright, one at a time, because the save file's reader
// meets them one at a time and its order cannot move -- and because the wizard
// commands break the promise between the level and the experience on purpose.
void player_set_level(uint16_t value);
void player_set_experience(int32_t value);
void player_set_max_experience(int32_t value);
void player_set_experience_fraction(uint16_t value);
void player_set_experience_factor(uint8_t value);

#endif
