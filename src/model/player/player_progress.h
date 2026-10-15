// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#ifndef PLAYER_PROGRESS_H
#define PLAYER_PROGRESS_H

#include "config.h"
#include "constant.h"
#include "types.h"

// Level and experience: level is fixed by experience via advance_price(level) =
// player_exp[level - 1] * expfact / 100. Two ways to maintain the promise: climbing
// (player_deserves_next_level) and recounting (player_recompute_level).
// Experience fraction is division remainder (by level), not regeneration share.

// Current level.
uint16_t player_level(void);

// Current experience.
int32_t player_experience(void);

// Maximum experience ever held (for restore-level).
int32_t player_max_experience(void);

// Experience fraction (division remainder in 65536ths).
uint16_t player_experience_fraction(void);

// Experience factor (percentage, never below 100).
uint8_t player_experience_factor(void);

// Cost to leave given level: player_exp[level - 1] * expfact / 100.
int32_t player_experience_to_advance_from(uint16_t level);

// Cost to advance from current level.
int32_t player_experience_needed_to_advance(void);

// Gain experience (whole amount). Not capped here.
void player_gain_experience(int32_t amount);

// Gain shared experience (total / level, with fixed-point remainder).
void player_gain_shared_experience(int32_t total);

// Lose experience, stopping at zero. Does not recount level.
void player_lose_experience(int32_t amount);

// Cap experience at MAX_EXP. Returns whether it was capped.
bool player_cap_experience(void);

// Whether experience has paid for next level.
bool player_deserves_next_level(void);

// Advance to next level.
void player_advance_level(void);

// Discard half of experience surplus after level gain.
void player_trim_surplus_experience(void);

// Record new max experience if higher. Returns whether updated.
bool player_record_max_experience(void);

// Restore experience to max. Returns whether changed.
bool player_restore_experience(void);

// Level deserved by current experience, counted from bottom.
uint16_t player_level_deserved_by_experience(void);

// Recompute level from experience. Returns whether changed.
bool player_recompute_level(void);

// Set experience values directly (for save file and wizard commands).
void player_set_level(uint16_t value);
void player_set_experience(int32_t value);
void player_set_max_experience(int32_t value);
void player_set_experience_fraction(uint16_t value);
void player_set_experience_factor(uint8_t value);

// Hit die faces (6..21): race base + class adjustment. Set at creation, never changed.
int player_hit_die(void);

// Set hit die faces.
void player_hit_die_set(int faces);

// Adjust hit die by class bonus.
void player_hit_die_adjust(int faces);

// Hit points: current, maximum, and regeneration fraction (65536ths).
// Negative HP marks a dead character; not clamped at zero.

// Current HP. Negative means dead.
int16_t player_hp(void);

// Maximum HP.
int16_t player_max_hp(void);

// HP regeneration fraction.
uint16_t player_hp_fraction(void);

// Whether HP is negative (dead).
bool player_hp_marks_death(void);

// Take damage. Returns whether fatal (HP went negative).
bool player_take_hp_damage(int damage);

// Heal, capping at max. Returns whether healed.
bool player_heal_hp(int amount);

// Regenerate at given rate (65536ths).
void player_regenerate_hp(int percent);

// Change max HP, carrying current proportionally. Returns whether changed.
bool player_change_max_hp(int16_t new_max);

// Gain temporary max HP (heroism).
void player_gain_temporary_max_hp(int16_t bonus);

// Lose temporary max HP. Returns whether capped.
bool player_lose_temporary_max_hp(int16_t bonus);

// Reset to new max, filled.
void player_reset_hp(int16_t max);

// Resurrect dead character to zero HP. Returns whether was dead.
bool player_resurrect_hp(void);

// Set HP values directly (for save file).
void player_set_hp(int16_t value);
void player_set_max_hp(int16_t value);
void player_set_hp_fraction(uint16_t value);

// HP table: cumulative totals rolled at creation. Level not bounds-checked.
uint16_t hp_total_at_level(int level);
void set_hp_total_at_level(int level, uint16_t total);

// HP table address for save file bulk read/write.
uint16_t *hp_table_slots(void);

// Mana: current, maximum, and regeneration fraction (65536ths).
// Spending empties fraction only when insufficient; regeneration uses fraction.

// Current mana.
int16_t player_mana(void);

// Maximum mana. Zero means no spells known.
int16_t player_max_mana(void);

// Mana regeneration fraction.
uint16_t player_mana_fraction(void);

// Spend mana. Returns amount actually spent (may be less than cost).
int player_spend_mana(int cost);

// Regenerate at given rate (65536ths).
void player_regenerate_mana(int percent);

// Restore mana to full. Returns whether changed.
bool player_restore_mana(void);

// Change max mana, carrying current proportionally. Returns whether changed.
// Starting from zero fills to new max.
bool player_change_max_mana(int16_t new_max);

// Forget all spells (zero mana and max). Returns whether changed.
bool player_lose_all_mana(void);

// Set mana values directly (for save file and wizard).
void player_set_mana(int16_t value);
void player_set_max_mana(int16_t value);
void player_set_mana_fraction(uint16_t value);

// Gold: storage only. Not clamped, overflow not guarded.

// Current gold.
int32_t player_gold(void);

// Gain gold.
void player_gain_gold(int32_t amount);

// Pay gold (caller ensures sufficient).
void player_pay_gold(int32_t amount);

// Set gold directly.
void player_set_gold(int32_t amount);

// Max depth: deepest dungeon level reached (0 = never left town). Never decreases.

// Deepest level reached.
int player_max_depth(void);

// Note depth reached (keeps deeper of current and new).
void player_note_depth_reached(int level);

// Set max depth directly (for save file).
void player_max_depth_set(int level);

#endif // PLAYER_PROGRESS_H
