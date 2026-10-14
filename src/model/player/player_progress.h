// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#ifndef PLAYER_PROGRESS_H
#define PLAYER_PROGRESS_H

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

// THE QUESTION. Three numbers answer it together: what is left, what it can reach
// and the part of a point still on its way back.
//
// The fraction is here for the reason it is in player_mana.h -- regeneration
// hands back a few hundred 65536ths of the maximum each turn, so without
// somewhere to keep the remainder nothing would ever come back. That part of
// this module is the mana's part with different names.
//
// WHAT IS NEW HERE IS THAT A NEGATIVE NUMBER IS NOT A QUANTITY. It is the mark
// of a dead character, and it is the only mark there is: nothing clamps hit
// points at zero, so a killing blow leaves them negative and they STAY negative
// all the way through the save file. Four places read the mark:
//
//   - take_hit() subtracts the damage and asks the question straight away
//     (player_damage.c),
//   - app/main.c asks it after loading, because a character can be put away dead by
//     a signal or a hangup and has to be marked dead again on the way back in,
//   - save.c asks it to decide whether to overwrite the "killed by" line with
//     "(alive and well)",
//   - save.c asks it again for the wizard resurrection, which is the one place
//     that brings the number back up to zero.
//
// So a window that tidied the number up to zero would silently break all four.
// The mark is the reason player_take_hp_damage() returns a bool and the reason
// player_hp_marks_death() exists at all: the callers should be asking a question
// with a name, not comparing a number with zero.
//
// Where this module stops is also set by the mark. Whether the wound killed the
// character is a question about the number, so it is answered here. What death
// then DOES -- the death flag, the string naming what killed them, leaving the
// level -- is the caller's, and none of it is mentioned in this file.
//
// Five rules live inside:
//
//   - regeneration (player_regenerate_hp) is the fixed-point sum, the guard for
//     a maximum so large the short wraps round, and the stop at the top where
//     the fraction is cleared ("must set frac to zero even if equal"),
//   - taking damage (player_take_hp_damage) subtracts and reports whether the
//     result is the mark of death. It does NOT clamp,
//   - healing (player_heal_hp) refuses to do anything to a character already at
//     the top, and caps at the top when it overshoots,
//   - a new maximum (player_change_max_hp) carries what is left across in
//     proportion, fraction included. UNLIKE THE MANA, A MAXIMUM OF ZERO MEANS
//     "DO NOTHING AT ALL", not "start full": mhp is zero only while the
//     character is still being made, and calc_hitpoints() deliberately does not
//     even store the new maximum then (create.c fills both in a moment later).
//     This is the first place the mana's windows cannot simply be copied,
//   - a temporary maximum going up and coming back down (the two
//     player_*_temporary_max_hp windows). Going up adds to both numbers and
//     leaves the fraction; coming down takes the maximum away first and then
//     caps what is left. Heroism (ten points) and super heroism (twenty) wrote
//     the same four lines each (app/dungeon.c).
//
// What the rules deliberately do NOT do:
//
//   - how fast regeneration runs is the caller's number, and so is the decision
//     not to regenerate at all while poisoned (app/dungeon.c),
//   - what the maximum SHOULD be comes from the level, the hit-point table and
//     the constitution (calc_hitpoints in level_ops.c), and raising PY_HP afterwards
//     belongs to the status flags, which are another question,
//   - "is it full?" is not a window. The three places that ask are not asking
//     the same thing -- twice it is `chp < mhp` (regenerate, heal) and once it
//     is `chp == mhp` (stop resting) -- and folding them would change what
//     happens if chp ever exceeded mhp,
//   - printing. prt_chp() and prt_mhp() stay with the callers, which watch for a
//     change themselves.

// What is left. Read by the status line, the character sheet, the character
// dump, the save file, the high-score record and the rest-until-recovered test.
// A NEGATIVE VALUE MEANS THE CHARACTER IS DEAD -- see player_hp_marks_death().
int16_t player_hp(void);

// What it can reach. Zero only while a character is being made.
int16_t player_max_hp(void);

// The part of a point still on its way back. Only the save file asks.
uint16_t player_hp_fraction(void);

// Whether what is left is the mark of a dead character: less than zero.
bool player_hp_marks_death(void);

// Takes a wound and says whether it was fatal. Nothing is clamped: a fatal
// wound leaves the number negative on purpose, because that is what the mark of
// death is made of. The caller decides what dying involves.
bool player_take_hp_damage(int damage);

// Heals, capping at the top, and says whether anything happened -- a character
// already at the top is not healed at all, and the caller uses that to stay
// quiet. The fraction survives a partial heal and is cleared when the top is
// reached.
bool player_heal_hp(int amount);

// One turn of coming back, at the caller's rate (a factor in 65536ths).
// Everything the fraction needs is in here.
void player_regenerate_hp(int percent);

// Sets a new maximum, carrying what is left across in proportion, and says
// whether the maximum moved. A maximum of zero (a character still being made)
// is left alone entirely -- this is where the mana's window does not fit.
bool player_change_max_hp(int16_t new_max);

// A temporary maximum arriving: both numbers go up together, so what is left
// stays as far from the top as it was. Heroism and super heroism.
void player_gain_temporary_max_hp(int16_t bonus);

// The same temporary maximum wearing off: the maximum comes down and what is
// left is capped to it. Says whether the cap bit, because the callers only
// redraw when it did.
bool player_lose_temporary_max_hp(int16_t bonus);

// A brand new maximum, filled to the top. Character creation and the wizard
// command wrote the same three lines.
void player_reset_hp(int16_t max);

// Brings a dead character back to nothing at all and says whether there was
// anything to bring back. The wizard resurrection is the only caller, and the
// only place the mark of death is ever rubbed out.
bool player_resurrect_hp(void);

// The three parts set outright, one at a time, because the save file's reader
// meets them one at a time and its order cannot move.
void player_set_hp(int16_t value);
void player_set_max_hp(int16_t value);
void player_set_hp_fraction(uint16_t value);

// MAX_PLAYER_LEVEL comes from constant.h, which has to be included before this
// header.

// This is storage and only storage, the same as progress.h and score_death.h.
// The window does not range-check the level and does not roll anything, because
// the old code did neither: the rolling (and the bounds it re-rolls against)
// belongs to character creation, which needs randint and the class tables.
//
// The table is rolled once, at character creation, and never changes again --
// gaining a level does not roll new hit points, it reads the number that was
// decided at birth. The entries accumulate: level 1 holds the hit die, and each
// later level holds the previous total plus that level's roll. So the value at
// the top level is the character's whole life's worth of hit points, which is
// what creation checks against its bounds.

// The total rolled for this level, counting from 1: level 1 is the first entry.
// The old code wrote player_hp[lev - 1] at every reader, so the subtraction
// happens here now, in one place.
//
// The level is not checked. Levels outside 1..MAX_PLAYER_LEVEL read and write
// past the table, exactly as the old array subscript did.
uint16_t hp_total_at_level(int level);
void set_hp_total_at_level(int level, uint16_t total);

// The table itself, for the save file only. The save file reads and writes all
// MAX_PLAYER_LEVEL entries in one call (rd_shorts / wr_shorts), so it needs the
// address rather than the values -- the same reason death_cause() hands back a
// pointer. Nothing else should use this.
uint16_t *hp_table_slots(void);

// THE QUESTION. Three numbers answer it together: what is left, what it can
// reach and the part of a point still on its way back. They are statics in
// player_mana.c and these windows are the only way in.
//
// THE FRACTION IS THE REASON THIS MODULE HAS RULES IN IT. Regeneration gives
// back a tiny amount each turn -- the whole maximum multiplied by a factor of
// a few hundred 65536ths (PLAYER_REGEN_NORMAL is 197), plus a floor of
// PLAYER_REGEN_MNBASE. For a character with ten mana that is well under a
// hundredth of a point a turn, so without somewhere to keep the remainder
// nothing would ever come back. The fraction is that remainder: the integer
// part goes onto what is left, the sixteen low bits are carried, and when the
// carry fills up it becomes one more point. Nobody outside wants to see it --
// only the save file does, because a character put away mid-recovery should not
// lose the progress -- so the arithmetic that touches it belongs here.
//
// Three rules live inside, and each of them is a rule about the fraction:
//
//   - regeneration (player_regenerate_mana) is the fixed-point sum above, plus
//     a guard for a maximum so large that the short wraps
//     round to negative, plus the stop at the top, WHERE THE FRACTION IS
//     CLEARED ("must set frac to zero even if equal" -- a full store has
//     nothing on its way back),
//   - spending (player_spend_mana) empties the fraction ONLY when it cannot
//     pay in full. An ordinary cast takes the cost off and LEAVES THE FRACTION
//     ALONE, so progress towards the next point survives being spent. Three
//     places spend: casting a spell (magic.c), saying a prayer (prayer.c) and a
//     monster drinking the mana (creature.c),
//   - a new maximum (player_change_max_mana) carries what is left across in
//     proportion, fraction included, because the old maximum and the new one
//     are rarely a whole multiple apart. Going up a level does this.
//
// What the rules deliberately do NOT do is decide anything outside the mana:
//
//   - how fast regeneration runs is the caller's number (hunger, resting and
//     the regeneration flag all move it -- app/dungeon.c),
//   - what happens when a cast cannot be paid for is the caller's: fainting,
//     the paralysis that lasts five turns per point missing, and the one in
//     three chance of hurting the constitution (magic.c, prayer.c),
//   - what the maximum SHOULD be is worked out from the stat, the class and
//     the level (calc_mana in spellbook.c), and raising PY_MANA afterwards belongs
//     to the status flags, which are another question,
//   - printing. prt_cmana() cannot be called from inside a store, so the
//     callers watch for a change themselves (the same arrangement
//     player_display_numbers.c has with PY_ARMOR).

// What is left. Read by the status line, the character sheet, the character
// dump, the save file, the two "can I afford this spell?" questions
// (spellbook.c and the casting itself) and the rest-until-recovered test.
int16_t player_mana(void);

// What it can reach. Zero for a character who knows no spells at all, which is
// how gain_spells() (spellbook.c) asks whether a first spell has just been learned.
int16_t player_max_mana(void);

// The part of a point still on its way back. Only the save file asks.
uint16_t player_mana_fraction(void);

// Spends what a spell costs and says how much was actually spent -- the whole
// cost when there was enough, less when there was not. Coming up short empties
// the store (and the fraction with it) rather than going negative; the caller
// decides what being short costs, and can tell from the return value.
int player_spend_mana(int cost);

// One turn of coming back, at the caller's rate (a factor in 65536ths).
// Everything the fraction needs is in here.
void player_regenerate_mana(int percent);

// Fills the store to the top and says whether that changed anything -- a
// potion of restore mana on a character who is already full does nothing and
// is not even identified by it.
bool player_restore_mana(void);

// Sets a new maximum, carrying what is left across in proportion, and says
// whether the maximum moved. A character who had no mana at all starts full.
bool player_change_max_mana(int16_t new_max);

// Leaves the character with no mana at all, maximum included, and says whether
// that changed anything. This is forgetting the last spell, not spending.
bool player_lose_all_mana(void);

// The three parts set outright, one at a time, because the save file's reader
// meets them one at a time and its order cannot move. The wizard command sets
// all three at once.
void player_set_mana(int16_t value);
void player_set_max_mana(int16_t value);
void player_set_mana_fraction(uint16_t value);

// One question, one module. The purse is read and written on its own, never
// alongside another of the player's values.

// This is storage and only storage. The windows do not clamp, do not refuse a
// negative, and do not guard against overflow, because the old code did none
// of those things here and doing them here would change behaviour rather than
// preserve it. Every rule about the purse has exactly one reader and stays
// with it:
//
//   - a shop refuses a purchase you cannot afford (store_ui.c asks first),
//   - a thief takes a tenth and cannot take more than you have
//     (monster_melee.c compares first),
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

// THE QUESTION. How far down has this character ever got? Not where it is now --
// that is the current depth (dungeon_level(), dungeon_level.h), which changes
// every time a staircase is used and goes back up as well as down. This number
// only ever grows.
//
// It is measured in dungeon levels, the same unit as the depth, and the town is
// level 0. So zero means "has never gone below the town", which is where every
// character starts.
//
// NOT ABOUT THE CHARACTER'S BODY OR SKILL AT ALL: it is a record of where the
// character has been. Three
// different files ask it for three unrelated reasons (the score, the scroll of
// word-of-recall, and the saved file), and only one place answers it.

// The deepest level ever reached. Zero means the character has never left town.
int player_max_depth(void);

// Remember that this level has been reached. KEEPS THE DEEPER OF THE TWO -- the
// comparison is in here because there is one caller, the reason can be said in
// the words of this module, and no `>` against this number is left outside.
//
// Going back up the stairs therefore cannot lower the record, which is what the
// question means.
void player_note_depth_reached(int level);

// Put a record back, REPLACING whatever is there. For the saved file only, the
// same as hp_table_slots(): a load has to restore the number that was written
// rather than merge with it.
//
// Play may not use this -- the window above is the only way a character records
// having got deeper, and it cannot go backwards. (Today the two would agree
// anyway, because a game is loaded once, at startup, into a store that is still
// zero. The two windows are still separate, because "remember I reached level 12"
// and "the file says level 12" are different sentences, and only one of them is
// allowed to be a step backwards.)
void player_max_depth_set(int level);

// WHAT THIS MODULE DOES NOT ANSWER -- four things, all of them still in the
// callers:
//
//   1. WHICH LEVEL THE CHARACTER IS ON NOW. That is the depth, which has a
//      window of its own (dungeon_level.h), and it is the input
//      to the window above rather than something this module keeps.
//   2. WHAT THE RECORD IS WORTH. total_points() in death.c pays 100 points per
//      level. Scoring reads this number, but how generous the score is has
//      nothing to do with where the character has been.
//   3. WHERE WORD-OF-RECALL DROPS YOU. app/dungeon.c reads the record and hands it
//      to leave_for_level(); the "nowhere to be yanked to" branch for a
//      character still in town is about what the scroll does, not about the
//      record.
//   4. THE POSITION OF THE SHORT IN THE SAVED FILE. save.c keeps the byte order.

#endif // PLAYER_PROGRESS_H
