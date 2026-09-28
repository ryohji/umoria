// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「体力」のテスト -- 現在のふるまいを保護する
 *
 * py から出す 5 つめの問い（1 つめは持っている金、2 つめは腹の具合、3 つめは
 * 画面に出す数字、4 つめは魔力）。フィールドは 3 つあるが 1 つの問いに
 * 答えている —— もとは py.misc.chp（残り）・py.misc.mhp（上限）・
 * py.misc.chp_frac（まだ 1 点に届いていない端数）。
 *
 * **端数は魔力と同じ理由でここにある。** 再生は 1 ターンに
 * 上限 × 数百 / 65536（PLAYER_REGEN_NORMAL は 197）＋ 底上げ
 * PLAYER_REGEN_HPBASE（1442）しか返さないので、端数を置く場所が無ければ
 * 永久に 1 点も戻らない。そこは player_mana_test.c と同じ形。
 *
 * **この単位で新しいのは「負の値は量ではない」ということ。** 負の体力は
 * 死んでいる人物の印で、しかもそれが唯一の印 —— 0 で下げ止める場所が
 * どこにも無いので、致命の一撃は数を負のまま残し、**セーブファイルを
 * またいで負のまま生きのびる**。読み手は 4 か所（take_hit・ロード直後の
 * main.c・死因を上書きするか決める save.c・wizard の蘇生）。
 * **だから「0 で下げ止める」ように直したら 4 か所が黙って壊れる。**
 * ここに釘を打つのがこのテストの主眼。
 *
 * 保護したい性質は 8 つ。
 *
 *   1. **走りだしは 3 つとも 0**（もとの 3 つは初期化子なしの構造体の中に
 *      あった）。本物の値は create.c が人物を作るときに、あるいは save.c が
 *      ファイルから読んで置く。
 *
 *   2. **負が死の印で、ちょうど 0 は死んでいない**（もとの判定は `chp < 0`）。
 *      ちょうど払いきる一撃は生きのびる。
 *
 *   3. **傷は頭打ちにしない。** 致命なら負のまま置いて、窓口は「致命だったか」
 *      を返す。死が何を引きおこすか（死の旗・死因・階を出る）は呼び手の言葉。
 *
 *   4. **傷は端数を触らない。**
 *
 *   5. **治すのは満杯の人には効かない**（もとの `chp < mhp`）。超えたら
 *      頭打ちにして端数を消す。届かなければ端数はそのまま。
 *
 *   6. **満杯で止まり、そのとき端数は消える**こと（もとのコードの
 *      「must set frac to zero even if equal」）。
 *
 *   7. **上限が動いたら残りは比例で連れていく**（端数込み。「先に割って
 *      溢れを避ける。精度は少し落ちる」という落ちかたまで写す）。
 *      **ただし上限 0 は何もしない** —— 魔力は「満杯で始まる」にしたが、
 *      体力の上限が 0 なのは人物を作っている最中だけで、もとのコードは
 *      新しい上限の代入すらしなかった。**魔力の窓口を写せない 1 か所。**
 *
 *   8. **一時的な上限（英雄 +10・超英雄 +20）は上げるとき両方を動かし、
 *      下げるとき上限だけ引いてから残りを頭打ちにする。** 頭打ちになったか
 *      を返すのは、呼び手がそのときだけ画面を書きなおすから。
 *
 * テストは 1 プロセスで状態を共有するので、走りだしを見る 1 件は main() の
 * 先頭に置き、以降は各件が最初に窓口で足場を作る。
 */
/* externs.h は要らない。窓口と、int16_t・MAX_SHORT・PLAYER_REGEN_* のための
 * types.h と constant.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_hp.h"

#include "minunit.h"

/* 上限 10 の人物が毒でもなく休んでもいないときの 1 ターンぶん（65536 分の）。
 * dungeon.c が渡すのは PLAYER_REGEN_NORMAL で、そこから
 * 10 * 197 + 1442 = 3412 が出る。65536 / 3412 = 19.2 ターンで 1 点。 */
#define GAIN_PER_TURN (10 * PLAYER_REGEN_NORMAL + PLAYER_REGEN_HPBASE)

/* 3 つをまとめて置く（各テストの足場作り）。 */
static void given(int16_t max, int16_t current, uint16_t fraction) {
    player_set_max_hp(max);
    player_set_hp(current);
    player_set_hp_fraction(fraction);
}

/* --- 走りだし ------------------------------------------------------------- */
/* この 1 件は main() の先頭で走らせる。ほかのテストが状態を動かすので。 */

TEST(the_three_numbers_start_at_nothing) {
    ASSERT_EQ_INT(0, player_hp());
    ASSERT_EQ_INT(0, player_max_hp());
    ASSERT_EQ_INT(0, player_hp_fraction());

    /* 走りだしの人物は死んでいない（0 は印ではない）。 */
    ASSERT_TRUE(!player_hp_marks_death());
}

/* 3 つが別々の器であること。save.c の読みが取りちがえたら気づけるように
 * （3 つとも同じ順で並んでいるので入れかえが起きやすい）。 */
TEST(each_number_stands_on_its_own) {
    given(40, 17, 999);

    ASSERT_EQ_INT(17, player_hp());
    ASSERT_EQ_INT(40, player_max_hp());
    ASSERT_EQ_INT(999, player_hp_fraction());
}

/* --- 死の印 -------------------------------------------------------------- */

/* ちょうど 0 は死んでいない。もとの判定が `chp < 0` であって
 * `chp <= 0` でないことの釘。 */
TEST(exactly_nothing_left_is_not_the_mark_of_death) {
    given(10, 0, 0);

    ASSERT_TRUE(!player_hp_marks_death());
}

TEST(a_negative_number_is_the_mark_of_death) {
    given(10, -1, 0);

    ASSERT_TRUE(player_hp_marks_death());
}

/* 致命の一撃は数を負のまま置く。**頭打ちにしない** —— それが唯一の印で、
 * セーブファイルをまたいで死を覚えている。 */
TEST(a_fatal_wound_leaves_the_number_negative_on_purpose) {
    given(10, 3, 0);

    ASSERT_TRUE(player_take_hp_damage(5));

    ASSERT_EQ_INT(-2, player_hp());
    ASSERT_TRUE(player_hp_marks_death());
}

TEST(a_wound_that_is_not_fatal_says_so) {
    given(10, 8, 0);

    ASSERT_TRUE(!player_take_hp_damage(3));

    ASSERT_EQ_INT(5, player_hp());
    ASSERT_TRUE(!player_hp_marks_death());
}

/* ちょうど残りぜんぶを持っていく一撃では死なない（境目）。 */
TEST(a_wound_that_takes_exactly_everything_is_not_fatal) {
    given(10, 5, 0);

    ASSERT_TRUE(!player_take_hp_damage(5));

    ASSERT_EQ_INT(0, player_hp());
    ASSERT_TRUE(!player_hp_marks_death());
}

TEST(a_wound_leaves_the_fraction_alone) {
    given(10, 5, 0x4000);

    (void)player_take_hp_damage(2);

    ASSERT_EQ_INT(3, player_hp());
    ASSERT_EQ_INT(0x4000, player_hp_fraction());
}

/* 無敵のとき take_hit() は damage を 0 にしてから渡す。 */
TEST(no_damage_at_all_changes_nothing) {
    given(10, 7, 0x1111);

    ASSERT_TRUE(!player_take_hp_damage(0));

    ASSERT_EQ_INT(7, player_hp());
    ASSERT_EQ_INT(0x1111, player_hp_fraction());
}

/* wizard の蘇生。印を消す唯一の場所で、端数も消す。 */
TEST(resurrection_rubs_out_the_mark) {
    given(10, -7, 0x3333);

    ASSERT_TRUE(player_resurrect_hp());

    ASSERT_EQ_INT(0, player_hp());
    ASSERT_EQ_INT(0, player_hp_fraction());
    ASSERT_TRUE(!player_hp_marks_death());
}

TEST(resurrecting_a_living_character_changes_nothing) {
    given(10, 0, 0x2222);

    ASSERT_TRUE(!player_resurrect_hp());

    ASSERT_EQ_INT(0, player_hp());
    ASSERT_EQ_INT(0x2222, player_hp_fraction());
}

/* --- 治す（hp_player の中身） ----------------------------------------------- */

TEST(healing_adds_and_says_so) {
    given(20, 5, 0);

    ASSERT_TRUE(player_heal_hp(7));

    ASSERT_EQ_INT(12, player_hp());
}

/* 満杯の人には効かない。もとの `chp < mhp` の釘で、呼び手はこの戻り値を見て
 * 「気分がよくなった」の知らせを出すかどうかを決める。 */
TEST(healing_a_character_at_the_top_does_nothing) {
    given(20, 20, 0);

    ASSERT_TRUE(!player_heal_hp(7));

    ASSERT_EQ_INT(20, player_hp());
}

TEST(healing_stops_at_the_top_and_clears_the_fraction) {
    given(20, 18, 50000);

    ASSERT_TRUE(player_heal_hp(7));

    ASSERT_EQ_INT(20, player_hp());
    ASSERT_EQ_INT(0, player_hp_fraction());
}

/* 頭打ちに届かなかったぶんには端数を触らない —— 次の 1 点への道のりは
 * 治療で失われない。 */
TEST(healing_short_of_the_top_keeps_the_fraction) {
    given(20, 5, 50000);

    ASSERT_TRUE(player_heal_hp(7));

    ASSERT_EQ_INT(12, player_hp());
    ASSERT_EQ_INT(50000, player_hp_fraction());
}

/* 治すほうは死の印を特別扱いしない（もとの hp_player もしていなかった）。
 * 実際の道では致命の直後に階を出るので通らないが、窓口が勝手に
 * 「死んでいるなら治さない」を足していないことの釘。 */
TEST(healing_does_not_special_case_the_mark_of_death) {
    given(20, -2, 0);

    ASSERT_TRUE(player_heal_hp(5));

    ASSERT_EQ_INT(3, player_hp());
    ASSERT_TRUE(!player_hp_marks_death());
}

/* --- 再生（regenhp の中身） ------------------------------------------------- */

/* 端数の存在理由そのもの。上限 10・速さ 197 なら 1 ターンぶんは 3412 で、
 * 19 ターンでは 1 点も戻らず（19 * 3412 = 64828 < 65536）、
 * **20 ターン目に 1 点**になって 2704 余る。 */
TEST(the_fraction_is_what_makes_a_slow_recovery_add_up) {
    given(10, 0, 0);

    for (int turn = 0; turn < 19; turn++) {
        player_regenerate_hp(PLAYER_REGEN_NORMAL);
    }

    ASSERT_EQ_INT(0, player_hp());
    ASSERT_EQ_INT(19 * GAIN_PER_TURN, player_hp_fraction());

    player_regenerate_hp(PLAYER_REGEN_NORMAL);

    ASSERT_EQ_INT(1, player_hp());
    ASSERT_EQ_INT(20 * GAIN_PER_TURN - 0x10000, player_hp_fraction());
}

/* 上限が大きければ 1 ターンで整数部が動く。3000 * 197 + 1442 = 592442 で、
 * 65536 で割ると 9 点と 2618 の余り。 */
TEST(a_big_store_comes_back_by_whole_points) {
    given(3000, 0, 0);

    player_regenerate_hp(PLAYER_REGEN_NORMAL);

    ASSERT_EQ_INT(9, player_hp());
    ASSERT_EQ_INT(3000 * PLAYER_REGEN_NORMAL + PLAYER_REGEN_HPBASE - 9 * 0x10000,
                  player_hp_fraction());
}

/* 「must set frac to zero even if equal」。持ちこしが 1 点になって
 * ちょうど満杯に届いた場合も端数は消える。 */
TEST(recovery_stops_at_the_top_and_clears_the_fraction) {
    given(10, 9, 65000);

    player_regenerate_hp(PLAYER_REGEN_NORMAL);

    ASSERT_EQ_INT(10, player_hp());
    ASSERT_EQ_INT(0, player_hp_fraction());
}

TEST(recovery_does_not_overshoot_the_top) {
    given(10, 9, 0);

    player_regenerate_hp(65535);

    ASSERT_EQ_INT(10, player_hp());
    ASSERT_EQ_INT(0, player_hp_fraction());
}

/* 溢れの見はり。上限が大きいと整数部が short を越えて負にまわりこむので、
 * もとのコードは頭で止めていた（`chp < 0 && old > 0` なら MAX_SHORT）。
 * **この釘が無いと、まわりこんだ負の値が「死んでいる」印に化ける。** */
TEST(recovery_pins_a_runaway_sum_at_the_top) {
    given(MAX_SHORT, 5, 0);

    player_regenerate_hp(65535);

    ASSERT_EQ_INT(MAX_SHORT, player_hp());
    ASSERT_EQ_INT(0, player_hp_fraction());
    ASSERT_TRUE(!player_hp_marks_death());
}

/* --- 上限が動いたとき（calc_hitpoints の比例） ----------------------------- */

/* 端数込みで比例配分する。10 の半分と少し（5 と 0x8000）が 20 では
 * 10 と 65520 —— 「先に割って溢れを避ける」ので 65536 にならず 16 足りない。
 * その精度の落ちかたまで写しとる。 */
TEST(a_new_maximum_carries_what_is_left_across_in_proportion) {
    given(10, 5, 0x8000);

    ASSERT_TRUE(player_change_max_hp(20));

    ASSERT_EQ_INT(20, player_max_hp());
    ASSERT_EQ_INT(10, player_hp());
    ASSERT_EQ_INT(65520, player_hp_fraction());
}

TEST(a_smaller_maximum_takes_what_is_left_down_with_it) {
    given(20, 10, 0);

    ASSERT_TRUE(player_change_max_hp(10));

    ASSERT_EQ_INT(10, player_max_hp());
    ASSERT_EQ_INT(5, player_hp());
    ASSERT_EQ_INT(0, player_hp_fraction());
}

/* **魔力の窓口を写せない 1 か所。** 上限 0 は「人物を作っている最中」で、
 * もとの calc_hitpoints() は新しい上限の代入すらしなかった（create.c が
 * すぐあとに両方を埋める）。魔力は同じ場面を「満杯で始まる」にしている。 */
TEST(a_maximum_of_zero_is_left_alone_entirely) {
    given(0, 0, 0);

    ASSERT_TRUE(!player_change_max_hp(50));

    ASSERT_EQ_INT(0, player_max_hp());
    ASSERT_EQ_INT(0, player_hp());
}

TEST(the_same_maximum_changes_nothing) {
    given(10, 5, 0x1234);

    ASSERT_TRUE(!player_change_max_hp(10));

    ASSERT_EQ_INT(10, player_max_hp());
    ASSERT_EQ_INT(5, player_hp());
    ASSERT_EQ_INT(0x1234, player_hp_fraction());
}

/* --- 一時的な上限（英雄 +10・超英雄 +20） --------------------------------- */

/* 上げるときは両方に足す —— 満杯からの距離が変わらない。端数も触らない。 */
TEST(a_temporary_maximum_lifts_both_numbers_together) {
    given(50, 30, 0x1000);

    player_gain_temporary_max_hp(10);

    ASSERT_EQ_INT(60, player_max_hp());
    ASSERT_EQ_INT(40, player_hp());
    ASSERT_EQ_INT(0x1000, player_hp_fraction());
}

/* 切れるときは上限だけ引いてから残りを頭打ちに。頭打ちになったかを返すのは、
 * 呼び手（dungeon.c）がそのときだけ prt_chp() を呼ぶから。 */
TEST(a_temporary_maximum_wearing_off_caps_what_is_left) {
    given(60, 60, 0x2000);

    ASSERT_TRUE(player_lose_temporary_max_hp(10));

    ASSERT_EQ_INT(50, player_max_hp());
    ASSERT_EQ_INT(50, player_hp());
    ASSERT_EQ_INT(0, player_hp_fraction());
}

TEST(a_temporary_maximum_wearing_off_need_not_cap_anything) {
    given(60, 30, 0x2000);

    ASSERT_TRUE(!player_lose_temporary_max_hp(10));

    ASSERT_EQ_INT(50, player_max_hp());
    ASSERT_EQ_INT(30, player_hp());
    ASSERT_EQ_INT(0x2000, player_hp_fraction());
}

/* --- 上限を決めて満たす（create.c と wizard.c が同じ 3 行を書いていた） --- */

TEST(a_brand_new_maximum_fills_to_the_top) {
    given(10, 3, 0x999);

    player_reset_hp(41);

    ASSERT_EQ_INT(41, player_max_hp());
    ASSERT_EQ_INT(41, player_hp());
    ASSERT_EQ_INT(0, player_hp_fraction());
}

/* --- 通し ---------------------------------------------------------------- */

/* 呼び手が実際に通す順で一巡する。人物を作る → 傷を受ける → 休んで戻る →
 * 治療 → 英雄になって切れる → 階級が上がる → 死ぬ → 蘇る。 */
TEST(a_whole_session_follows_the_callers_order) {
    /* create.c: 上限を決めて満たす。 */
    player_reset_hp(19);
    ASSERT_EQ_INT(19, player_hp());

    /* moria1.c take_hit: 致命ではない傷。 */
    ASSERT_TRUE(!player_take_hp_damage(6));
    ASSERT_EQ_INT(13, player_hp());

    /* dungeon.c regenhp: 70 ターン休む。1 ターンぶんは 19 * 197 + 1442 = 5185
     * で、70 ターンで 362950 —— 5 点と 35270 の余り。 */
    for (int turn = 0; turn < 70; turn++) {
        player_regenerate_hp(PLAYER_REGEN_NORMAL);
    }
    ASSERT_EQ_INT(18, player_hp());
    ASSERT_EQ_INT(70 * (19 * PLAYER_REGEN_NORMAL + PLAYER_REGEN_HPBASE) - 5 * 0x10000,
                  player_hp_fraction());

    /* spells.c hp_player: 頭打ちまで治って端数が消える。 */
    ASSERT_TRUE(player_heal_hp(5));
    ASSERT_EQ_INT(19, player_hp());
    ASSERT_EQ_INT(0, player_hp_fraction());

    /* dungeon.c: 英雄になって、切れる。 */
    player_gain_temporary_max_hp(10);
    ASSERT_EQ_INT(29, player_hp());
    ASSERT_TRUE(player_lose_temporary_max_hp(10));
    ASSERT_EQ_INT(19, player_hp());

    /* misc3.c calc_hitpoints: 階級が上がって上限が増える（満杯のままなので
     * 比例でもぴったり満杯）。 */
    ASSERT_TRUE(player_change_max_hp(30));
    ASSERT_EQ_INT(30, player_max_hp());
    ASSERT_EQ_INT(30, player_hp());

    /* moria1.c take_hit: 致命の一撃。負のまま置く。 */
    ASSERT_TRUE(player_take_hp_damage(40));
    ASSERT_EQ_INT(-10, player_hp());
    ASSERT_TRUE(player_hp_marks_death());

    /* save.c: wizard の蘇生。印を消す唯一の場所。 */
    ASSERT_TRUE(player_resurrect_hp());
    ASSERT_EQ_INT(0, player_hp());
    ASSERT_TRUE(!player_hp_marks_death());
}

int main(void) {
    /* 走りだしの状態を見る 1 件を最初に。以降のテストが状態を動かす。 */
    RUN_TEST(the_three_numbers_start_at_nothing);

    RUN_TEST(each_number_stands_on_its_own);

    RUN_TEST(exactly_nothing_left_is_not_the_mark_of_death);
    RUN_TEST(a_negative_number_is_the_mark_of_death);
    RUN_TEST(a_fatal_wound_leaves_the_number_negative_on_purpose);
    RUN_TEST(a_wound_that_is_not_fatal_says_so);
    RUN_TEST(a_wound_that_takes_exactly_everything_is_not_fatal);
    RUN_TEST(a_wound_leaves_the_fraction_alone);
    RUN_TEST(no_damage_at_all_changes_nothing);
    RUN_TEST(resurrection_rubs_out_the_mark);
    RUN_TEST(resurrecting_a_living_character_changes_nothing);

    RUN_TEST(healing_adds_and_says_so);
    RUN_TEST(healing_a_character_at_the_top_does_nothing);
    RUN_TEST(healing_stops_at_the_top_and_clears_the_fraction);
    RUN_TEST(healing_short_of_the_top_keeps_the_fraction);
    RUN_TEST(healing_does_not_special_case_the_mark_of_death);

    RUN_TEST(the_fraction_is_what_makes_a_slow_recovery_add_up);
    RUN_TEST(a_big_store_comes_back_by_whole_points);
    RUN_TEST(recovery_stops_at_the_top_and_clears_the_fraction);
    RUN_TEST(recovery_does_not_overshoot_the_top);
    RUN_TEST(recovery_pins_a_runaway_sum_at_the_top);

    RUN_TEST(a_new_maximum_carries_what_is_left_across_in_proportion);
    RUN_TEST(a_smaller_maximum_takes_what_is_left_down_with_it);
    RUN_TEST(a_maximum_of_zero_is_left_alone_entirely);
    RUN_TEST(the_same_maximum_changes_nothing);

    RUN_TEST(a_temporary_maximum_lifts_both_numbers_together);
    RUN_TEST(a_temporary_maximum_wearing_off_caps_what_is_left);
    RUN_TEST(a_temporary_maximum_wearing_off_need_not_cap_anything);

    RUN_TEST(a_brand_new_maximum_fills_to_the_top);

    RUN_TEST(a_whole_session_follows_the_callers_order);

    return TEST_SUMMARY();
}
