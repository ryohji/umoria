// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#include "config.h"
#include "constant.h"
#include "types.h"
#include "externs.h"
#include "player_progress.h"

// Experience stores. All start at zero (no character).
static uint16_t player_level_reached;
static int32_t player_experience_held;
static int32_t player_experience_high_water_mark;
static uint16_t player_experience_remainder;
static uint8_t player_experience_percentage;

#define EXPERIENCE_FRACTION_FULL 0x10000L

uint16_t player_level(void) {
    return player_level_reached;
}

int32_t player_experience(void) {
    return player_experience_held;
}

int32_t player_max_experience(void) {
    return player_experience_high_water_mark;
}

uint16_t player_experience_fraction(void) {
    return player_experience_remainder;
}

uint8_t player_experience_factor(void) {
    return player_experience_percentage;
}

int32_t player_experience_to_advance_from(uint16_t level) {
    return (int32_t)(player_exp[level - 1] * player_experience_percentage / 100);
}

int32_t player_experience_needed_to_advance(void) {
    return player_experience_to_advance_from(player_level_reached);
}

void player_gain_experience(int32_t amount) {
    player_experience_held += amount;
}

void player_gain_shared_experience(int32_t total) {
    int32_t whole = total / player_level_reached;
    int32_t fraction = (total % player_level_reached) * EXPERIENCE_FRACTION_FULL / player_level_reached + player_experience_remainder;

    if (fraction >= EXPERIENCE_FRACTION_FULL) {
        whole++;
        player_experience_remainder = (uint16_t)(fraction - EXPERIENCE_FRACTION_FULL);
    } else {
        player_experience_remainder = (uint16_t)fraction;
    }

    player_experience_held += whole;
}

void player_lose_experience(int32_t amount) {
    if (amount > player_experience_held) {
        player_experience_held = 0;
    } else {
        player_experience_held -= amount;
    }
}

bool player_cap_experience(void) {
    if (player_experience_held > MAX_EXP) {
        player_experience_held = MAX_EXP;
        return true;
    }

    return false;
}

bool player_deserves_next_level(void) {
    if (player_level_reached >= MAX_PLAYER_LEVEL) {
        return false;
    }

    return player_experience_to_advance_from(player_level_reached) <= player_experience_held;
}

void player_advance_level(void) {
    player_level_reached++;
}

void player_trim_surplus_experience(void) {
    int32_t need_exp = player_experience_to_advance_from(player_level_reached);

    if (player_experience_held > need_exp) {
        int32_t dif_exp = player_experience_held - need_exp;
        player_experience_held = need_exp + (dif_exp / 2);
    }
}

bool player_record_max_experience(void) {
    if (player_experience_held > player_experience_high_water_mark) {
        player_experience_high_water_mark = player_experience_held;
        return true;
    }

    return false;
}

bool player_restore_experience(void) {
    if (player_experience_high_water_mark > player_experience_held) {
        player_experience_held = player_experience_high_water_mark;
        return true;
    }

    return false;
}

uint16_t player_level_deserved_by_experience(void) {
    int level = 1;

    while (player_experience_to_advance_from((uint16_t)level) <= player_experience_held) {
        level++;
    }

    return (uint16_t)level;
}

bool player_recompute_level(void) {
    uint16_t deserved = player_level_deserved_by_experience();

    if (player_level_reached == deserved) {
        return false;
    }

    player_level_reached = deserved;

    return true;
}

void player_set_level(uint16_t value) {
    player_level_reached = value;
}

void player_set_experience(int32_t value) {
    player_experience_held = value;
}

void player_set_max_experience(int32_t value) {
    player_experience_high_water_mark = value;
}

void player_set_experience_fraction(uint16_t value) {
    player_experience_remainder = value;
}

void player_set_experience_factor(uint8_t value) {
    player_experience_percentage = value;
}

// Hit die faces. Starts at zero (no race chosen).
static uint8_t the_faces;

int player_hit_die(void) {
    return the_faces;
}

void player_hit_die_set(int faces) {
    the_faces = (uint8_t)faces;
}

void player_hit_die_adjust(int faces) {
    the_faces = (uint8_t)(the_faces + faces);
}

#define HP_FRACTION_FULL 0x10000L

// HP stores. All start at zero (not made yet; zero is not death mark).
static int16_t hp_max = 0;
static int16_t current_hp = 0;
static uint16_t hp_fraction = 0;

int16_t player_hp(void) {
    return current_hp;
}

int16_t player_max_hp(void) {
    return hp_max;
}

uint16_t player_hp_fraction(void) {
    return hp_fraction;
}

bool player_hp_marks_death(void) {
    return current_hp < 0;
}

bool player_take_hp_damage(int damage) {
    current_hp = (int16_t)(current_hp - damage);
    return player_hp_marks_death();
}

bool player_heal_hp(int amount) {
    if (current_hp >= hp_max) {
        return false;
    }

    current_hp = (int16_t)(current_hp + amount);

    if (current_hp > hp_max) {
        current_hp = hp_max;
        hp_fraction = 0;
    }

    return true;
}

void player_regenerate_hp(int percent) {
    int16_t before = current_hp;
    int32_t gain = (int32_t)hp_max * percent + PLAYER_REGEN_HPBASE;

    current_hp = (int16_t)(current_hp + (gain >> 16));

    // Guard against wraparound.
    if (current_hp < 0 && before > 0) {
        current_hp = MAX_SHORT;
    }

    int32_t carried = (gain & 0xFFFF) + hp_fraction;
    if (carried >= HP_FRACTION_FULL) {
        hp_fraction = (uint16_t)(carried - HP_FRACTION_FULL);
        current_hp = (int16_t)(current_hp + 1);
    } else {
        hp_fraction = (uint16_t)carried;
    }

    if (current_hp >= hp_max) {
        current_hp = hp_max;
        hp_fraction = 0;
    }
}

bool player_change_max_hp(int16_t new_max) {
    if (hp_max == new_max) {
        return false;
    }

    if (hp_max == 0) {
        return false;
    }

    // Carry current proportionally.
    int32_t value = (((int32_t)current_hp << 16) + hp_fraction) / hp_max * new_max;
    current_hp = (int16_t)(value >> 16);
    hp_fraction = (uint16_t)(value & 0xFFFF);

    hp_max = new_max;
    return true;
}

void player_gain_temporary_max_hp(int16_t bonus) {
    hp_max = (int16_t)(hp_max + bonus);
    current_hp = (int16_t)(current_hp + bonus);
}

bool player_lose_temporary_max_hp(int16_t bonus) {
    hp_max = (int16_t)(hp_max - bonus);

    if (current_hp > hp_max) {
        current_hp = hp_max;
        hp_fraction = 0;
        return true;
    }

    return false;
}

void player_reset_hp(int16_t max) {
    hp_max = max;
    current_hp = max;
    hp_fraction = 0;
}

bool player_resurrect_hp(void) {
    if (!player_hp_marks_death()) {
        return false;
    }

    current_hp = 0;
    hp_fraction = 0;
    return true;
}

void player_set_hp(int16_t value) {
    current_hp = value;
}

void player_set_max_hp(int16_t value) {
    hp_max = value;
}

void player_set_hp_fraction(uint16_t value) {
    hp_fraction = value;
}

// HP table by level. No initial value; filled at creation.
static uint16_t hp_by_level[MAX_PLAYER_LEVEL];

uint16_t hp_total_at_level(int level) {
    return hp_by_level[level - 1];
}

void set_hp_total_at_level(int level, uint16_t total) {
    hp_by_level[level - 1] = total;
}

uint16_t *hp_table_slots(void) {
    return hp_by_level;
}

#define MANA_FRACTION_FULL 0x10000L

// Mana stores. All start at zero.
static int16_t max_mana = 0;
static int16_t current_mana = 0;
static uint16_t mana_fraction = 0;

int16_t player_mana(void) {
    return current_mana;
}

int16_t player_max_mana(void) {
    return max_mana;
}

uint16_t player_mana_fraction(void) {
    return mana_fraction;
}

int player_spend_mana(int cost) {
    if (cost > current_mana) {
        int spent = current_mana;
        current_mana = 0;
        mana_fraction = 0;
        return spent;
    }

    current_mana = (int16_t)(current_mana - cost);
    return cost;
}

void player_regenerate_mana(int percent) {
    int16_t before = current_mana;
    int32_t gain = (int32_t)max_mana * percent + PLAYER_REGEN_MNBASE;

    current_mana = (int16_t)(current_mana + (gain >> 16));

    // Guard against wraparound.
    if (current_mana < 0 && before > 0) {
        current_mana = MAX_SHORT;
    }

    int32_t carried = (gain & 0xFFFF) + mana_fraction;
    if (carried >= MANA_FRACTION_FULL) {
        mana_fraction = (uint16_t)(carried - MANA_FRACTION_FULL);
        current_mana = (int16_t)(current_mana + 1);
    } else {
        mana_fraction = (uint16_t)carried;
    }

    if (current_mana >= max_mana) {
        current_mana = max_mana;
        mana_fraction = 0;
    }
}

bool player_restore_mana(void) {
    if (current_mana >= max_mana) {
        return false;
    }

    current_mana = max_mana;
    return true;
}

bool player_change_max_mana(int16_t new_max) {
    if (max_mana == new_max) {
        return false;
    }

    if (max_mana != 0) {
        // Carry current proportionally.
        int32_t value = (((int32_t)current_mana << 16) + mana_fraction) / max_mana * new_max;
        current_mana = (int16_t)(value >> 16);
        mana_fraction = (uint16_t)(value & 0xFFFF);
    } else {
        current_mana = new_max;
        mana_fraction = 0;
    }

    max_mana = new_max;
    return true;
}

bool player_lose_all_mana(void) {
    if (max_mana == 0) {
        return false;
    }

    max_mana = 0;
    current_mana = 0;
    return true;
}

void player_set_mana(int16_t value) {
    current_mana = value;
}

void player_set_max_mana(int16_t value) {
    max_mana = value;
}

void player_set_mana_fraction(uint16_t value) {
    mana_fraction = value;
}

// Gold. Starts at zero.
static int32_t gold = 0;

int32_t player_gold(void) {
    return gold;
}

void player_gain_gold(int32_t amount) {
    gold += amount;
}

void player_pay_gold(int32_t amount) {
    gold -= amount;
}

void player_set_gold(int32_t amount) {
    gold = amount;
}

// Max depth. Starts at zero (never left town).
static uint16_t the_deepest;

int player_max_depth(void) {
    return the_deepest;
}

void player_note_depth_reached(int level) {
    if (level > the_deepest) {
        the_deepest = (uint16_t)level;
    }
}

void player_max_depth_set(int level) {
    the_deepest = (uint16_t)level;
}
