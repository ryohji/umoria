// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// What the character's state is right now: one word of thirty bits, asked and
// answered a hundred and five times

#ifndef PLAYER_STATUS_FLAGS_H
#define PLAYER_STATUS_FLAGS_H

// One question, one module -- the seventh question to leave `py`, after the
// purse (player_gold.c), the stomach (player_food.c), the four numbers the
// character sheet shows (player_display_numbers.c), the mana (player_mana.c),
// the hit points (player_hp.c) and how far the character has come
// (player_level.c). ONE field answers it -- it was py.flags.status -- but that
// field carries THIRTY-ONE NAMES, touched from a hundred and five places in ten
// files (dungeon.c sixty of them).
//
// THE QUESTION IS A SET, NOT A NUMBER: "which of these thirty things is true". The bits are declared in constant.h as PY_HUNGRY
// through PY_MANA, and NONE OF THEM APPEARS IN A CALLER ANY MORE: the mask
// arithmetic lives here, the way spells_known.c hides `1L << spell`.
//
// THE BIT LAYOUT IS THE SAVE FILE'S FORMAT. save.c writes and reads the whole
// word with wr_long/rd_long, so the numbering cannot move. That is what
// player_status_word() and player_set_status_word() are for, and THEY HAVE ONLY
// TWO CALLERS, both halves of the save file.
//
// The thirty bits fall into four families, and telling them apart is most of
// what this module is for. Reading the field alone does not reveal them: the
// same `status & PY_X` stands for four different kinds of fact.
//
//   1. A MARK THAT AN EFFECT IS IN FORCE (fourteen bits). Twelve of them pair
//      with a counter in py.flags -- hero, shero, blind, confused, afraid,
//      poisoned, fast, slow, invulnerability, blessing, see-invisible and
//      infra-vision -- and two pair with the stomach (hungry, weak). The mark
//      says "this has begun and has been announced", which is why the caller's
//      shape is always
//
//          if ((PY_X & status) == 0) { status |= PY_X; ...one-time effects... }
//
//      and why player_note_effect_started() RETURNS WHETHER THE MARK WAS NEW.
//      THIS MODULE DOES NOT FOLD THAT SHAPE UP. dungeon.c writes it out twelve
//      times, and the twelve bodies differ (messages, hit points, to-hit,
//      redraws); what they share is a skeleton that counts down, and the counter
//      is a different question (the ninth one, the timed states, some two
//      hundred and ninety references). Folding the skeleton needs both halves,
//      so it belongs to whichever module ends up owning the counters. The same
//      line as prt_experience()'s climb in player_level.c: a window can answer a
//      question about the state, not decide what a turn does.
//
//   2. A REQUEST THAT SOMETHING BE WORKED OUT OR DRAWN AGAIN (twelve bits).
//      Whoever changes a number sets the bit and walks away; one block in
//      dungeon.c picks the requests up on the next turn. Six of the twelve are
//      the six stat bits, which the code shifts (`PY_STR << stat`) and masks
//      (PY_STATS) -- constant.h says "these 6 stat flags must be adjacent" and
//      three places depended on it. That dependence is now inside this module.
//      NOTE THAT THE REQUESTS ARE NOT UNIFORM: the speed, the armour, the hit
//      points, the mana and the stats are taken and cleared in the same breath
//      (player_take_*_request), but the strength check and the study line are
//      read in dungeon.c and cleared by the function that does the work
//      (check_strength, prt_study), so those keep a separate clearing window.
//
//   3. WHAT THE CHARACTER IS DOING (two bits): searching and resting. No clock,
//      no recomputation -- a mode that movement, regeneration, the speed line
//      and the state line all ask about. PY_SEARCH is the most-read bit of the
//      thirty (nine reads).
//
//   4. WHAT THE STATUS LINE IS CURRENTLY SHOWING (two bits): paralysis and
//      repeat. These are neither state nor request but a record of the screen,
//      kept so that the line is not redrawn every turn. prt_state() clears the
//      repeat mark and sets it again if it drew "Repeat"; dungeon.c reads it to
//      decide whether the line needs drawing before a command.
//
// What this module deliberately does NOT do:
//
//   - the countdown, and the one-time effects that hang off it (family 1
//     above) -- that is the ninth question plus the callers' own words,
//   - the drawing. prt_hunger(), prt_blind(), prt_confused(), prt_afraid(),
//     prt_poisoned(), prt_state() and prt_speed() read the marks and put
//     characters on the screen; only the reading comes here,
//   - deciding what a request means. player_take_hp_redraw_request() says a
//     request was waiting; prt_mhp() and prt_chp() are the caller's business.

// No includes: the callers bring config.h, constant.h and types.h in first, the
// same as player_level.h and the five before it. types.h has no guard of its own.

// The fourteen marks that an effect is in force. The order here is not the bit
// order and nothing depends on it -- the mapping to PY_* lives in the .c file,
// which is the point.
typedef enum {
    PLAYER_EFFECT_HERO,
    PLAYER_EFFECT_SUPER_HERO,
    PLAYER_EFFECT_BLIND,
    PLAYER_EFFECT_CONFUSED,
    PLAYER_EFFECT_AFRAID,
    PLAYER_EFFECT_POISONED,
    PLAYER_EFFECT_HASTED,
    PLAYER_EFFECT_SLOWED,
    PLAYER_EFFECT_INVULNERABLE,
    PLAYER_EFFECT_BLESSED,
    PLAYER_EFFECT_SEE_INVISIBLE,
    PLAYER_EFFECT_INFRA_VISION,
    PLAYER_EFFECT_HUNGRY,
    PLAYER_EFFECT_WEAK,
    PLAYER_EFFECT_COUNT
} player_effect;

// Whether the mark is set: the effect has begun and has been announced.
bool player_effect_in_force(player_effect effect);

// Sets the mark and says WHETHER IT WAS NEW. False means the effect was already
// in force, so whatever the caller does once at the start has been done.
bool player_note_effect_started(player_effect effect);

// Clears the mark. The caller's own words follow -- the message, the bonus going
// away, the redraw.
void player_note_effect_ended(player_effect effect);

// Both hunger marks at once, which is how eat.c ends them: a full stomach
// clears "weak" and "hungry" together. THE ONLY PLACE TWO MARKS MOVE IN ONE STEP
// outside the six stat requests.
void player_note_hunger_satisfied(void);

// A request that the weapon and the pack be weighed against the character's
// strength again. Seven callers set it -- putting something on, taking
// something off, a strength change either way, a bag of tricks. dungeon.c reads
// it; check_strength() clears it once the work is done, which is why the reading
// and the clearing are two windows here instead of one.
void player_request_strength_check(void);
bool player_strength_check_requested(void);
void player_clear_strength_check_request(void);

// A request that the "Study" corner of the status line be drawn again. Set when
// the number of spells that can be learnt changes; prt_study() clears it, so
// again the reading and the clearing are separate.
void player_request_study_redraw(void);
bool player_study_redraw_requested(void);
void player_clear_study_redraw_request(void);

// Requests that are taken and cleared in the same breath, all of them by the one
// block in dungeon.c. player_take_*_request() says a request was waiting and
// leaves none behind.
void player_request_speed_redraw(void);
bool player_take_speed_redraw_request(void);

void player_request_armor_redraw(void);
bool player_take_armor_redraw_request(void);

void player_request_hp_redraw(void);
bool player_take_hp_redraw_request(void);

void player_request_mana_redraw(void);
bool player_take_mana_redraw_request(void);

// The six stat requests. The caller names a stat by its index (0 to 5, the order
// of py.stats), and the shift that turns an index into a bit stays in here --
// constant.h's "these 6 stat flags must be adjacent" is now a fact about one
// file instead of three.
void player_request_stat_redraw(int stat);
bool player_any_stat_redraw_requested(void);
bool player_stat_redraw_requested(int stat);
void player_clear_stat_redraw_requests(void);

// Search mode: slower, more thorough, hungrier. The most-read of the thirty
// bits -- movement, regeneration, the speed line and the state line all ask.
bool player_is_searching(void);
void player_start_searching(void);
void player_stop_searching(void);

// Rest mode. The bit says the character is resting; how much longer is behind
// player_resting.h, which is a different question. The two halves keep their
// own callers.
bool player_is_resting(void);
void player_start_resting(void);
void player_stop_resting(void);

// What the status line is showing, kept so that it is not drawn every turn.
bool player_status_line_shows_paralysis(void);
void player_set_status_line_shows_paralysis(bool shown);

bool player_status_line_shows_repeat(void);
void player_set_status_line_shows_repeat(bool shown);

// The whole word. THE BIT NUMBERING IS THE SAVE FILE'S FORMAT, so these two are
// what the save file's writer and reader use, and one redraw takes a copy that
// cannot change underneath it.
uint32_t player_status_word(void);
void player_set_status_word(uint32_t word);

#endif
