// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#ifndef PLAYER_STATS_H
#define PLAYER_STATS_H

// Seven stat-to-bonus tables.
//
// Every one of them reads the player's used stat (player_stat_use()) rather
// than taking it as a parameter. Only stat_adj() takes an argument, and that
// argument is which stat to read.

// Adjustment for wisdom/intelligence -JWT-
// `stat` is one of A_STR .. A_CHR. The only adjustment with a floor of 0.
int stat_adj(int stat);

// Adjustment for charisma -RAK-
// Percent decrease or increase in price of goods
int chr_adj(void);

// Returns a character's adjustment to hit points -JWT-
int con_adj(void);

// Returns a character's adjustment to hit. -JWT-
int tohit_adj(void);

// Returns a character's adjustment to armor class -JWT-
int toac_adj(void);

// Returns a character's adjustment to disarm -RAK-
int todis_adj(void);

// Returns a character's adjustment to damage -JWT-
int todam_adj(void);

// The four values of each stat, indexed A_STR .. A_CHR: what is restored (max),
// what is natural (cur), what worn items add (mod) and what is used (use).
// The save file writes them in this order, six of each.
uint8_t player_stat_max(int stat);
uint8_t player_stat_cur(int stat);
int16_t player_stat_mod(int stat);
uint8_t player_stat_use(int stat);

// The windows store what they are given and check nothing.
void player_stat_set_max(int stat, uint8_t value);
void player_stat_set_cur(int stat, uint8_t value);
void player_stat_set_mod(int stat, int16_t value);
void player_stat_add_mod(int stat, int16_t amount);
void player_stat_set_use(int stat, uint8_t value);

// The nine values shown in the "(Miscellaneous Abilities)" block.
//
// Returned as one value rather than through output parameters: nine
// out-parameters would be unreadable, and the caller needs all of them at once.
//
// `bth`, `bthb`, `fos`, `srh`, `stl`, `dis`, `save` and `dev` are fed to
// likert() by the callers; the divisor differs per ability so it stays there.
//
// `infra` is already formatted. Both call sites print it with the very same
// "%d feet", so keeping the format here is what stops the screen and the file
// from drifting apart again.
struct player_abilities {
    int bth;  // Fighting
    int bthb; // Bows/Throw
    int fos;  // Perception
    int srh;  // Searching
    int stl;  // Stealth
    int dis;  // Disarming
    int save; // Saving Throw
    int dev;  // Magic Device
    char infra[24]; // "%d feet"; 24 covers the whole int16_t range times ten
};

// Computes the ratings from the player's windows and class_level_adj.
// Reads the player rather than taking it as a parameter, so that the two
// callers stay as they were.
struct player_abilities calc_player_abilities(void);

// SEVENTEEN yes-or-no answers about what the character can do and resist.
//
// THE ANSWER IS NOT KEPT, IT IS DERIVED. These seventeen are WORKED OUT FROM THE
// EQUIPMENT, and calc_bonuses() in player_bonuses.c does it from scratch every
// time anything changes: it clears all seventeen and sets them again from the
// flags of what is worn.
// ALL THIRTY-NINE OF player_bonuses.c'S REFERENCES ARE THAT ONE FUNCTION.
// These seventeen are still in the save file only because nothing calls
// calc_bonuses() when a game is loaded.
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

// This is storage and only storage, the same as progress.h, score_death.h,
// hp_table.h and player_light.h. Both answers are computed from the character's
// strength and the weight they carry -- check_strength() (inven_ops.c) does that
// once, when PY_STR_WGT says the weight or the strength has changed -- but they
// are *remembered* rather than derived, for the same reason as the light:
// only the transitions carry the messages and the side effects.
//
//   the weapon: going from light enough to too heavy prints "You have trouble
//   wielding such a heavy weapon." and recomputes the bonuses; coming back
//   prints "You are strong enough to wield your weapon."
//
//   the pack: the speed change is applied as a *difference*
//   (change_speed(new - remembered)), so without the remembered number the
//   character's speed cannot be corrected -- it would be reapplied from
//   scratch every time.
//
// The windows do not range-check or clamp, and they decide nothing on the
// caller's behalf, because the old assignments did neither.

// --- the weapon that is too heavy ----------------------------------------

// True while the wielded weapon weighs more than the character's strength can
// handle (use_stat[A_STR] * 15 < weight). While it is true the to-hit shown on
// the character sheet is reduced (player_bonuses.c) and digging is harder
// (terrain_commands.c).
//
// Wielding something else clears it before check_strength() looks again
// (inven_menu.c twice), and so does restoring a save file, because the flag is not
// in the save file.
bool weapon_is_too_heavy(void);
void set_weapon_too_heavy(bool too_heavy);

// --- the pack that slows the character down ------------------------------

// How many steps of speed the pack is costing right now; 0 when the pack is
// light enough to be free. It is a count, not a flag: the weight over the limit
// is divided by the limit, so a very heavy pack costs several steps.
//
// Reading it is how the callers answer two different questions: "has the
// penalty changed?" (compared against a freshly computed number, in
// check_strength and in inven_check_weight, which is what makes picking an
// object up refusable) and "how much speed do I have to give back?" (save.c
// hands change_speed the negated number before writing the file, so that the
// speed in the save file is the unencumbered one).
int pack_speed_penalty(void);
void set_pack_speed_penalty(int steps);

// THE QUESTION. How much does the character weigh? In pounds, not tenths of a
// pound -- a male Halfling is about sixty, a male Human about a hundred and
// eighty (src/model/data/player.c:107, the m_b_wt / f_b_wt columns of race[]).
//
// NOT VANITY -- A PHYSICAL QUANTITY. The number is on the character sheet, but
// four of the six readers are rules, not display: a heavier character carries
// more (inven_ops.c's weight_limit()), bashes monsters with a shield harder
// (player_melee.c's py_bash(), twice) and breaks doors down more easily
// (terrain_commands.c's bash()).
//
// WRITTEN ONCE IN A NORMAL CHARACTER'S LIFE, at creation (create.c, one line for
// each sex), plus a saved game being read back and the wizard's tweak.
//
// THERE IS NO `_adjust` WINDOW, and the reason is not the reason the race had
// one. THE RACE WAS NOT A NUMBER; THIS IS A NUMBER THAT NOBODY ADDS TO. Nothing
// in the game makes the character heavier or lighter -- not food, not the pack,
// not a curse. THE CHARACTER DOES NOT PUT ON WEIGHT. So the two windows below are
// the whole question.

// The weight in pounds. EIGHT CALLERS: the carrying limit (inven_ops.c), the shield
// bash's to-hit and damage (player_melee.c, twice, folded into one call), the door
// bash (terrain_commands.c), the character sheet (char_screen.c), the dumped
// character file (files.c), the saved file being written (save.c) and the
// wizard's prompt (wizard.c).
//
// An int, though the field is a uint16_t: every caller either divides it
// (`/ 10`, `/ 60`, `/ 2`), adds it to an int, or prints it with `(int)` already
// written in. Returning the narrow type would only move those casts around.
int player_body_weight(void);

// The weight outright. FOUR CALLERS: creation twice (one line per sex, because
// the male and female columns of race[] are different numbers), a saved file's
// short put back (save.c) and the wizard's "Weight =" prompt (wizard.c).
//
// THAT IS THE SEVENTH ANSWER to the question the fifteenth unit raised -- "does
// loading a saved game use the same window as the game itself?" -- and it is "the
// same", agreeing with the hit die, the base to-hit, the disarming skill, the
// saving throw and the race rather than with the armour class.
//
// IT REFUSES NOTHING (findings.md 24).
// The wizard's caller keeps its own `tmp_val > -1` check, which is about what
// that prompt will accept, not about what a body may weigh. WIDTH IS STILL A
// SHORT, so 65536 lands on 0 and -1 lands on 65535.
void player_body_weight_set(int pounds);

// WHAT THIS MODULE DOES NOT ANSWER -- three things, all still in the callers:
//
//   1. HOW MUCH THE CHARACTER CAN CARRY. inven_ops.c:118 computes
//      `use_stat[A_STR] * PLAYER_WEIGHT_CAP + weight`, capped at 3000. THE
//      SUBJECT OF THAT SENTENCE IS STRENGTH; the body's weight is the smaller
//      term, and there is only one such site, so nothing is piling up.
//   2. HOW HARD A BASH LANDS. py_bash() (`/ 10`, `/ 60 + 3`) and bash() (`/ 2`).
//      Three different divisors for three different rules: those are facts
//      about bashing, not about the body, and there is no shared expression to
//      fold.
//   3. HOW THE NUMBER IS SHOWN. char_screen.c:109 hands it to prt_num(), files.c:209
//      to fprintf("%6d"). Unlike the race's name, THE TWO SITES HAVE NO COMMON
//      EXPRESSION -- the receivers differ -- so there is no display window here.

// THE QUESTION. How much armour class does this character really have? The unit
// is armour class, and BIGGER MEANS HARDER TO HIT: every reader hands the number
// to test_hit() as the amount an attacker has to beat, or takes it as the
// percentage-like amount that shaves a blow down. Read from twenty-six places.
//
// TWO FIELDS, ONE QUESTION. The split is BY WHERE THE POINTS CAME FROM, not by
// what they mean:
//
//   - THE ARMOUR HALF is what is worn: the `ac` of every worn item added up,
//     plus the temporary points a spell lends (invulnerability 100, blessing 2).
//   - THE MAGICAL HALF is the plusses: the character's dexterity bonus
//     (toac_adj()) plus the `toac` of every worn item.
//
// NOT ONE READER LOOKS AT A HALF. All twenty-six read `pac + ptoac`, so the
// split is invisible to everyone who uses the answer; only the four places that
// build it, and the two shorts in the saved file, can tell the halves apart.
// The proof is in create.c, which fills the halves THE OTHER WAY ROUND when the
// race is chosen (dexterity bonus into the armour half) from when the class is
// chosen (dexterity bonus into the magical half) -- and nothing in the game can
// tell the difference, because both spellings add up to the same number.
//
// WHO ASKS: the monsters. Twenty-one of the twenty-six readers are the lines in
// monster_melee.c where one blow of one monster is tested, one for each kind
// of attack; four more are in traps.c (a trap's dart, a thrown thing); and the
// last one is monster_melee.c's `damage -= (ac * damage) / 200`, which is why a
// character in plate takes less from a blow that did land.

// The answer: both halves added up. TWENTY-SIX CALLERS, which is what this
// module is for.
//
// The sum is an int rather than an int16_t on purpose (C widens both halves
// before adding them). It is worth saying because 100 points of invulnerability
// on top of a heavy suit is the one place the halves get large.
int player_armour_class(void);

// One half at a time. THREE CALLERS FOR THE MAGICAL HALF -- the character
// sheet's starting numbers in calc_bonuses() and in create.c, and the saved
// file -- and ONE FOR THE ARMOUR HALF, the saved file.
//
// The sheet is a DIFFERENT QUESTION (player_display_numbers.h): prt_pac() shows
// player_display_ac(), not this number, and the two are deliberately allowed to
// disagree -- an unidentified suit protects the character while the sheet still
// says nothing about it. These two windows exist so that the file keeps its two
// shorts and the sheet keeps its starting value; nothing else needs a half.
int player_armour_class_magical(void);
int player_armour_class_armour(void);

// Start again from nothing worn: the armour half becomes zero and the magical
// half becomes what is handed over (always toac_adj(), the dexterity bonus).
// TWO CALLERS, calc_bonuses() and create.c's class table.
//
// ONE WINDOW RATHER THAN TWO ASSIGNMENTS, because "nothing is worn yet" is one
// sentence: every time the equipment changes, calc_bonuses() wipes both halves
// and adds the worn items back one by one.
void player_armour_class_reset(int magical);

// Both halves outright. TWO CALLERS: the saved file's two shorts put back, and
// create.c's race table, which is the one place that puts the dexterity bonus in
// the armour half instead of the magical one.
//
// SEPARATE FROM player_armour_class_reset(), and this is the second answer to
// the question the fifteenth unit raised (does the save file's restore need a
// window of its own?). For the hit die it did not -- the race's base and the
// file's byte were the same sentence. HERE IT DOES: _reset() carries the rule
// "the armour half is zero", which a restore cannot obey, because the number in
// the file already has the worn armour in it.
void player_armour_class_set_parts(int armour, int magical);

// One worn item's worth: its `ac` into the armour half, its `toac` into the
// magical half. ONE CALLER, the loop in calc_bonuses() that walks the worn
// slots.
//
// The two go together because an item gives both at once, and taking them in one
// window keeps the caller from touching the store twice per slot.
void player_armour_class_add_item(int armour, int magical);

// More armour class by this many, or less if it is negative. SIX CALLERS, all of
// them spells, and ALL OF THEM TOUCH THE ARMOUR HALF -- invulnerability is worth
// 100 and a blessing is worth 2:
//
//   - calc_bonuses() adds them whenever it rebuilds, for as long as the spell is
//     in force;
//   - app/dungeon.c adds them the turn the spell begins and takes them away the turn
//     it runs out.
//
// THE DOUBLE BOOKKEEPING IS THE CALLER'S, not this module's (the same shape as
// the twelfth unit's infravision). Whether a spell is in force, and whether this
// is the turn it began, is a question player_timed_effects.c answers; this
// window only moves the number.
//
// NOTHING IS REFUSED AND NOTHING IS CLAMPED, because nothing was before: the old
// code was a bare `+=` and a bare `-=`.
void player_armour_class_adjust(int armour);

// WHAT THIS MODULE DOES NOT ANSWER -- four things, all still in the callers:
//
//   1. WHETHER A BLOW LANDS. test_hit() in hit_rolls.c weighs the armour class
//      against the attacker; twenty-five of the twenty-six readers only pass the
//      number to it.
//   2. HOW MUCH A BLOW THAT LANDED HURTS. monster_melee.c's `* damage / 200`
//      is a rule about damage, and its one reader is monster_melee.c.
//   3. WHAT THE SHEET SAYS. player_display_numbers.c keeps that, and it is
//      allowed to differ (see above).
//   4. HOW LONG A SPELL LASTS. player_timed_effects.c keeps the clocks; this
//      module is only told to add 100 or take 2 away.

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

// THE QUESTION. How good is this character at hitting things, before the weapon,
// the level and the strength of the arm are counted? The unit is the same one
// test_hit() weighs against a target's armour class, and BIGGER MEANS MORE LIKELY
// TO HIT. The range the game itself admits to is 0 to 200 (wizard.c asks for a
// number in that span); a Human Warrior starts at 70 swinging and 55 shooting, a
// Human Mage at 34 and 20. NEGATIVE IS POSSIBLE AND REAL: an Elf's racial base is
// -5 for swinging and +15 for shooting, so an Elven Mage begins at 29.
//
// TWO NUMBERS, BOTH OF THEM ANSWERS:
//
//   - THE MELEE NUMBER is used when the character swings what is wielded.
//   - THE BOWS NUMBER is used when something is fired or thrown, and it is the
//     one the game leans on more heavily -- throw.c picks it out again for every
//     kind of launcher.
//
// THE MIRROR IMAGE OF THE ARMOUR CLASS (player_armour_class.h). There, two fields
// held one answer: every reader added them up and no reader wanted a half. Here,
// NOT ONE READER ADDS THEM UP -- a reader always knows which of the two it wants,
// because it already knows whether a blow or a shot is being tested.
//
// SO WHY ONE MODULE AND NOT TWO? Because the WRITERS never separate. All eight
// places that write do it in pairs, and six of the eight pairs hand both numbers
// the very same amount: heroism is worth 12 to each, super heroism 24 to each, a
// blessing 5 to each. A race gives both, a class adds to both, the saved file
// carries both, and the wizard screen is the only caller in the game that ever
// changes one without the other. Two questions that share every way of changing
// are one question with two answers.
//
// WHO ASKS: the four places a to-hit roll is made. player_melee.c for a swing (twice,
// because an unlit target is harder to hit), throw.c for a shot or a throw (seven
// times, once per launcher), abilities.c for the two ratings on the character
// sheet, and the saved file.

// The two answers. FIVE CALLERS FOR THE MELEE NUMBER (abilities.c's rating,
// player_melee.c's two branches, the wizard screen, the saved file) and TEN FOR THE BOWS
// NUMBER (abilities.c, throw.c's seven, the wizard screen, the saved file).
//
// Both are ints, as `p_ptr->bth` always was once C had widened it: every caller
// puts the number straight into a larger sum.
int player_base_to_hit(void);
int player_base_to_hit_with_bows(void);

// Both numbers outright. TWO CALLERS: create.c's race table, and the saved file's
// two shorts put back.
//
// ONE WINDOW FOR THE PAIR, because neither caller has ever set one alone -- the
// race hands over both bases in two adjacent lines, and the file holds them in two
// adjacent shorts (melee first, and that order cannot move).
//
// UNLIKE THE ARMOUR CLASS, the file's restore and the race's base ARE the same
// sentence here, so this one window serves both: nothing is wiped and nothing is
// implied, the two numbers are simply replaced. That is the third answer to the
// question the fifteenth unit raised, and it agrees with the hit die rather than
// with the armour class.
void player_base_to_hit_set(int melee, int with_bows);

// One number alone. ONE CALLER EACH, both of them the wizard screen, which shows
// the melee number, takes a new one, and only then moves on to the bows number.
//
// THESE TWO WINDOWS ARE NOT SHORTHAND for player_base_to_hit_set() with the other
// number read back: the old code wrote one field per prompt, so the window writes
// one number per prompt. THE 0 TO 200 RANGE IS THE WIZARD SCREEN'S OWN and stays
// there; these windows refuse nothing, because the fields refused nothing.
void player_base_to_hit_set_melee(int melee);
void player_base_to_hit_set_with_bows(int with_bows);

// The class's worth added to whatever the race left here. ONE CALLER, create.c,
// for the melee and the bow amounts alike.
//
// The two amounts differ (a Warrior's class is worth 70 to swinging and 55 to
// shooting), which is why this window takes two numbers where the next one takes
// one.
void player_base_to_hit_adjust(int melee, int with_bows);

// This many more to BOTH numbers, or fewer if it is negative. SIX CALLERS, all of
// them spells:
//
//   - heroism, 12 the turn it begins and -12 the turn it runs out;
//   - super heroism, 24 and -24;
//   - a blessing, 5 and -5.
//
// The reason this is one window: no spell in the game has ever moved one of the
// two numbers by a different amount than the other.
//
// THE DOUBLE BOOKKEEPING IS THE CALLER'S, the same shape as the armour class and
// the infravision before it: whether a spell is in force, and whether this is the
// turn it began or ended, is what player_timed_effects.c answers. This window only
// moves the numbers.
void player_base_to_hit_adjust_both(int amount);

// WHAT THIS MODULE DOES NOT ANSWER -- four things, all still in the callers:
//
//   1. WHETHER A BLOW LANDS. test_hit(int bth, ...) in hit_rolls.c does that, and ITS
//      FIRST ARGUMENT HAS THE SAME NAME AS THIS QUESTION but is only whatever the
//      caller handed over -- a monster's own to-hit goes through the same door.
//   2. WHAT THE CHARACTER SHEET SAYS. abilities.c adds the weapon bonus times
//      BTH_PLUS_ADJ and the class's per-level column on top of these numbers, and
//      likert() turns the total into words. That sum is the sheet's question.
//   3. THE BARE-HANDED AND UNLIT PENALTIES. player_melee.c halves the melee number and
//      subtracts when the target cannot be seen; throw.c takes 75 percent of the
//      bows number for a thing merely thrown. Each of those rules has exactly one
//      reader, and it stays with that reader.
//   4. HOW LONG A SPELL LASTS. player_timed_effects.c keeps the clocks.

// Four numbers: the +To Hit, +To Damage, +To AC and Total AC printed on the status
// panel, the character sheet and the character dump.
//
// These are *not* the numbers the game fights with. Those are the real plusses:
// the aim and the force are in player_attack_bonuses.c, the two halves of the
// armour class are in player_armour_class.c. This question is only about what the
// character is told. Four things make the difference real, and all four are reasons
// the copies cannot become a derivation of the real numbers:
//
//   - **They are filtered by what the character knows.** calc_bonuses() adds
//     every worn item's plusses to the real numbers, but adds them here only
//     when the item is identified (known2_p). An unidentified item still shows
//     its base AC -- unless it is cursed, in which case even that is withheld.
//   - **One penalty exists only here.** A weapon too heavy to wield properly
//     costs the *shown* to-hit (use_stat[A_STR] * 15 - weight) and nothing
//     else; the real aim never carries it (player_to_hit_bonus), because the
//     attack code takes it
//     off separately (attack_blows(), hit_rolls.c). So the shown to-hit is not a
//     filtered real number -- it is assembled in its own right.
//   - **The AC is shown twice, summed differently.** The real numbers keep the
//     armour (pac) and the bonus (ptoac) apart; the sheet prints "+ To AC"
//     *and* "Total AC", so the visible bonus has to be folded into the visible
//     total (player_display_fold_to_ac).
//   - **They are remembered, not recomputed.** All four go into the save file
//     and come back out of it, and nothing calls calc_bonuses() on the way in.
//     The sheet a restored character sees is the one that was saved. Like the
//     lit circle in player_light.c, this stays a place to put a number rather
//     than becoming a derivation.

// --- what the sheet says --------------------------------------------------

// Read by the status panel (status_line.c) and the character sheet
// (char_screen.c), the character dump (files.c) and the save file's writer
// (save.c). Nothing else asks.
int16_t player_display_to_hit(void);
int16_t player_display_to_dam(void);
int16_t player_display_to_ac(void);
int16_t player_display_ac(void);

// --- putting the sheet together again -------------------------------------

// Start over from the real plusses: the three bonuses shown are the real ones
// before any equipment is counted, and nothing of the armour is visible yet.
// This is the rule that opens calc_bonuses() (player_bonuses.c) and closes character
// creation (create.c) -- the same four lines in both places, which is why they
// are in here.
//
// The caller passes the real numbers in because the real numbers are not this
// module's business; what is this module's business is that the sheet begins
// as a copy of them.
void player_display_start_from_real(int16_t to_hit, int16_t to_dam, int16_t to_ac);

// A worn item's contribution, one number at a time, as calc_bonuses() walks the
// equipment. The amount may be negative: a known cursed item lowers what the
// sheet says, and so does a weapon too heavy to wield (that one goes through
// player_display_add_to_hit).
void player_display_add_to_hit(int amount);
void player_display_add_to_dam(int amount);
void player_display_add_to_ac(int amount);

// Changes the Total AC. Worn armour goes on here as the equipment is walked,
// and so do the temporary blessings: invulnerability is worth 100 and a
// blessing 2, added when the spell takes hold and taken off when it lapses
// (app/dungeon.c does that without recalculating anything else).
void player_display_add_ac(int amount);

// The "+ To AC" the sheet shows is part of the "Total AC" it shows. Fold it in
// once the equipment has been counted. Calling this twice would count the bonus
// twice -- the one caller calls it once, at the end of the walk.
void player_display_fold_to_ac(void);

// --- putting a number in outright -----------------------------------------

// Replaces what was there. Only the save file's reader needs these: the four
// numbers are stored one by one, in their own order, and each one goes back
// where it came from.
void player_display_set_to_hit(int16_t value);
void player_display_set_to_dam(int16_t value);
void player_display_set_to_ac(int16_t value);
void player_display_set_ac(int16_t value);

#endif // PLAYER_STATS_H
