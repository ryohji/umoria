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
#include "externs.h"
#include "player_stats.h"
#include "player_identity.h"
#include "player_skills.h"
#include "player_progress.h"

// The four values of each stat (see stats.h).
static uint8_t max_stat[6];
static uint8_t cur_stat[6];
static int16_t mod_stat[6];
static uint8_t use_stat[6];

// A step in a stat-to-bonus table: `bonus` applies from `min_stat` upward,
// until the next entry's `min_stat`. Entries must be in ascending order of
// `min_stat`, and the first entry must be 0 so that every stat value matches.
typedef struct {
    uint8_t min_stat;
    int bonus;
} stat_bonus_step;

// Look up the bonus for `stat` in an ascending table.
// Callers pass `count` via STAT_BONUS_STEPS() so it always matches the table.
static int lookup_stat_bonus(uint8_t stat, const stat_bonus_step *table, size_t count) {
    int bonus = table[0].bonus;

    for (size_t i = 1; i < count && stat >= table[i].min_stat; i++) {
        bonus = table[i].bonus;
    }

    return bonus;
}

// Derive the element count from the table itself, so the two can not drift.
#define STAT_BONUS_STEPS(table) ((sizeof(table)) / (sizeof((table)[0])))

// Adjustment for wisdom/intelligence -JWT-
int stat_adj(int stat) {
    // The original chain tested `value > N` in descending order, so each
    // bonus started at N + 1: `> 117` becomes a lower bound of 118, and
    // so on. Note this is the only adjustment that never returns a
    // penalty -- its floor is 0, not a negative value.
    static const stat_bonus_step by_stat[] = {
        {0, 0}, {8, 1}, {15, 2}, {18, 3}, {68, 4}, {88, 5}, {108, 6}, {118, 7},
    };

    return lookup_stat_bonus(use_stat[stat], by_stat, STAT_BONUS_STEPS(by_stat));
}

// Adjustment for charisma -RAK-
// Percent decrease or increase in price of goods
int chr_adj(void) {
    // Percent of the base price. Below 3 the switch fell through to its
    // default of 100, which is why the table starts there. Steps are wide
    // at the low end (130 -> 125 -> 122) and narrow to one point around
    // 15..18, then widen again above 19.
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

// Returns a character's adjustment to hit. -JWT-
int tohit_adj(void) {
    // Dexterity and strength contribute independently; the two are summed.
    // Note the top steps do not agree: dexterity's runs from 118, strength's
    // from 117.
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

// Returns a character's adjustment to disarm -RAK-
int todis_adj(void) {
    // Note the gaps: 2 is followed by 4, and 6 by 8. Unlike the other
    // adjustments this one does not step by 1, so 3 and 7 never occur.
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

    // **Base to-hit is from the window.** Nine recipes link this file. What is
    // added here is the per-level column from the class table.
    //
    // **Equipment bonus folded once at entry** — nothing between the two lines
    // moves the bonus (observation 35, first form). **Multiplying by BTH_PLUS_ADJ
    // (3) is this screen's rule**, not the window's — melee and throwing add
    // the bonus as-is (player_melee.c, throw.c).
    const int to_hit_bonus = player_to_hit_bonus();
    a.bth = player_base_to_hit() + to_hit_bonus * BTH_PLUS_ADJ + (class_level_adj[player_class()][CLA_BTH] * player_level());
    a.bthb = player_base_to_hit_with_bows() + to_hit_bonus * BTH_PLUS_ADJ + (class_level_adj[player_class()][CLA_BTHB] * player_level());

    // **Inverting and clamping to 0 are this screen's rule**, not the window's —
    // auto-search passes frequency as-is to randint() (player_move.c). 0 when the
    // frequency is >= 40; exceeds 29 when it is < 11 (search gear lowers it,
    // player_bonuses.c)
    a.fos = 40 - player_search_frequency();
    if (a.fos < 0) {
        a.fos = 0;
    }

    // Search chance has no modifiers applied (unlike to-hit, which multiplies
    // the bonus by 3).
    a.srh = player_search_chance();

    // **The +1 is this screen's rule**, not the window's — likert() divides by 1,
    // so this single increment moves the word one step (stl + 1, so the minimum
    // is 1 (not 0)).
    a.stl = player_stealth() + 1;

    a.dis = player_disarm() + 2 * todis_adj() + stat_adj(A_INT) + (class_level_adj[player_class()][CLA_DISARM] * player_level() / 3);
    // **The same window answers both saving throw and device skill** — only what
    // is added differs (A_WIS and CLA_SAVE vs. A_INT and CLA_DEVICE).
    a.save = player_saving_throw() + stat_adj(A_WIS) + (class_level_adj[player_class()][CLA_SAVE] * player_level() / 3);

    // Based on the SAVING THROW, not `disarm`. Preserved as it stands; the
    // intent is unclear but changing it would change the game's behaviour.
    a.dev = player_saving_throw() + stat_adj(A_INT) + (class_level_adj[player_class()][CLA_DEVICE] * player_level() / 3);

    // The window answers in SQUARES; the ten is this screen's own doing, one
    // square being ten feet (player_infra_range.h). The multiplication stays
    // here because it is how the number is shown, not what it is.
    (void)sprintf(a.infra, "%d feet", player_infra_range() * 10);

    return a;
}

// This module does not know what any of the seventeen answers leads to -- no
// messages, no rolls, no drawing.

// The seventeen, IN THE ORDER THE SAVE FILE KEEPS THEM. save.c writes them as
// seventeen bytes in one run, so this order is the file format and cannot move;
// player_abilities_saved_byte() turns a position in the file into one of these.
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

// The count and the save file's run of bytes are the same thing said twice, so
// say so once and for all: adding a member without widening the run would write
// a byte nobody reads back.
_Static_assert(ABILITY_COUNT == PLAYER_ABILITIES_SAVED_BYTES, "the seventeen are seventeen bytes");

// THE SEVENTEEN ANSWERS THEMSELVES. This array is the only place they live, and
// nothing outside this file can name one of them.
//
// All zero at the start, which is the same start the seventeen fields had: A
// CHARACTER CAN DO NOTHING AND RESIST NOTHING until calc_bonuses() works the
// answers out from the equipment, or the save file's seventeen bytes are read
// back in.
//
// A byte, not a bool: a byte read out of a save file is written back unchanged
// (a 5 stays a 5), and anything not zero counts as having the ability.
static uint8_t ability_held[ABILITY_COUNT];

static bool has(player_ability ability) {
    return ability_held[ability] != 0;
}

static void grant(player_ability ability) {
    ability_held[ability] = true;
}

// The eleven that a worn item can grant, each with the flag that grants it. THIS
// TABLE IS THE ONLY PLACE A TR_* CONSTANT IS NAMED, which is what lets
// calc_bonuses() hand over one word and say nothing about what is in it. Nothing
// depends on the order, because granting one ability never affects another.
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

// THE TWO STAT ORDERS. The rest of the game numbers the stats A_STR, A_INT,
// A_WIS, A_DEX, A_CON, A_CHR (constant.h); an item that sustains one stat says
// which in its p1, numbered 1 to 6 as strength, intelligence, wisdom,
// CONSTITUTION, DEXTERITY, charisma. THE FOURTH AND FIFTH ARE SWAPPED between
// the two, so one table cannot serve both and indexing either order by the other
// silently sustains the wrong stat.
#define PLAYER_STAT_COUNT 6

static const player_ability sustain_of_stat[PLAYER_STAT_COUNT] = {
    ABILITY_SUSTAIN_STR, // A_STR
    ABILITY_SUSTAIN_INT, // A_INT
    ABILITY_SUSTAIN_WIS, // A_WIS
    ABILITY_SUSTAIN_DEX, // A_DEX
    ABILITY_SUSTAIN_CON, // A_CON
    ABILITY_SUSTAIN_CHR, // A_CHR
};

static const player_ability sustain_of_item_p1[PLAYER_STAT_COUNT] = {
    ABILITY_SUSTAIN_STR, // p1 == 1
    ABILITY_SUSTAIN_INT, // p1 == 2
    ABILITY_SUSTAIN_WIS, // p1 == 3
    ABILITY_SUSTAIN_CON, // p1 == 4
    ABILITY_SUSTAIN_DEX, // p1 == 5
    ABILITY_SUSTAIN_CHR, // p1 == 6
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

// Two remembered numbers: the only way in is through the windows below.
// The starting values are: no weapon is too heavy for an empty hand,
// and an empty pack costs no speed.
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

// This file reaches out to nothing at all. Creation looks up the race's weight
// columns itself and hands the result in.

// THE POUNDS THEMSELVES. This one short is the only place the answer lives, and the
// two windows below are the only way to reach it.
//
// ZERO IS WHERE A CHARACTER STARTS and it is not a real answer -- a body with no
// weight is a character that creation has not reached yet. THE TESTS LEAN ON THAT
// ZERO: tests/check_strength_test.c:244 expects a carrying limit of 1300, which is
// 10 * PLAYER_WEIGHT_CAP + 0.
static uint16_t the_pounds;

int player_body_weight(void) {
    return the_pounds;
}

void player_body_weight_set(int pounds) {
    // Creation (one line per sex), a saved file's short put back, and the
    // wizard's tweak. One sentence for all three, because all three are a plain
    // replacement -- and there is no `_adjust` to go with it, because NOBODY ADDS
    // TO THIS NUMBER. The character does not put on weight.
    //
    // The cast is the store's own width (uint16_t), not a rule this window adds.
    the_pounds = (uint16_t)pounds;
}

// No externs.h here.

// THE TWO NUMBERS THEMSELVES. These two shorts are the only place the answer
// lives, and the windows below are the only way to reach it.
//
// ZERO IS A REAL ANSWER HERE: a character with nothing worn and a middling
// dexterity has no armour class at all, and the game means it.
static int16_t worn_armour;
static int16_t magical_plusses;

int player_armour_class(void) {
    // The whole point of the module: callers ask for the sum instead of adding
    // the halves. It is an int.
    return worn_armour + magical_plusses;
}

int player_armour_class_magical(void) {
    return magical_plusses;
}

int player_armour_class_armour(void) {
    return worn_armour;
}

void player_armour_class_reset(int magical) {
    // "Nothing is worn yet": the armour half goes to zero and the magical half
    // becomes the dexterity bonus. calc_bonuses() spelled out both assignments
    // before every rebuild.
    worn_armour = 0;
    magical_plusses = (int16_t)magical;
}

void player_armour_class_set_parts(int armour, int magical) {
    // The saved file's two shorts, and create.c's race table. Not _reset(),
    // because the armour half is given here rather than wiped.
    worn_armour = (int16_t)armour;
    magical_plusses = (int16_t)magical;
}

void player_armour_class_add_item(int armour, int magical) {
    // One worn slot: `i_ptr->ac` and `i_ptr->toac` in one window, so the store
    // is touched once per slot instead of twice.
    worn_armour = (int16_t)(worn_armour + armour);
    magical_plusses = (int16_t)(magical_plusses + magical);
}

void player_armour_class_adjust(int armour) {
    // A spell's worth, added to the armour half -- +100 or -100 for
    // invulnerability, +2 or -2 for a blessing. Both signs come through here,
    // which is why one window does the work of the old `+=` and `-=` pair.
    worn_armour = (int16_t)(worn_armour + armour);
}

// No externs.h here: NOT ONE LINE THAT REACHES ANYTHING. The two numbers come from
// the strength and dexterity tables, but this file never reads those tables -- the
// callers look them up and hand the answers in, the same way the body's weight never
// reads race[].

// THE PLACE THE TWO NUMBERS LIVE. These two shorts are the only place the answers
// live, and the five windows below are the only way to reach them.
//
// ZERO IS THE HONEST START. An ordinary character really does add nothing -- the
// dexterity table gives 0 for 8 through 15 and the strength table for 5 through 15
// -- so a character who has not been rolled yet is indistinguishable from an
// average one.
static int16_t the_aim;
static int16_t the_force;

int player_to_hit_bonus(void) {
    return the_aim;
}

int player_to_damage_bonus(void) {
    return the_force;
}

void player_attack_bonuses_set(int to_hit, int to_damage) {
    // Creation's first guess and then its real values, the recalculation that runs
    // when equipment changes, and a saved file's two shorts put back. One sentence
    // for all four, because all four replace the pair outright.
    //
    // The casts are the store's own width, not a rule this window adds: both fields
    // were int16_t and `p_ptr->misc.ptohit = tohit_adj()` truncated exactly like
    // this, so the two statics are int16_t as well.
    the_aim = (int16_t)to_hit;
    the_force = (int16_t)to_damage;
}

void player_to_hit_bonus_adjust(int amount) {
    // One worn or wielded item's tohit. NEGATIVE IS ORDINARY: a cursed weapon
    // takes away.
    the_aim = (int16_t)(the_aim + amount);
}

void player_to_damage_bonus_adjust(int amount) {
    // One worn or wielded item's todam. The caller skips bows before calling ("Bows
    // can't damage. -CJS-"); that condition is about bows, not about this character.
    the_force = (int16_t)(the_force + amount);
}

// No externs.h here.

// THE TWO NUMBERS THEMSELVES. These two shorts are the only place the answer lives,
// and the windows below are the only way to reach it.
//
// ZERO IS WHERE A CHARACTER STARTS AND ALSO A REAL ANSWER: nothing is chosen yet
// until create.c picks a race, and a Human's racial base happens to be zero too.
static int16_t the_melee;
static int16_t the_bows;

int player_base_to_hit(void) {
    return the_melee;
}

int player_base_to_hit_with_bows(void) {
    return the_bows;
}

void player_base_to_hit_set(int melee, int with_bows) {
    // The race's two bases, and the saved file's two shorts put back. One window
    // for both, because both are a plain replacement of the pair (the hit die
    // needed only one door here too; the armour class needed two).
    the_melee = (int16_t)melee;
    the_bows = (int16_t)with_bows;
}

void player_base_to_hit_set_melee(int melee) {
    // The wizard screen, which asks for one number at a time.
    the_melee = (int16_t)melee;
}

void player_base_to_hit_set_with_bows(int with_bows) {
    the_bows = (int16_t)with_bows;
}

void player_base_to_hit_adjust(int melee, int with_bows) {
    // The class's mbth and mbthb, added to what the race left. TWO AMOUNTS,
    // because a class is not equally good at both.
    the_melee = (int16_t)(the_melee + melee);
    the_bows = (int16_t)(the_bows + with_bows);
}

void player_base_to_hit_adjust_both(int amount) {
    // A spell's worth: heroism 12, super heroism 24, a blessing 5, and the same
    // number taken away again when the spell runs out. ONE AMOUNT FOR BOTH
    // NUMBERS, which is what twelve lines in app/dungeon.c were saying six times.
    the_melee = (int16_t)(the_melee + amount);
    the_bows = (int16_t)(the_bows + amount);
}

// Four numbers that are only ever shown need nothing from the game around them. In
// particular this module does not know what the equipment is, whether an item is
// identified or cursed, or how heavy a weapon may be -- the caller decides all of
// that and hands over an amount. It prints nothing, and it does not raise PY_ARMOR
// when the AC moves (player_bonuses.c still watches for that, because it cannot
// print inside a store).

// Owned here and static: the only way in is through the windows below. The four
// have their initial values: a sheet that says nothing is what a program starts
// with, and create.c puts the real numbers in once the character exists (or save.c
// puts back the ones the file remembers).
//
// The names follow the windows rather than the old fields: "dis_tac" was the
// bonus the sheet shows and "dis_ac" the total it shows, which the old pair of
// names does not say.
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
    // The sheet begins as a copy of the real plusses, with none of the armour
    // visible yet: what calc_bonuses() and create.c both spelled out.
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
    // The bonus the sheet shows is part of the total the sheet shows.
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
