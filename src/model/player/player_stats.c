// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#include <stddef.h>
#include "config.h"
#include "constant.h"
#include "types.h"
#include "player.h"
#include "player_stats.h"
#include "player_identity.h"
#include "player_skills.h"
#include "player_progress.h"

static uint8_t max_stat[6];
static uint8_t cur_stat[6];
static int16_t mod_stat[6];
static uint8_t use_stat[6];

// Stat-to-bonus lookup table: bonus applies from min_stat upward until next entry.
typedef struct {
    uint8_t min_stat;
    int bonus;
} stat_bonus_step;

static int lookup_stat_bonus(uint8_t stat, const stat_bonus_step *table, size_t count) {
    int bonus = table[0].bonus;
    for (size_t i = 1; i < count && stat >= table[i].min_stat; i++) {
        bonus = table[i].bonus;
    }
    return bonus;
}

#define STAT_BONUS_STEPS(table) ((sizeof(table)) / (sizeof((table)[0])))

int stat_adj(int stat) {
    // Floor is 0, not negative.
    static const stat_bonus_step by_stat[] = {
        {0, 0}, {8, 1}, {15, 2}, {18, 3}, {68, 4}, {88, 5}, {108, 6}, {118, 7},
    };
    return lookup_stat_bonus(use_stat[stat], by_stat, STAT_BONUS_STEPS(by_stat));
}

int chr_adj(void) {
    static const stat_bonus_step by_charisma[] = {
        {0, 100},  {3, 130},  {4, 125},  {5, 122},  {6, 120},  {7, 118},
        {8, 116},  {9, 114},  {10, 112}, {11, 110}, {12, 108}, {13, 106},
        {14, 104}, {15, 103}, {16, 102}, {17, 101}, {18, 100}, {19, 98},
        {68, 96},  {88, 94},  {108, 92}, {118, 90},
    };
    return lookup_stat_bonus(use_stat[A_CHR], by_charisma, STAT_BONUS_STEPS(by_charisma));
}

// Returns a character's adjustment to hit points -JWT-
int con_adj(void) {
    int con = use_stat[A_CON];

    if (con < 7) {
        return (con - 7);
    } else if (con < 17) {
        return 0;
    } else if (con == 17) {
        return 1;
    } else if (con < 94) {
        return 2;
    } else if (con < 117) {
        return 3;
    } else {
        return 4;
    }
}

int tohit_adj(void) {
    // Dexterity and strength summed.
    static const stat_bonus_step by_dexterity[] = {
        {0, -3}, {4, -2}, {6, -1}, {8, 0}, {16, 1}, {17, 2}, {18, 3}, {69, 4}, {118, 5},
    };
    static const stat_bonus_step by_strength[] = {
        {0, -3}, {4, -2}, {5, -1}, {7, 0}, {18, 1}, {94, 2}, {109, 3}, {117, 4},
    };
    return lookup_stat_bonus(use_stat[A_DEX], by_dexterity, STAT_BONUS_STEPS(by_dexterity)) +
           lookup_stat_bonus(use_stat[A_STR], by_strength, STAT_BONUS_STEPS(by_strength));
}

// Returns a character's adjustment to armor class -JWT-
int toac_adj(void) {
    static const stat_bonus_step by_dexterity[] = {
        {0, -4}, {4, -3}, {5, -2}, {6, -1}, {7, 0}, {15, 1}, {18, 2}, {59, 3}, {94, 4}, {117, 5},
    };

    return lookup_stat_bonus(use_stat[A_DEX], by_dexterity, STAT_BONUS_STEPS(by_dexterity));
}

int todis_adj(void) {
    // Gaps in steps: 2→4, 6→8.
    static const stat_bonus_step by_dexterity[] = {
        {0, -8}, {4, -6}, {5, -4}, {6, -2}, {7, -1}, {8, 0},
        {13, 1}, {16, 2}, {18, 4}, {59, 5}, {94, 6}, {117, 8},
    };
    return lookup_stat_bonus(use_stat[A_DEX], by_dexterity, STAT_BONUS_STEPS(by_dexterity));
}

// Returns a character's adjustment to damage -JWT-
int todam_adj(void) {
    static const stat_bonus_step by_strength[] = {
        {0, -2}, {4, -1}, {5, 0}, {16, 1}, {17, 2}, {18, 3}, {94, 4}, {109, 5}, {117, 6},
    };

    return lookup_stat_bonus(use_stat[A_STR], by_strength, STAT_BONUS_STEPS(by_strength));
}

// Read player stat fields.
uint8_t player_stat_max(int stat) {
    return max_stat[stat];
}

uint8_t player_stat_cur(int stat) {
    return cur_stat[stat];
}

int16_t player_stat_mod(int stat) {
    return mod_stat[stat];
}

uint8_t player_stat_use(int stat) {
    return use_stat[stat];
}

// Write player stat fields.
void player_stat_set_max(int stat, uint8_t value) {
    max_stat[stat] = value;
}

void player_stat_set_cur(int stat, uint8_t value) {
    cur_stat[stat] = value;
}

void player_stat_set_mod(int stat, int16_t value) {
    mod_stat[stat] = value;
}

void player_stat_add_mod(int stat, int16_t amount) {
    mod_stat[stat] += amount;
}

void player_stat_set_use(int stat, uint8_t value) {
    use_stat[stat] = value;
}

struct player_abilities calc_player_abilities(void) {
    struct player_abilities a;

    // BTH_PLUS_ADJ multiplier (3) is for display; melee/throwing use bonus as-is.
    const int to_hit_bonus = player_to_hit_bonus();
    a.bth = player_base_to_hit() + to_hit_bonus * BTH_PLUS_ADJ + (class_level_adj[player_class()][CLA_BTH] * player_level());
    a.bthb = player_base_to_hit_with_bows() + to_hit_bonus * BTH_PLUS_ADJ + (class_level_adj[player_class()][CLA_BTHB] * player_level());

    a.fos = 40 - player_search_frequency();
    if (a.fos < 0) {
        a.fos = 0;
    }

    a.srh = player_search_chance();
    a.stl = player_stealth() + 1;  // +1 for likert() display

    a.dis = player_disarm() + 2 * todis_adj() + stat_adj(A_INT) + (class_level_adj[player_class()][CLA_DISARM] * player_level() / 3);
    a.save = player_saving_throw() + stat_adj(A_WIS) + (class_level_adj[player_class()][CLA_SAVE] * player_level() / 3);
    a.dev = player_saving_throw() + stat_adj(A_INT) + (class_level_adj[player_class()][CLA_DEVICE] * player_level() / 3);  // Uses save, not disarm

    (void)sprintf(a.infra, "%d feet", player_infra_range() * 10);

    return a;
}

// Seventeen abilities in save file order (order is file format, cannot change).
typedef enum {
    ABILITY_SEE_INVISIBLE,
    ABILITY_TELEPORT_AT_RANDOM,
    ABILITY_NEVER_PARALYZED,
    ABILITY_SLOW_DIGESTION,
    ABILITY_AGGRAVATE,
    ABILITY_RESIST_FIRE,
    ABILITY_RESIST_COLD,
    ABILITY_RESIST_ACID,
    ABILITY_REGENERATE,
    ABILITY_RESIST_LIGHT,
    ABILITY_NO_FALLING_DAMAGE,
    ABILITY_SUSTAIN_STR,
    ABILITY_SUSTAIN_INT,
    ABILITY_SUSTAIN_WIS,
    ABILITY_SUSTAIN_CON,
    ABILITY_SUSTAIN_DEX,
    ABILITY_SUSTAIN_CHR,
    ABILITY_COUNT
} player_ability;

_Static_assert(ABILITY_COUNT == PLAYER_ABILITIES_SAVED_BYTES, "the seventeen are seventeen bytes");

// All zero at start; any non-zero value counts as having the ability.
static uint8_t ability_held[ABILITY_COUNT];

static bool has(player_ability ability) {
    return ability_held[ability] != 0;
}

static void grant(player_ability ability) {
    ability_held[ability] = true;
}

// Eleven abilities granted by item flags. Order doesn't matter.
static const struct {
    uint32_t item_flag;
    player_ability ability;
} granted_by_item[] = {
    {TR_SLOW_DIGEST, ABILITY_SLOW_DIGESTION},
    {TR_AGGRAVATE, ABILITY_AGGRAVATE},
    {TR_TELEPORT, ABILITY_TELEPORT_AT_RANDOM},
    {TR_REGEN, ABILITY_REGENERATE},
    {TR_RES_FIRE, ABILITY_RESIST_FIRE},
    {TR_RES_ACID, ABILITY_RESIST_ACID},
    {TR_RES_COLD, ABILITY_RESIST_COLD},
    {TR_FREE_ACT, ABILITY_NEVER_PARALYZED},
    {TR_SEE_INVIS, ABILITY_SEE_INVISIBLE},
    {TR_RES_LIGHT, ABILITY_RESIST_LIGHT},
    {TR_FFALL, ABILITY_NO_FALLING_DAMAGE},
};

#define GRANTED_BY_ITEM_COUNT ((int)(sizeof granted_by_item / sizeof granted_by_item[0]))

// Two stat orders: game uses A_STR..A_CHR, item p1 uses 1..6 with CON/DEX swapped.
#define PLAYER_STAT_COUNT 6

static const player_ability sustain_of_stat[PLAYER_STAT_COUNT] = {
    ABILITY_SUSTAIN_STR, ABILITY_SUSTAIN_INT, ABILITY_SUSTAIN_WIS,
    ABILITY_SUSTAIN_DEX, ABILITY_SUSTAIN_CON, ABILITY_SUSTAIN_CHR,
};

static const player_ability sustain_of_item_p1[PLAYER_STAT_COUNT] = {
    ABILITY_SUSTAIN_STR, ABILITY_SUSTAIN_INT, ABILITY_SUSTAIN_WIS,
    ABILITY_SUSTAIN_CON, ABILITY_SUSTAIN_DEX, ABILITY_SUSTAIN_CHR,  // p1: 1..6
};

bool player_can_see_invisible(void) {
    return has(ABILITY_SEE_INVISIBLE);
}

bool player_never_paralyzed(void) {
    return has(ABILITY_NEVER_PARALYZED);
}

bool player_resists_fire(void) {
    return has(ABILITY_RESIST_FIRE);
}

bool player_resists_cold(void) {
    return has(ABILITY_RESIST_COLD);
}

bool player_resists_acid(void) {
    return has(ABILITY_RESIST_ACID);
}

bool player_resists_light(void) {
    return has(ABILITY_RESIST_LIGHT);
}

bool player_takes_no_falling_damage(void) {
    return has(ABILITY_NO_FALLING_DAMAGE);
}

bool player_regenerates(void) {
    return has(ABILITY_REGENERATE);
}

bool player_has_slow_digestion(void) {
    return has(ABILITY_SLOW_DIGESTION);
}

bool player_teleports_randomly(void) {
    return has(ABILITY_TELEPORT_AT_RANDOM);
}

bool player_aggravates_monsters(void) {
    return has(ABILITY_AGGRAVATE);
}

bool player_stat_sustained(int stat) {
    if (stat < 0 || stat >= PLAYER_STAT_COUNT) {
        return false;
    }

    return has(sustain_of_stat[stat]);
}

void player_abilities_forget_all(void) {
    for (int i = 0; i < ABILITY_COUNT; i++) {
        ability_held[i] = false;
    }
}

void player_abilities_note_item_flags(uint32_t item_flags) {
    for (int i = 0; i < GRANTED_BY_ITEM_COUNT; i++) {
        if (granted_by_item[i].item_flag & item_flags) {
            grant(granted_by_item[i].ability);
        }
    }
}

void player_abilities_note_sustain(int item_p1) {
    if (item_p1 < 1 || item_p1 > PLAYER_STAT_COUNT) {
        return;
    }

    grant(sustain_of_item_p1[item_p1 - 1]);
}

void player_grant_see_invisible(void) {
    grant(ABILITY_SEE_INVISIBLE);
}

uint8_t player_abilities_saved_byte(int position) {
    if (position < 0 || position >= ABILITY_COUNT) {
        return 0;
    }

    return ability_held[position];
}

void player_abilities_restore_byte(int position, uint8_t value) {
    if (position < 0 || position >= ABILITY_COUNT) {
        return;
    }

    ability_held[position] = value;
}

static bool weapon_heavy = false;
static int pack_heavy = 0;

bool weapon_is_too_heavy(void) {
    return weapon_heavy;
}

void set_weapon_too_heavy(bool too_heavy) {
    weapon_heavy = too_heavy;
}

int pack_speed_penalty(void) {
    return pack_heavy;
}

void set_pack_speed_penalty(int steps) {
    pack_heavy = steps;
}

static uint16_t the_pounds;  // Zero until creation

int player_body_weight(void) {
    return the_pounds;
}

void player_body_weight_set(int pounds) {
    the_pounds = (uint16_t)pounds;
}

static int16_t worn_armour;
static int16_t magical_plusses;

int player_armour_class(void) {
    return worn_armour + magical_plusses;
}

int player_armour_class_magical(void) {
    return magical_plusses;
}

int player_armour_class_armour(void) {
    return worn_armour;
}

void player_armour_class_reset(int magical) {
    worn_armour = 0;
    magical_plusses = (int16_t)magical;
}

void player_armour_class_set_parts(int armour, int magical) {
    worn_armour = (int16_t)armour;
    magical_plusses = (int16_t)magical;
}

void player_armour_class_add_item(int armour, int magical) {
    worn_armour = (int16_t)(worn_armour + armour);
    magical_plusses = (int16_t)(magical_plusses + magical);
}

void player_armour_class_adjust(int armour) {
    worn_armour = (int16_t)(worn_armour + armour);
}

static int16_t the_aim;
static int16_t the_force;

int player_to_hit_bonus(void) {
    return the_aim;
}

int player_to_damage_bonus(void) {
    return the_force;
}

void player_attack_bonuses_set(int to_hit, int to_damage) {
    the_aim = (int16_t)to_hit;
    the_force = (int16_t)to_damage;
}

void player_to_hit_bonus_adjust(int amount) {
    the_aim = (int16_t)(the_aim + amount);
}

void player_to_damage_bonus_adjust(int amount) {
    the_force = (int16_t)(the_force + amount);
}

static int16_t the_melee;
static int16_t the_bows;

int player_base_to_hit(void) {
    return the_melee;
}

int player_base_to_hit_with_bows(void) {
    return the_bows;
}

void player_base_to_hit_set(int melee, int with_bows) {
    the_melee = (int16_t)melee;
    the_bows = (int16_t)with_bows;
}

void player_base_to_hit_set_melee(int melee) {
    the_melee = (int16_t)melee;
}

void player_base_to_hit_set_with_bows(int with_bows) {
    the_bows = (int16_t)with_bows;
}

void player_base_to_hit_adjust(int melee, int with_bows) {
    the_melee = (int16_t)(the_melee + melee);
    the_bows = (int16_t)(the_bows + with_bows);
}

void player_base_to_hit_adjust_both(int amount) {
    the_melee = (int16_t)(the_melee + amount);
    the_bows = (int16_t)(the_bows + amount);
}

static int16_t shown_to_hit = 0;
static int16_t shown_to_dam = 0;
static int16_t shown_to_ac = 0;
static int16_t shown_ac = 0;

int16_t player_display_to_hit(void) {
    return shown_to_hit;
}

int16_t player_display_to_dam(void) {
    return shown_to_dam;
}

int16_t player_display_to_ac(void) {
    return shown_to_ac;
}

int16_t player_display_ac(void) {
    return shown_ac;
}

void player_display_start_from_real(int16_t to_hit, int16_t to_dam, int16_t to_ac) {
    shown_to_hit = to_hit;
    shown_to_dam = to_dam;
    shown_to_ac = to_ac;
    shown_ac = 0;
}

void player_display_add_to_hit(int amount) {
    shown_to_hit = (int16_t)(shown_to_hit + amount);
}

void player_display_add_to_dam(int amount) {
    shown_to_dam = (int16_t)(shown_to_dam + amount);
}

void player_display_add_to_ac(int amount) {
    shown_to_ac = (int16_t)(shown_to_ac + amount);
}

void player_display_add_ac(int amount) {
    shown_ac = (int16_t)(shown_ac + amount);
}

void player_display_fold_to_ac(void) {
    shown_ac = (int16_t)(shown_ac + shown_to_ac);
}

void player_display_set_to_hit(int16_t value) {
    shown_to_hit = value;
}

void player_display_set_to_dam(int16_t value) {
    shown_to_dam = value;
}

void player_display_set_to_ac(int16_t value) {
    shown_to_ac = value;
}

void player_display_set_ac(int16_t value) {
    shown_ac = value;
}
