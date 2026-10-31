// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Player condition: status flags, timed effects, food, resting, running, light,
// glowing hands, pending teleport, and position.

#ifndef PLAYER_CONDITION_H
#define PLAYER_CONDITION_H

#include "config.h"
#include "constant.h"
#include "types.h"
#include "coordinate.h"

// === Status Flags ===

// Fourteen marks that an effect is in force. Order is not the bit order.
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

// Whether the effect has begun and has been announced.
bool player_effect_in_force(player_effect effect);

// Sets the mark and returns whether it was new. False means already in force.
bool player_note_effect_started(player_effect effect);

// Clears the mark. Caller handles message and bonus removal.
void player_note_effect_ended(player_effect effect);

// Both hunger marks at once (hungry and weak).
void player_note_hunger_satisfied(void);

// Request that weapon and pack be weighed against strength again.
void player_request_strength_check(void);
bool player_strength_check_requested(void);
void player_clear_strength_check_request(void);

// Request that "Study" corner of status line be drawn again.
void player_request_study_redraw(void);
bool player_study_redraw_requested(void);
void player_clear_study_redraw_request(void);

// Requests taken and cleared in same breath by dungeon loop.
void player_request_speed_redraw(void);
bool player_take_speed_redraw_request(void);

void player_request_armor_redraw(void);
bool player_take_armor_redraw_request(void);

void player_request_hp_redraw(void);
bool player_take_hp_redraw_request(void);

void player_request_mana_redraw(void);
bool player_take_mana_redraw_request(void);

// Six stat requests. Stat is 0 to 5 (A_STR .. A_CHR).
void player_request_stat_redraw(int stat);
bool player_any_stat_redraw_requested(void);
bool player_stat_redraw_requested(int stat);
void player_clear_stat_redraw_requests(void);

// Search mode: slower, more thorough, hungrier. Most-read of the thirty bits.
bool player_is_searching(void);
void player_start_searching(void);
void player_stop_searching(void);

// Rest mode. How much longer is separate (player_rest_turns).
bool player_is_resting(void);
void player_start_resting(void);
void player_stop_resting(void);

// What status line shows (not redrawn every turn).
bool player_status_line_shows_paralysis(void);
void player_set_status_line_shows_paralysis(bool shown);

bool player_status_line_shows_repeat(void);
void player_set_status_line_shows_repeat(bool shown);

// Whole word (save file format).
uint32_t player_status_word(void);
void player_set_status_word(uint32_t word);

// === Timed Effects ===

// Eighteen temporary states in save file order.
typedef enum {
    PLAYER_TIMED_BLINDNESS,
    PLAYER_TIMED_PARALYSIS,
    PLAYER_TIMED_CONFUSION,
    PLAYER_TIMED_HASTE,
    PLAYER_TIMED_SLOWNESS,
    PLAYER_TIMED_FEAR,
    PLAYER_TIMED_POISON,
    PLAYER_TIMED_HALLUCINATION,
    PLAYER_TIMED_PROTECTION_FROM_EVIL,
    PLAYER_TIMED_INVULNERABILITY,
    PLAYER_TIMED_HEROISM,
    PLAYER_TIMED_SUPER_HEROISM,
    PLAYER_TIMED_BLESSING,
    PLAYER_TIMED_HEAT_RESISTANCE,
    PLAYER_TIMED_COLD_RESISTANCE,
    PLAYER_TIMED_SEEING_INVISIBLE,
    PLAYER_TIMED_WORD_OF_RECALL,
    PLAYER_TIMED_INFRA_VISION,
    PLAYER_TIMED_COUNT
} player_timed_effect;

// Whether more than zero turns left.
bool player_timed_in_force(player_timed_effect effect);

// Number of turns left.
int player_timed_turns(player_timed_effect effect);

// Add turns to whatever is left.
void player_timed_add(player_timed_effect effect, int turns);

// Set exactly how long, replacing what was left.
void player_timed_set(player_timed_effect effect, int turns);

// No turns left, mark untouched.
void player_timed_clear(player_timed_effect effect);

// Shorten without lengthening.
void player_timed_shorten_to(player_timed_effect effect, int turns);

// State in force and not yet announced. False for effects with no mark.
bool player_timed_beginning(player_timed_effect effect);

// One turn passes. Returns true if state ran out this turn.
bool player_timed_count_down(player_timed_effect effect);

// === Food ===

// How full the stomach is. Compared against thresholds each turn.
int16_t player_food(void);

// Food in (ration or potion nourishment).
void player_gain_food(int amount);

// Food out for reason other than time passing.
void player_burn_food(int amount);

// Set stomach outright, replacing what was there.
void player_set_food(int16_t amount);

// One turn passes: rate comes off counter.
void player_digest(void);

// How much one turn costs.
int16_t player_digestion(void);

// Set the rate outright.
void player_set_digestion(int16_t amount);

// Change rate by a step (rings, searching, resting).
void player_adjust_digestion(int delta);

// === Resting ===

// Resting at all (count not zero).
bool player_resting(void);

// Count itself with sign (positive: turns left, negative: until healed).
int player_rest_turns(void);

// Whether "rest until healed" (count negative).
bool player_rest_is_until_healed(void);

// How long to rest. Positive is turns, negative is "until healed".
void player_rest_set(int turns);

// Not resting any more.
void player_rest_stop(void);

// One turn of resting passes. Returns true if rest ran out this turn.
bool player_rest_count_down(void);

// === Running ===

// Start running (first step can be taken).
void begin_run(void);

// Is player running?
bool player_is_running(void);

// Stop running.
void stop_running(void);

// Nobody running (level has begun).
void forget_run(void);

// One more step. Returns false if run has gone long enough.
bool keep_running(void);

// How many steps (for game state snapshot).
int running_steps(void);

// === Light ===

// Whether a light is burning.
bool player_has_light(void);
void set_player_has_light(bool lit);

// Whether glow is laid down on map (ring of cave[][].tl bits).
bool player_light_is_drawn(void);
void set_player_light_drawn(bool drawn);

// === Glowing Hands ===

// Charge waiting (zero means not glowing). Int because save file holds byte.
int player_glowing_hands(void);

// Hands light up (scroll 11).
void player_glowing_hands_begin(void);

// Blow connected, charge gone.
void player_glowing_hands_spend(void);

// Restore byte from save file.
void player_glowing_hands_restore(int charge);

// === Pending Teleport ===

// Teleport when turn's commands are over.
void schedule_teleport(void);

// Is one waiting?
bool teleport_is_pending(void);

// It happened.
void teleport_done(void);

// Nothing waiting (level has begun).
void forget_pending_teleport(void);

// === Position ===

// Player's row and column in cave[][].
int16_t player_row(void);
int16_t player_col(void);

// The same two, as one square.
coordinate player_position(void);

// Put player somewhere (both coordinates always set together).
void player_place(int row, int col);

// Forget where player was (generating new level).
void player_pos_forget(void);

#endif // PLAYER_CONDITION_H
