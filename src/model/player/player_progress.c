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

// This module knows nothing about what a level is worth, about messages, about
// the hit points, the spells or the mana, and it draws nothing. It works out what
// the experience has paid for; the caller decides what that buys.
//
// The five numbers are owned here and are static; the only way in is through the
// windows in player_level.h. Their initial value is nothing -- a level of zero
// means no character exists yet, and neither the price (player_exp[lev - 1]) nor
// the share of a kill (divided by lev) means anything until creation writes a
// level of one.
static uint16_t player_level_reached;
static int32_t player_experience_held;
static int32_t player_experience_high_water_mark;
static uint16_t player_experience_remainder;
static uint8_t player_experience_percentage;

// The price list stays where it is. It is one of the read-only constant tables
// in externs.h (with player_title, race, class and class_level_adj). Bringing it
// in here would need a setter that no caller in the game would ever use, because
// unlike the hit-point table in hp_table.c nothing ever writes this one.

// The remainder is kept in 65536ths, so this is a whole point of experience.
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

// The table entry is unsigned and so is the factor, so the multiplication and
// the division both happen in unsigned arithmetic and the result is only then
// read as signed -- which is what the `(signed)` casts at the five old call
// sites were doing. There is room: the dearest character pays a hundred and
// sixty-five per cent (a race base of 100, 110, 120 or 125 plus a class share of
// 0, 20, 30, 35 or 40), and 10000000 * 165 is 1.65e9, still inside a signed
// thirty-two-bit number. A factor above about 214 would turn the last few levels
// negative and the character would climb for ever.
int32_t player_experience_to_advance_from(uint16_t level) {
    return (int32_t)(player_exp[level - 1] * player_experience_percentage / 100);
}

int32_t player_experience_needed_to_advance(void) {
    return player_experience_to_advance_from(player_level_reached);
}

void player_gain_experience(int32_t amount) {
    player_experience_held += amount;
}

// A dead monster is worth mexp * its level, shared out by how far the character
// has already come. The part that does not divide evenly is kept, so that a
// high-level character killing something small still creeps towards the next
// point instead of collecting nothing at all, for ever.
//
// The divisor is the level, so this must not be called with a level of nothing.
// The one caller (mon_take_hit) cannot be reached before a character exists.
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

// Nothing is the floor, and the fraction is left alone: losing experience does
// not throw away the part of a point that has been collected.
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

// Arriving at a level with more experience than it costs means several levels
// were paid for at once (or the level was handed over by something other than
// the experience). Half of the surplus goes.
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

// Counting from the bottom of the price list, the way lose_exp() has always done
// it. The walk stops because the last entry in the table (ten million) is dearer
// than the ceiling on experience (MAX_EXP, one less than ten million) for every
// factor of a hundred or more -- THAT ONE DIFFERENCE IS THE WHOLE REASON THIS
// DOES NOT READ PAST THE END OF THE TABLE. It is not checked anywhere, here or
// in the original. Nothing is changed about that; it is written down instead.
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

// THE NUMBER ITSELF. This one byte is the only place it lives, and the windows
// below are the only way to reach it.
//
// Zero is not a die any character plays with -- create.c puts a 6 to a 12 here
// as soon as a race is chosen. Zero only means "no race yet", which is where
// the game starts.
static uint8_t the_faces;

int player_hit_die(void) {
    return the_faces;
}

void player_hit_die_set(int faces) {
    // The race's base, and the saved file's byte put back. One sentence for
    // both, because both are a plain replacement (player_hit_die.h says why
    // player_max_depth needed two doors here and this question does not).
    the_faces = (uint8_t)faces;
}

void player_hit_die_adjust(int faces) {
    // The class's adj_hd, added to whatever the race left here. This was
    // `m_ptr->hitdie += c_ptr->adj_hd;` in create.c: one statement then, one
    // statement now, so the store is still touched once.
    the_faces = (uint8_t)(the_faces + faces);
}

// This module knows nothing about levels, the constitution, poison, resting or
// what dying does -- the caller works out what the maximum should be and how
// fast the store refills, and hands over a number. It prints nothing and raises
// no status flag.

// PLAYER_REGEN_HPBASE and the factor the caller passes are both in 65536ths,
// which is why everything below shifts by 16.
#define HP_FRACTION_FULL 0x10000L

// The three numbers themselves. Nothing outside this file can reach them. All
// three start at nothing: the real values arrive when create.c makes a character,
// or when the save file is read. A START OF ZERO IS NOT THE MARK OF DEATH -- the
// mark is a negative number, so a character who has not been made yet is alive.
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
    // Nothing is clamped on purpose. A fatal wound has to leave the number
    // negative, because that is the only record anyone keeps of the death --
    // including the save file.
    current_hp = (int16_t)(current_hp - damage);
    return player_hp_marks_death();
}

bool player_heal_hp(int amount) {
    if (current_hp >= hp_max) {
        // Already at the top: not healed at all, and the caller stays quiet.
        return false;
    }

    current_hp = (int16_t)(current_hp + amount);

    if (current_hp > hp_max) {
        current_hp = hp_max;

        // A full store has nothing on its way back.
        hp_fraction = 0;
    }

    return true;
}

void player_regenerate_hp(int percent) {
    int16_t before = current_hp;
    int32_t gain = (int32_t)hp_max * percent + PLAYER_REGEN_HPBASE;

    current_hp = (int16_t)(current_hp + (gain >> 16));

    // A maximum large enough to hand back more than a short can hold comes
    // round negative; the old code pinned it at the top instead.
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

        // "must set frac to zero even if equal" -- the old code's words.
        hp_fraction = 0;
    }
}

bool player_change_max_hp(int16_t new_max) {
    if (hp_max == new_max) {
        return false;
    }

    if (hp_max == 0) {
        // Still being made. The old code did not even store the new maximum
        // here -- create.c fills both numbers in a moment later. This is where
        // the mana's window and this one part company: an empty mana store
        // starts full, an empty hit point store is not touched at all.
        return false;
    }

    // What is left keeps its share of the new maximum. Divide before
    // multiplying to stay inside the long, at the cost of a little accuracy --
    // the old code's words.
    int32_t value = (((int32_t)current_hp << 16) + hp_fraction) / hp_max * new_max;
    current_hp = (int16_t)(value >> 16);
    hp_fraction = (uint16_t)(value & 0xFFFF);

    hp_max = new_max;
    return true;
}

void player_gain_temporary_max_hp(int16_t bonus) {
    // Both numbers move together, so how far from the top the character is does
    // not change, and the fraction is left where it is.
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

// The table is owned here and is static: the only way in is through the three
// windows below. It has no initial value: character creation fills every entry
// before anything reads one.
//
// Why the numbers are kept at all: drain life and restore life must not change
// what the character rolled, so the rolls are recorded once and read back,
// never re-rolled.
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

// No externs.h here. This module knows nothing about spells, classes, stats,
// hunger or resting -- the caller works out what the maximum should be and how
// fast the store refills, and hands over a number. It prints nothing and raises
// no status flag.

// PLAYER_REGEN_MNBASE and the factor the caller passes are both in 65536ths,
// which is why everything below shifts by 16.
#define MANA_FRACTION_FULL 0x10000L

// The three numbers themselves. Nothing outside this file can reach them. All
// three start at nothing: the real values arrive when calc_mana() sees the first
// spell learned, or when the save file is read.
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
        // Not enough: the store empties, and so does the fraction. The caller
        // is told how little it got.
        int spent = current_mana;
        current_mana = 0;
        mana_fraction = 0;
        return spent;
    }

    // Enough: the fraction is left alone, so progress towards the next point
    // survives being spent.
    current_mana = (int16_t)(current_mana - cost);
    return cost;
}

void player_regenerate_mana(int percent) {
    int16_t before = current_mana;
    int32_t gain = (int32_t)max_mana * percent + PLAYER_REGEN_MNBASE;

    current_mana = (int16_t)(current_mana + (gain >> 16));

    // A maximum large enough to hand back more than a short can hold comes
    // round negative; the old code pinned it at the top instead.
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

        // A full store has nothing on its way back.
        mana_fraction = 0;
    }
}

bool player_restore_mana(void) {
    if (current_mana >= max_mana) {
        return false;
    }

    // The fraction is deliberately left where it is: the potion says nothing
    // about it, and neither did the old code.
    current_mana = max_mana;
    return true;
}

bool player_change_max_mana(int16_t new_max) {
    if (max_mana == new_max) {
        return false;
    }

    if (max_mana != 0) {
        // What is left keeps its share of the new maximum. Divide before
        // multiplying to stay inside the long, at the cost of a little
        // accuracy -- the old code's words.
        int32_t value = (((int32_t)current_mana << 16) + mana_fraction) / max_mana * new_max;
        current_mana = (int16_t)(value >> 16);
        mana_fraction = (uint16_t)(value & 0xFFFF);
    } else {
        // Nothing to carry across: a character who had no mana starts full.
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

    // The fraction is left alone, as it was before: nothing reads it while the
    // maximum is zero, because regeneration is not even attempted then, and the
    // next maximum to arrive comes through the branch above that clears it.
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

// Owned here and static: the only way in is through the four windows below.
// An empty purse is what a program starts with; character creation puts the
// starting money in at the end.
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

// THE RECORD ITSELF. This one short is the only place it lives, and the windows
// below are the only way to reach it.
//
// Zero -- "has never gone below the town" -- is where every character starts, so
// there is nothing to set up when a game begins.
static uint16_t the_deepest;

int player_max_depth(void) {
    return the_deepest;
}

void player_note_depth_reached(int level) {
    // Keeping the deeper of the two is the whole meaning of the number, so it
    // belongs on this side of the window: there is no way in from outside that
    // can make the record shallower.
    if (level > the_deepest) {
        the_deepest = (uint16_t)level;
    }
}

void player_max_depth_set(int level) {
    // Replacing, not keeping the deeper one. The saved file's short is the whole
    // record, and putting it back has to give the number that was written, so
    // this door does not compare (player_max_depth.h says who may use it).
    the_deepest = (uint16_t)level;
}
