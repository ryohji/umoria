// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#ifndef PLAYER_STATS_H
#define PLAYER_STATS_H

#include "config.h"
#include "constant.h"
#include "types.h"

// Seven stat-to-bonus tables. All read player_stat_use() internally.

int stat_adj(int stat);    // Wisdom/intelligence adjustment; `stat` is A_STR..A_CHR
int chr_adj(void);         // Charisma price adjustment (percent)
int con_adj(void);         // Constitution HP adjustment
int tohit_adj(void);       // Dexterity+strength to-hit adjustment
int toac_adj(void);        // Dexterity AC adjustment
int todis_adj(void);       // Dexterity disarm adjustment
int todam_adj(void);       // Strength damage adjustment

// Four values per stat (A_STR..A_CHR): max (restored), cur (natural), mod (equipment), use (actual).
uint8_t player_stat_max(int stat);
uint8_t player_stat_cur(int stat);
int16_t player_stat_mod(int stat);
uint8_t player_stat_use(int stat);

void player_stat_set_max(int stat, uint8_t value);
void player_stat_set_cur(int stat, uint8_t value);
void player_stat_set_mod(int stat, int16_t value);
void player_stat_add_mod(int stat, int16_t amount);
void player_stat_set_use(int stat, uint8_t value);

// Nine ability ratings shown in "(Miscellaneous Abilities)".
// Most are fed to likert(); infra is pre-formatted as "%d feet".
struct player_abilities {
    int bth;  // Fighting
    int bthb; // Bows/Throw
    int fos;  // Perception
    int srh;  // Searching
    int stl;  // Stealth
    int dis;  // Disarming
    int save; // Saving Throw
    int dev;  // Magic Device
    char infra[24];
};

struct player_abilities calc_player_abilities(void);

// Seventeen abilities derived from equipment. calc_bonuses() clears and recomputes all.
// Stored in save file but not kept between recalculations.

bool player_can_see_invisible(void);      // From equipment or timed potion
bool player_never_paralyzed(void);        // Immunity to paralysis
bool player_resists_fire(void);
bool player_resists_cold(void);
bool player_resists_acid(void);
bool player_resists_light(void);
bool player_takes_no_falling_damage(void);
bool player_regenerates(void);
bool player_has_slow_digestion(void);
bool player_teleports_randomly(void);
bool player_aggravates_monsters(void);
bool player_stat_sustained(int stat);     // Stat (A_STR..A_CHR) cannot be drained

// Recompute abilities from equipment.
void player_abilities_forget_all(void);
void player_abilities_note_item_flags(uint32_t item_flags);  // Eleven abilities from TR_* flags
void player_abilities_note_sustain(int item_p1);             // Sustain from item's p1 (1..6)
void player_grant_see_invisible(void);                       // Grant from non-equipment source

// Save file: seventeen bytes in fixed order.
#define PLAYER_ABILITIES_SAVED_BYTES 17

uint8_t player_abilities_saved_byte(int position);
void player_abilities_restore_byte(int position, uint8_t value);

// Weight penalties remembered for transition messages and delta calculations.

// Weapon too heavy for strength (use_stat[A_STR] * 15 < weight).
// Reduces sheet to-hit and digging. Not in save file.
bool weapon_is_too_heavy(void);
void set_weapon_too_heavy(bool too_heavy);

// Pack speed penalty in steps. 0 when pack is light enough.
int pack_speed_penalty(void);
void set_pack_speed_penalty(int steps);

// Body weight in pounds. Used for carry limit, bash strength, and display.
// Set at creation and never changes.
int player_body_weight(void);
void player_body_weight_set(int pounds);

// Armour class: armour half (worn AC + spells) plus magical half (dexterity + item bonuses).
// Higher is harder to hit.
int player_armour_class(void);
int player_armour_class_magical(void);
int player_armour_class_armour(void);

void player_armour_class_reset(int magical);                // Reset to unworn state (armour=0, magical=given)
void player_armour_class_set_parts(int armour, int magical);
void player_armour_class_add_item(int armour, int magical);
void player_armour_class_adjust(int armour);                // Adjust armour half (for spells)

// To-hit and to-damage bonuses from stats and equipment. Negative is normal.
// Aim is used for hits; force is added to damage (bows don't use force).
int player_to_hit_bonus(void);
int player_to_damage_bonus(void);

void player_attack_bonuses_set(int to_hit, int to_damage);
void player_to_hit_bonus_adjust(int amount);
void player_to_damage_bonus_adjust(int amount);

// Base to-hit before weapon/level/bonuses. Melee for swinging, bows for shooting/throwing.
// Higher means more likely to hit. Negative is possible (e.g., Elf melee -5, bows +15).
int player_base_to_hit(void);
int player_base_to_hit_with_bows(void);

void player_base_to_hit_set(int melee, int with_bows);
void player_base_to_hit_set_melee(int melee);
void player_base_to_hit_set_with_bows(int with_bows);
void player_base_to_hit_adjust(int melee, int with_bows);  // Class adjustment (different values)
void player_base_to_hit_adjust_both(int amount);           // Spell adjustment (same value)

// Display numbers shown on sheet/panel. Filtered by identification, includes heavy weapon penalty.
// Not the numbers used in combat (see player_attack_bonuses.h, player_armour_class.h).
int16_t player_display_to_hit(void);
int16_t player_display_to_dam(void);
int16_t player_display_to_ac(void);
int16_t player_display_ac(void);

// Rebuild display numbers from real bonuses and equipment.
void player_display_start_from_real(int16_t to_hit, int16_t to_dam, int16_t to_ac);
void player_display_add_to_hit(int amount);
void player_display_add_to_dam(int amount);
void player_display_add_to_ac(int amount);
void player_display_add_ac(int amount);
void player_display_fold_to_ac(void);  // Fold bonus into total; call once after equipment walk

// Restore from save file.
void player_display_set_to_hit(int16_t value);
void player_display_set_to_dam(int16_t value);
void player_display_set_to_ac(int16_t value);
void player_display_set_ac(int16_t value);

#endif // PLAYER_STATS_H
