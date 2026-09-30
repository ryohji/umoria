// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「足音はどれくらい静かか」のテスト -- 現在のふるまいを保護する
 *
 * py から出す 26 つめの問いで、**`struct misc` から出る 12 つめ**
 * （財布・どこまで来たか・どこまで潜ったか・体力の骰子・守りの点数・
 * 素の命中力・罠と鍵をはずす腕・抵抗・どの種族か・体の重さ・
 * 命中と打撃の下駄・どれくらい探すか・人物の身上書き 6 つにつづく）。
 * 答えは short 1 本 —— もとは py.misc.stl で、6 ファイルから 9 か所が
 * 名ざしていた。**残るフィールドは `pclass` 1 つだけ**（#18-12-28）。
 *
 * **1 フィールド 1 つの答えで、この道でいちばん素直な形**（19 つめ
 * 「罠と鍵をはずす腕」・20 つめ「抵抗」と同じ）。6 つの答えを一度に出した
 * 25 つめのあとに来る、いちばん小さな単位（**呼びは 9。前の単位は 44**）。
 *
 * **単位は「割る回数」で、割合ではない。** 使うのは遊びの中で 1 か所だけ:
 *
 *     notice = randint(1024);
 *     if (notice * notice * notice <= (1L << (29 - stl)))   // 寝ている者が動く
 *
 * つまり **1 点ごとに気づかれる見こみが半分になる** —— だから数が粗い
 * （罠と鍵の腕は 0〜200 の幅を持つが、こちらは -1〜18 の 20 段しかなく、
 * その 1 段が向こうの全幅ぶんの効き目を持つ）。**大きいほど静か。**
 *
 * **幅は -1〜18 で、wizard 画面の「(-1-18)」とぴったり同じ** ——
 * 種族 -2〜+4（Half-Troll 〜 Halfling）＋ 階級 +1〜+5（Warrior 〜 Rogue）で
 * 創成が -1〜9、装備は 3 か所 × 1〜3 点で +9（Defender の武器・静かさの靴・
 * 静かさの外套）。**ずらしの余裕もちょうど使いきっている**（下の 3 件）。
 *
 * 寝ている者が動くかの判定・画面の +1 と等級・装備の符号・-1〜18 の囲い・
 * TR_AGGRAVATE は **この module の外**（player_stealth.h）。
 *
 * **同じ幅の隣り（所見 46）はこの module の外にある。** セーブファイルは
 * `sc`（short）→ `stl`（short）→ `pclass`（byte）の順で、**手前の階層と
 * 同じ幅・隣りあわせ**なので、save.c で 2 本の窓口を入れちがえてもこの
 * ファイルの 1 件も落ちない（窓口が別の module にあるので、ここで並びを
 * 固定する 1 件は書けない）。**網は人物画面のほう** —— 階層は 1〜100 なので
 * 入れかわると Stealth が Superb に、Social Class が 1 桁になって目で見える。
 * B の起動確認でそこを読む。
 *
 * テストは 1 プロセスで状態を共有するので、MU_SETUP が毎件 0 に戻す。
 */
/* externs.h は要らない。窓口 3 本と、型のための types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_stealth.h"

/* 白紙は 0 —— 種族を選ぶ前の人物（`py` が全域 0 で始まるのと同じところ）。 */
#define MU_SETUP() player_stealth_set(0)
#include "minunit.h"

/* 寝ている者が動くかの右辺（creature.c:1576）。ここに写しがあるのは
 * **この module の外の規則**を 3 件で固定するためで、窓口の仕事ではない。 */
static long the_threshold_for(int stealth) { return 1L << (29 - stealth); }

/* ------------------------------------------------------------------
 * 数そのもの -- 種族が置き、階級が足す
 * ------------------------------------------------------------------ */

TEST(the_number_is_the_races_base) {
    player_stealth_set(4); /* Halfling */

    ASSERT_EQ_INT(player_stealth(), 4);
}

/* **いちばん静かな新入り**は Halfling の Rogue で 9（4 + 5）。 */
TEST(the_class_adds_to_what_the_race_left) {
    player_stealth_set(4);

    player_stealth_adjust(5); /* Rogue */

    ASSERT_EQ_INT(player_stealth(), 9);
}

/* **いちばん騒がしい新入り**は Half-Troll の Warrior で -1（-2 + 1）——
 * どの階級も何かしら足すので、創成はここより下に行けない。 */
TEST(the_loudest_new_character_is_minus_one) {
    player_stealth_set(-2); /* Half-Troll */

    player_stealth_adjust(1); /* Warrior */

    ASSERT_EQ_INT(player_stealth(), -1);
}

/* 0 は「まだ種族を選んでいない」でもあり、ありうる答えでもある ——
 * Human の土台が 0（ただし階級が必ず足すので、遊びの中で 0 の Human は
 * 呪われた装備でもなければ見ない）。 */
TEST(zero_is_both_the_starting_state_and_a_real_answer) {
    player_stealth_set(0); /* Human */

    ASSERT_EQ_INT(player_stealth(), 0);
}

TEST(setting_again_replaces_rather_than_adds) {
    player_stealth_set(4);

    player_stealth_set(1);

    ASSERT_EQ_INT(player_stealth(), 1);
}

/* ------------------------------------------------------------------
 * 装備 -- 身につけると足し、外すと戻る
 * ------------------------------------------------------------------ */

TEST(gear_adds_while_it_is_worn) {
    player_stealth_set(0);

    player_stealth_adjust(3); /* 静かさの外套（p1 = 3） */

    ASSERT_EQ_INT(player_stealth(), 3);
}

/* **符号は呼び手のもの**（player_bonuses.c は `t_ptr->p1 * factor` を渡し、
 * 外すときの factor は -1）。窓口は 1 本で両方を受けもつ。 */
TEST(taking_the_gear_off_gives_the_number_back) {
    player_stealth_set(4);
    player_stealth_adjust(3);

    player_stealth_adjust(-3);

    ASSERT_EQ_INT(player_stealth(), 4);
}

/* **幅の上端 18 は 3 か所ぜんぶに 3 点の装備を着けた Halfling の Rogue** ——
 * wizard 画面が訊く「(-1-18)」の 18 がこれ。 */
TEST(three_slots_of_gear_stack_on_top_of_creation) {
    player_stealth_set(4);
    player_stealth_adjust(5);

    player_stealth_adjust(3); /* 武器（Defender） */
    player_stealth_adjust(3); /* 靴 */
    player_stealth_adjust(3); /* 外套 */

    ASSERT_EQ_INT(player_stealth(), 18);
}

/* ------------------------------------------------------------------
 * 窓口は何も断らない（所見 24）
 * ------------------------------------------------------------------ */

/* -1〜18 は wizard 画面の規則で、窓口の規則ではない。 */
TEST(the_window_takes_a_number_above_the_prompts_range) {
    player_stealth_set(19);

    ASSERT_EQ_INT(player_stealth(), 19);
}

/* **下にも断らない。** -2 まで行くと右辺が `1L << 31` で符号つき long の
 * 未定義になるが、**遊びの中でそこへ行く道が無い**（2 つの表が -1 より下を
 * 作れず、静かさを下げる品物は 1 つも無い —— 騒がしくする呪いは
 * TR_AGGRAVATE という別の道）。囲うのは入り口の仕事。 */
TEST(the_window_takes_a_number_below_the_prompts_range) {
    player_stealth_set(-2);

    ASSERT_EQ_INT(player_stealth(), -2);
}

/* 置き場は 2 バイトの**符号つき**のまま（`py.misc.stl` は int16_t）。 */
TEST(the_number_is_two_bytes_wide_and_signed_the_way_the_field_was) {
    player_stealth_set(32768);

    ASSERT_EQ_INT(player_stealth(), -32768);
}

/* **負の静かさはセーブファイルの往復で生きのびる。** save.c は
 * `wr_short((uint16_t)stl)` で書き、`rd_short((uint16_t *)&stl)` で読む ——
 * B では読んだ符号なしを窓口に渡す形になるので、-1 が -1 で戻ることを
 * ここで固定する。 */
TEST(a_negative_stealth_survives_the_saved_files_unsigned_short) {
    uint16_t what_the_file_holds = (uint16_t)(-1);

    player_stealth_set((int16_t)what_the_file_holds);

    ASSERT_EQ_INT(player_stealth(), -1);
}

/* ------------------------------------------------------------------
 * 呼び手の規則 -- 窓口は数を覚えるだけ
 * ------------------------------------------------------------------ */

/* **画面の +1 は呼び手の規則**（abilities.c:64 の `a.stl = p_ptr->stl + 1`）。
 * likert() の除数は 1 —— あの画面でいちばん細かく、**1 点ごとに語が動く**。
 * 0 と 1 がどちらも "Bad" なので、+1 が見えはじめるのは 1 から。 */
TEST(the_plus_one_on_the_sheet_is_the_callers_rule) {
    player_stealth_set(1);

    int what_the_sheet_rates = player_stealth() + 1;

    ASSERT_EQ_INT(what_the_sheet_rates, 2); /* likert(2, 1) は "Poor" */
}

/* **1 点ごとに気づかれる見こみが半分になる** —— 右辺が半分になるということ。 */
TEST(every_point_halves_the_chance_of_being_noticed) {
    ASSERT_EQ_INT((int)(the_threshold_for(5) / the_threshold_for(6)), 2);
}

/* **下端ではずらしの余裕をちょうど使いきっている。** stl = -1 の右辺は
 * `1L << 30` = 1,073,741,824 で、**1024 の 3 乗とぴったり同じ** ——
 * つまり -1 の人物は必ず気づかれる（randint(1024) の最大でも等号で通る）。 */
TEST(at_the_bottom_of_the_range_the_sleeper_always_stirs) {
    long the_loudest_roll = 1024;

    long cube = the_loudest_roll * the_loudest_roll * the_loudest_roll;

    ASSERT_TRUE(cube == the_threshold_for(-1));
}

/* **上端では 1024 のうち 12。** stl = 18 の右辺は `1L << 11` = 2048 で、
 * 12 の 3 乗が 1728、13 の 3 乗が 2197 —— 12 までの振りだけが寝た者を
 * 動かす。**ここでも桁あふれは無い。** */
TEST(at_the_top_of_the_range_only_twelve_rolls_in_a_thousand_stir_the_sleeper) {
    ASSERT_EQ_INT((int)the_threshold_for(18), 2048);
    ASSERT_TRUE(12L * 12L * 12L <= the_threshold_for(18));
    ASSERT_FALSE(13L * 13L * 13L <= the_threshold_for(18));
}

int main(void) {
    RUN_TEST(the_number_is_the_races_base);
    RUN_TEST(the_class_adds_to_what_the_race_left);
    RUN_TEST(the_loudest_new_character_is_minus_one);
    RUN_TEST(zero_is_both_the_starting_state_and_a_real_answer);
    RUN_TEST(setting_again_replaces_rather_than_adds);

    RUN_TEST(gear_adds_while_it_is_worn);
    RUN_TEST(taking_the_gear_off_gives_the_number_back);
    RUN_TEST(three_slots_of_gear_stack_on_top_of_creation);

    RUN_TEST(the_window_takes_a_number_above_the_prompts_range);
    RUN_TEST(the_window_takes_a_number_below_the_prompts_range);
    RUN_TEST(the_number_is_two_bytes_wide_and_signed_the_way_the_field_was);
    RUN_TEST(a_negative_stealth_survives_the_saved_files_unsigned_short);

    RUN_TEST(the_plus_one_on_the_sheet_is_the_callers_rule);
    RUN_TEST(every_point_halves_the_chance_of_being_noticed);
    RUN_TEST(at_the_bottom_of_the_range_the_sleeper_always_stirs);
    RUN_TEST(at_the_top_of_the_range_only_twelve_rolls_in_a_thousand_stir_the_sleeper);

    return TEST_SUMMARY();
}
