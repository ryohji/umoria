// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 一撃の判定（src/world/combat/hit_rolls.c）のテスト -- 現在の実装を保護する
 *
 * 対象は test_hit・attack_blows・tot_dam・critical_blow の 4 本。どれも
 * 近接と飛び道具の両方から呼ばれるが、それまでどのテストも届いていなかった。
 *
 * 乱数は shared_stubs.c の代役で、決めた 1 つの値を毎回返し、渡された上限を
 * 記録する。上限を読めば、式（命中の i、会心の閾値）を値で確かめられる。
 * 表（class_level_adj・blows_table・c_list）は本物を使い、期待値は表の値で表す。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"
#include "fixture.h"
#include "hit_rolls_fixture.h"
#include "monster_recall.h"
#include "player_identity.h"
#include "player_progress.h"
#include "shared_stubs.h"
#include "player_stats.h"

#define MU_SETUP()                                                             \
    do {                                                                       \
        shared_stubs_reset();                                                  \
        fixture_disturb_reset();                                               \
        player_class_set(0);                                                   \
        player_set_level(0);                                                   \
    } while (0)

#include "minunit.h"

/* ---- test_hit ----------------------------------------------------------- */

/* 2 度目の randint の上限は bth + pth * BTH_PLUS_ADJ + level * class_level_adj[階級][種類] */
TEST(test_hit_rolls_against_the_combined_skill)
{
    int mismatch = 0;
    fixture_set_randint(10);
    for (int cls = 0; cls < MAX_CLASS; cls++) {
        player_class_set(cls);
        for (int type = 0; type < MAX_LEV_ADJ; type++) {
            int expected = 50 + 4 * BTH_PLUS_ADJ + 7 * class_level_adj[cls][type];
            (void)test_hit(50, 7, 4, 0, type);
            if (fixture_randint_last_maxval() != expected) {
                mismatch++;
            }
        }
    }
    ASSERT_EQ_INT(mismatch, 0);
}

/* 目が 1 なら、守りがどれほど低くても外れる */
TEST(test_hit_always_misses_on_one)
{
    fixture_set_randint(1);
    ASSERT_FALSE(test_hit(200, 50, 10, -100, CLA_BTH));
}

/* 目が 20 なら、守りがどれほど高くても当たり、2 度目は振らない */
TEST(test_hit_always_hits_on_twenty_without_a_second_roll)
{
    fixture_set_randint(20);
    bool hit = test_hit(0, 0, 0, 1000, CLA_BTH);
    ASSERT_TRUE(hit && fixture_randint_call_count() == 1);
}

/* それ以外は 2 度目の目が守りを上回れば当たる（10 > 9） */
TEST(test_hit_hits_when_the_roll_beats_the_armour)
{
    fixture_set_randint(10);
    ASSERT_TRUE(test_hit(50, 0, 0, 9, CLA_BTH));
}

/* 同じ値では当たらない（10 > 10 は偽） */
TEST(test_hit_misses_when_the_roll_equals_the_armour)
{
    fixture_set_randint(10);
    ASSERT_FALSE(test_hit(50, 0, 0, 10, CLA_BTH));
}

/* 技能の和が 0 以下（重すぎる武器で pth が負）なら、2 度目を振らずに外れる */
TEST(test_hit_misses_without_a_second_roll_when_skill_is_not_positive)
{
    fixture_set_randint(10);
    bool hit = test_hit(0, 0, -1, -100, CLA_BTH);
    ASSERT_TRUE(!hit && fixture_randint_call_count() == 1);
}

/* 技能の和がちょうど 0（3 + -1 * 3）でも、2 度目を振らずに外れる */
TEST(test_hit_misses_without_a_second_roll_when_skill_is_zero)
{
    fixture_set_randint(10);
    bool hit = test_hit(3, 0, -1, -100, CLA_BTH);
    ASSERT_TRUE(!hit && fixture_randint_call_count() == 1);
}

/* 判定のたびに 1 度 disturb する */
TEST(test_hit_disturbs_the_player_once)
{
    fixture_set_randint(10);
    (void)test_hit(50, 0, 0, 0, CLA_BTH);
    ASSERT_EQ_INT(fixture_disturb_count(), 1);
}

/* ---- attack_blows ------------------------------------------------------- */

static void set_str_dex(int s, int d)
{
    player_stat_set_use(A_STR, (uint8_t)s);
    player_stat_set_use(A_DEX, (uint8_t)d);
}

/* 腕力 * 15 より重い武器は 1 回で、命中に差の分の罰（150 - 151 = -1） */
TEST(attack_blows_too_heavy_gives_one_blow_and_a_penalty)
{
    set_str_dex(10, 3);
    int wtohit = 99;
    int blows = attack_blows(151, &wtohit);
    ASSERT_TRUE(blows == 1 && wtohit == -1);
}

/* ちょうど腕力 * 15 なら重すぎではない。罰は 0 で、表の [0][0] */
TEST(attack_blows_at_the_weight_limit_reads_the_table)
{
    set_str_dex(10, 3);
    int wtohit = 99;
    int blows = attack_blows(150, &wtohit);
    ASSERT_TRUE(blows == (int)blows_table[0][0] && wtohit == 0);
}

/* 器用さの段の境目：9|10、18|19、67|68、107|108、117|118。表の隣りあう列は行に
 * よって同じ値なので、腕力の 7 段（重さ 10、腕力 1・2・3・4・5・7・9）すべてで見る */
TEST(attack_blows_dexterity_steps)
{
    static const int dex[] = {9, 10, 18, 19, 67, 68, 107, 108, 117, 118};
    static const int column[] = {0, 1, 1, 2, 2, 3, 3, 4, 4, 5};
    static const int str[] = {1, 2, 3, 4, 5, 7, 9};
    int mismatch = 0;
    for (int row = 0; row < 7; row++) {
        for (int k = 0; k < 10; k++) {
            set_str_dex(str[row], dex[k]);
            int wtohit;
            if (attack_blows(10, &wtohit) != (int)blows_table[row][column[k]]) {
                mismatch++;
            }
        }
    }
    ASSERT_EQ_INT(mismatch, 0);
}

/* 腕力の段（腕力 * 10 / 重さ）の境目：1|2、2|3、3|4、4|5、6|7、8|9。
 * 器用さの 6 段（3・10・19・68・108・118）すべてで見る */
TEST(attack_blows_strength_steps)
{
    static const int str[] = {19, 20, 29, 30, 39, 40, 49, 50, 69, 70, 89, 90};
    static const int row[] = {0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6};
    static const int dex[] = {3, 10, 19, 68, 108, 118};
    int mismatch = 0;
    for (int column = 0; column < 6; column++) {
        for (int k = 0; k < 12; k++) {
            set_str_dex(str[k], dex[column]);
            int wtohit;
            if (attack_blows(100, &wtohit) != (int)blows_table[row[k]][column]) {
                mismatch++;
            }
        }
    }
    ASSERT_EQ_INT(mismatch, 0);
}

/* ---- tot_dam ------------------------------------------------------------ */

/* c_list から、守りの印 want をすべて持ち、avoid を 1 つも持たない最初の種 */
static creature_handle creature_with(uint16_t want, uint16_t avoid)
{
    for (uint16_t i = 0; i < MAX_CREATURES; i++) {
        creature_handle h = monster_make_creature_handle(i);
        uint16_t d = monster_get_creature(h)->cdefense;
        if ((d & want) == want && (d & avoid) == 0) {
            return h;
        }
    }
    return monster_make_creature_handle(0);
}

static int damage_with(uint8_t tval, uint32_t flags, uint16_t want, uint16_t avoid)
{
    inven_type weapon = {0};
    weapon.tval = tval;
    weapon.flags = flags;
    return tot_dam(&weapon, 10, creature_with(want, avoid));
}

/* 6 つの印と倍率：竜 x4、不死 x3、動物 x2、邪 x2、冷気 x3/2、炎 x3/2。
 * 武器の印は 1 つなので、相手のほかの印は効かない */
TEST(tot_dam_multiplies_for_each_slay)
{
    static const struct {
        uint32_t flag;
        uint16_t defence;
        int damage;
    } cases[] = {
        {TR_SLAY_DRAGON, CD_DRAGON, 40}, {TR_SLAY_UNDEAD, CD_UNDEAD, 30},
        {TR_SLAY_ANIMAL, CD_ANIMAL, 20}, {TR_SLAY_EVIL, CD_EVIL, 20},
        {TR_FROST_BRAND, CD_FROST, 15},  {TR_FLAME_TONGUE, CD_FIRE, 15},
    };
    int mismatch = 0;
    for (int k = 0; k < 6; k++) {
        if (damage_with(TV_SWORD, cases[k].flag, cases[k].defence, 0) != cases[k].damage) {
            mismatch++;
        }
    }
    ASSERT_EQ_INT(mismatch, 0);
}

/* 印が合わなければそのまま */
TEST(tot_dam_without_a_matching_defence_is_unchanged)
{
    ASSERT_EQ_INT(damage_with(TV_SWORD, TR_SLAY_DRAGON, 0, CD_DRAGON), 10);
}

/* 竜で邪な相手に、竜殺しと邪殺しの両方なら、先に見る竜の x4 */
TEST(tot_dam_dragon_comes_before_evil)
{
    ASSERT_EQ_INT(damage_with(TV_SWORD, TR_SLAY_DRAGON | TR_SLAY_EVIL, CD_DRAGON | CD_EVIL, 0), 40);
}

/* 効くのは弾と矢（10〜12）・打撃と長柄と刀剣（21〜23）・油壺だけ */
TEST(tot_dam_applies_only_to_ammo_melee_weapons_and_flasks)
{
    static const uint8_t tvals[] = {TV_SLING_AMMO, TV_ARROW, TV_SPIKE, TV_BOW, TV_HAFTED,
                                    TV_SWORD, TV_SWORD + 1, TV_FLASK};
    static const int expected[] = {40, 40, 10, 10, 40, 40, 10, 40};
    int mismatch = 0;
    for (int k = 0; k < 8; k++) {
        if (damage_with(tvals[k], TR_SLAY_DRAGON, CD_DRAGON, 0) != expected[k]) {
            mismatch++;
        }
    }
    ASSERT_EQ_INT(mismatch, 0);
}

/* 効いた印は、モンスターの記憶に足される */
TEST(tot_dam_records_the_defence_it_used)
{
    creature_handle h = creature_with(CD_UNDEAD, CD_DRAGON);
    recall_get(h)->r_cdefense = 0;
    inven_type weapon = {0};
    weapon.tval = TV_HAFTED;
    weapon.flags = TR_SLAY_UNDEAD;
    (void)tot_dam(&weapon, 10, h);
    ASSERT_EQ_INT(recall_get(h)->r_cdefense, CD_UNDEAD);
}

/* ---- critical_blow ------------------------------------------------------ */

/* 最初の上限は 5000、会心のときの 2 度目は 650 */
TEST(critical_blow_rolls_5000_then_650)
{
    fixture_set_randint(100);
    (void)critical_blow(100, 0, 10, CLA_BTH);
    ASSERT_TRUE(fixture_randint_call_count() == 2 && fixture_randint_last_maxval() == 650);
}

/* 目が閾値（重さ + 5 * 修正）以下なら会心。重さ 99 で目 100 は外れで、そのまま */
TEST(critical_blow_misses_above_the_threshold)
{
    fixture_set_randint(100);
    int dam = critical_blow(99, 0, 10, CLA_BTH);
    ASSERT_TRUE(dam == 10 && fixture_message_count() == 0 && fixture_randint_call_count() == 1);
}

/* 修正は 5 倍で効く：重さ 95 と +1 で閾値 100 */
TEST(critical_blow_counts_plus_five_times)
{
    fixture_set_randint(100);
    ASSERT_EQ_INT(critical_blow(95, 1, 10, CLA_BTH), 25);
}

/* 階級の補正 * レベルも閾値に入る。ちょうど閾値なら会心、1 上なら外れ */
TEST(critical_blow_counts_class_and_level)
{
    player_class_set(0);
    player_set_level(10);
    int threshold = class_level_adj[0][CLA_BTH] * 10;
    fixture_set_randint(threshold);
    bool at = critical_blow(0, 0, 10, CLA_BTH) != 10;
    fixture_set_randint(threshold + 1);
    bool above = critical_blow(0, 0, 10, CLA_BTH) != 10;
    ASSERT_TRUE(threshold > 0 && at && !above);
}

/* 重さ + 2 度目の目（100）で段が決まる：400 未満 x2+5、700 未満 x3+10、900 未満 x4+15、それ以上 x5+20 */
TEST(critical_blow_weight_steps)
{
    static const int weight[] = {299, 300, 599, 600, 799, 800};
    static const int expected[] = {25, 40, 40, 55, 55, 70};
    int mismatch = 0;
    fixture_set_randint(100);
    for (int k = 0; k < 6; k++) {
        if (critical_blow(weight[k], 0, 10, CLA_BTH) != expected[k]) {
            mismatch++;
        }
    }
    ASSERT_EQ_INT(mismatch, 0);
}

/* 段ごとのメッセージ */
TEST(critical_blow_messages)
{
    static const int weight[] = {100, 300, 600, 800};
    static const char *const expected[] = {
        "It was a good hit! (x2 damage)", "It was an excellent hit! (x3 damage)",
        "It was a superb hit! (x4 damage)", "It was a *GREAT* hit! (x5 damage)"};
    int mismatch = 0;
    fixture_set_randint(100);
    for (int k = 0; k < 4; k++) {
        (void)critical_blow(weight[k], 0, 10, CLA_BTH);
        if (strcmp(fixture_message_text(k), expected[k]) != 0) {
            mismatch++;
        }
    }
    ASSERT_TRUE(mismatch == 0 && fixture_message_count() == 4);
}

int main(void)
{
    RUN_TEST(test_hit_rolls_against_the_combined_skill);
    RUN_TEST(test_hit_always_misses_on_one);
    RUN_TEST(test_hit_always_hits_on_twenty_without_a_second_roll);
    RUN_TEST(test_hit_hits_when_the_roll_beats_the_armour);
    RUN_TEST(test_hit_misses_when_the_roll_equals_the_armour);
    RUN_TEST(test_hit_misses_without_a_second_roll_when_skill_is_not_positive);
    RUN_TEST(test_hit_misses_without_a_second_roll_when_skill_is_zero);
    RUN_TEST(test_hit_disturbs_the_player_once);
    RUN_TEST(attack_blows_too_heavy_gives_one_blow_and_a_penalty);
    RUN_TEST(attack_blows_at_the_weight_limit_reads_the_table);
    RUN_TEST(attack_blows_dexterity_steps);
    RUN_TEST(attack_blows_strength_steps);
    RUN_TEST(tot_dam_multiplies_for_each_slay);
    RUN_TEST(tot_dam_without_a_matching_defence_is_unchanged);
    RUN_TEST(tot_dam_dragon_comes_before_evil);
    RUN_TEST(tot_dam_applies_only_to_ammo_melee_weapons_and_flasks);
    RUN_TEST(tot_dam_records_the_defence_it_used);
    RUN_TEST(critical_blow_rolls_5000_then_650);
    RUN_TEST(critical_blow_misses_above_the_threshold);
    RUN_TEST(critical_blow_counts_plus_five_times);
    RUN_TEST(critical_blow_counts_class_and_level);
    RUN_TEST(critical_blow_weight_steps);
    RUN_TEST(critical_blow_messages);
    return TEST_SUMMARY();
}
