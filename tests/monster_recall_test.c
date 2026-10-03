// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// モンスターの記憶（monster_recall.c）の規則：OR で足す、最大を保つ、上限で止まる。

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"
#include "monster_recall.h"

#include "minunit.h"

// recall_update_characteristics は OR で設定する
TEST(recall_update_characteristics_or) {
    creature_handle h = {0};
    recall_type *r = recall_get(h);
    r->r_cdefense = 0;

    recall_update_characteristics(h, 0x01);
    ASSERT_EQ_INT(0x01, r->r_cdefense);

    recall_update_characteristics(h, 0x02);
    ASSERT_EQ_INT(0x03, r->r_cdefense);

    recall_update_characteristics(h, 0x01);
    ASSERT_EQ_INT(0x03, r->r_cdefense);
}

// recall_update_move は OR で設定する
TEST(recall_update_move_or) {
    creature_handle h = {1};
    recall_type *r = recall_get(h);
    r->r_cmove = 0;

    recall_update_move(h, 0x04);
    ASSERT_EQ_INT(0x04, r->r_cmove);

    recall_update_move(h, 0x08);
    ASSERT_EQ_INT(0x0C, r->r_cmove);

    recall_update_move(h, 0x04);
    ASSERT_EQ_INT(0x0C, r->r_cmove);
}

// recall_update_carry は最大値を保持する
TEST(recall_update_carry_max) {
    creature_handle h = {2};
    recall_type *r = recall_get(h);
    r->r_cmove = 0;

    recall_update_carry(h, 3);
    ASSERT_EQ_INT(3U << CM_TR_SHIFT, r->r_cmove & CM_TREASURE);

    recall_update_carry(h, 5);
    ASSERT_EQ_INT(5U << CM_TR_SHIFT, r->r_cmove & CM_TREASURE);

    recall_update_carry(h, 2);
    ASSERT_EQ_INT(5U << CM_TR_SHIFT, r->r_cmove & CM_TREASURE);

    recall_update_carry(h, 7);
    ASSERT_EQ_INT(7U << CM_TR_SHIFT, r->r_cmove & CM_TREASURE);
}

// recall_update_carry は CM_TREASURE 以外のビットを保持する
TEST(recall_update_carry_preserves_other_bits) {
    creature_handle h = {3};
    recall_type *r = recall_get(h);
    r->r_cmove = 0x000000FF;

    recall_update_carry(h, 4);
    ASSERT_EQ_INT(0x000000FF, r->r_cmove & 0x000000FF);
    ASSERT_EQ_INT(4U << CM_TR_SHIFT, r->r_cmove & CM_TREASURE);
}

// recall_update_spell は OR で設定する
TEST(recall_update_spell_or) {
    creature_handle h = {4};
    recall_type *r = recall_get(h);
    r->r_spells = 0;

    recall_update_spell(h, 0x10);
    ASSERT_EQ_INT(0x10, r->r_spells);

    recall_update_spell(h, 0x20);
    ASSERT_EQ_INT(0x30, r->r_spells);

    recall_update_spell(h, 0x10);
    ASSERT_EQ_INT(0x30, r->r_spells);
}

// recall_increment_spell_chance は CS_FREQ で飽和する
TEST(recall_increment_spell_chance_saturates) {
    creature_handle h = {5};
    recall_type *r = recall_get(h);
    r->r_spells = 0;

    for (int i = 0; i < 20; i++) {
        recall_increment_spell_chance(h);
    }

    ASSERT_EQ_INT(CS_FREQ, r->r_spells & CS_FREQ);
}

// recall_increment_spell_chance は CS_FREQ 以外のビットを保持する
TEST(recall_increment_spell_chance_preserves_other_bits) {
    creature_handle h = {6};
    recall_type *r = recall_get(h);
    r->r_spells = 0x00000100;

    recall_increment_spell_chance(h);
    ASSERT_EQ_INT(0x00000101, r->r_spells);

    for (int i = 0; i < 20; i++) {
        recall_increment_spell_chance(h);
    }

    ASSERT_EQ_INT(0x00000100, r->r_spells & 0xFFFFFFF0);
    ASSERT_EQ_INT(CS_FREQ, r->r_spells & CS_FREQ);
}

// recall_increment_kill は MAX_SHORT で飽和する
TEST(recall_increment_kill_saturates) {
    creature_handle h = {7};
    recall_type *r = recall_get(h);
    r->r_kills = MAX_SHORT - 2;

    recall_increment_kill(h);
    ASSERT_EQ_INT(MAX_SHORT - 1, r->r_kills);

    recall_increment_kill(h);
    ASSERT_EQ_INT(MAX_SHORT, r->r_kills);

    recall_increment_kill(h);
    ASSERT_EQ_INT(MAX_SHORT, r->r_kills);

    recall_increment_kill(h);
    ASSERT_EQ_INT(MAX_SHORT, r->r_kills);
}

// recall_increment_death は MAX_SHORT で飽和する
TEST(recall_increment_death_saturates) {
    creature_handle h = {8};
    recall_type *r = recall_get(h);
    r->r_deaths = MAX_SHORT - 2;

    recall_increment_death(h);
    ASSERT_EQ_INT(MAX_SHORT - 1, r->r_deaths);

    recall_increment_death(h);
    ASSERT_EQ_INT(MAX_SHORT, r->r_deaths);

    recall_increment_death(h);
    ASSERT_EQ_INT(MAX_SHORT, r->r_deaths);

    recall_increment_death(h);
    ASSERT_EQ_INT(MAX_SHORT, r->r_deaths);
}

int main(void) {
    RUN_TEST(recall_update_characteristics_or);
    RUN_TEST(recall_update_move_or);
    RUN_TEST(recall_update_carry_max);
    RUN_TEST(recall_update_carry_preserves_other_bits);
    RUN_TEST(recall_update_spell_or);
    RUN_TEST(recall_increment_spell_chance_saturates);
    RUN_TEST(recall_increment_spell_chance_preserves_other_bits);
    RUN_TEST(recall_increment_kill_saturates);
    RUN_TEST(recall_increment_death_saturates);

    TEST_SUMMARY();
    return mu_tests_failed;
}
