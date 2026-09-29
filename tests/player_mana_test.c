// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「魔力」のテスト -- 現在のふるまいを保護する
 *
 * py から出す 4 つめの問い（1 つめは持っている金、2 つめは腹の具合、
 * 3 つめは画面に出す数字）。フィールドは 3 つあるが 1 つの問いに答えている ——
 * もとは py.misc.cmana（残り）・py.misc.mana（上限）・py.misc.cmana_frac
 * （まだ 1 点に届いていない端数）。
 *
 * **端数がこの単位の主役。** 再生は 1 ターンにごくわずかしか戻さない ——
 * 上限 × 数百 / 65536（PLAYER_REGEN_NORMAL は 197）＋ 底上げ
 * PLAYER_REGEN_MNBASE（524）。上限 10 の人物なら 1 ターンあたり 0.04 点ほどで、
 * **端数を置く場所が無ければ永久に 1 点も戻らない。** だから端数は
 * 「持ちこし」で、溜まって 65536 に届いたときに 1 点になる。
 *
 * 保護したい性質は 6 つ。
 *
 *   1. **走りだしは 3 つとも 0**（もとの 3 つは初期化子なしの構造体の中に
 *      あった）。本物の値は misc3.c の calc_mana() が最初の呪文を覚えたときに、
 *      あるいは save.c がファイルから読んで置く。
 *
 *   2. **持ちこしは足し合わさって 1 点になる**こと。上限 10・速さ 197 なら
 *      26 ターンでは 1 点も戻らず、**27 ターン目に 1 点**になって端数が余る。
 *      ここが端数の存在理由そのもの。
 *
 *   3. **満杯で止まり、そのとき端数は消える**こと（もとのコードの
 *      「must set frac to zero even if equal」）。満杯の人に「途中」は無い。
 *
 *   4. **使うときは端数を触らない。ただし払いきれなかったときだけ消す。**
 *      ふつうの詠唱（`cmana -= smana`）は端数をそのまま残すので、次の 1 点への
 *      道のりは使っても失われない。払いきれないときは空にして、
 *      **窓口は「実際に使えた量」を返す**（呼び手はそこから足りなかった量を
 *      知って麻痺の長さを決める）。3 か所が同じ 6 行を書いていた
 *      （magic.c・prayer.c・creature.c）。
 *
 *   5. **上限が動いたら残りは比例で連れていく**こと（端数込み）。もとの
 *      コードは「先に割って溢れを避ける。精度は少し落ちる」と言っていて、
 *      その落ちかたまで写しとる。**上限が 0 だった人物は満杯から始まる。**
 *
 *   6. **呪文を 1 つも覚えていない身に戻ると上限も残りも 0。ただし端数は
 *      触らない**（もとのコードがそうだった）。上限が 0 のあいだ再生は
 *      呼ばれないので誰も読まず、次に上限が付くときは上の 5 で消える。
 *
 * 置き場は #18-12-4C で src/player/player_mana.c の static になり、初期値もそこに
 * 入る（それまで要る足場 tests/player_mana_fixture.c は C で消す。static に
 * したあとも同じ名前の器を足場に残すと、窓口に届かない別の器が生き残って
 * テストが何も検証しなくなる —— HANDOVER 第 7 節）。テストは 1 プロセスで
 * 状態を共有するので、走りだしを見る 1 件は main() の先頭に置き、以降は
 * 各件が最初に窓口で足場を作る。
 */
/* externs.h は要らない。窓口と、int16_t・MAX_SHORT・PLAYER_REGEN_* のための
 * types.h と constant.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_mana.h"

#include "minunit.h"

/* 上限 10 の人物が満腹で休んでいないときの 1 ターンぶん（65536 分の）。
 * dungeon.c が渡すのは PLAYER_REGEN_NORMAL で、そこから
 * 10 * 197 + 524 = 2494 が出る。65536 / 2494 = 26.3 ターンで 1 点。 */
#define GAIN_PER_TURN (10 * PLAYER_REGEN_NORMAL + PLAYER_REGEN_MNBASE)

/* 3 つをまとめて置く（各テストの足場作り）。 */
static void given(int16_t max, int16_t current, uint16_t fraction) {
    player_set_max_mana(max);
    player_set_mana(current);
    player_set_mana_fraction(fraction);
}

/* --- 走りだし ------------------------------------------------------------- */
/* この 1 件は main() の先頭で走らせる。ほかのテストが状態を動かすので。 */

TEST(the_store_starts_at_nothing) {
    ASSERT_EQ_INT(0, player_mana());
    ASSERT_EQ_INT(0, player_max_mana());
    ASSERT_EQ_INT(0, player_mana_fraction());
}

/* --- 3 つは互いに独立 ----------------------------------------------------- */

/* 1 つの窓口が触るのは 1 つの数だけ。**入れちがえの裏取り** ——
 * save の読みは 3 つを別々に置くので、窓口の中で混ざっていると分からない。 */
TEST(each_number_stands_on_its_own) {
    given(20, 7, 1234);

    ASSERT_EQ_INT(7, player_mana());
    ASSERT_EQ_INT(20, player_max_mana());
    ASSERT_EQ_INT(1234, player_mana_fraction());

    player_set_mana(8);
    ASSERT_EQ_INT(8, player_mana());
    ASSERT_EQ_INT(20, player_max_mana());
    ASSERT_EQ_INT(1234, player_mana_fraction());

    player_set_max_mana(21);
    ASSERT_EQ_INT(8, player_mana());
    ASSERT_EQ_INT(21, player_max_mana());
    ASSERT_EQ_INT(1234, player_mana_fraction());

    player_set_mana_fraction(4321);
    ASSERT_EQ_INT(8, player_mana());
    ASSERT_EQ_INT(21, player_max_mana());
    ASSERT_EQ_INT(4321, player_mana_fraction());
}

/* --- 使う ----------------------------------------------------------------- */

/* ふつうの詠唱。**端数はそのまま残る** —— 次の 1 点への道のりは使っても
 * 失われない。 */
TEST(spending_takes_the_cost_and_leaves_the_fraction) {
    given(10, 10, 5000);

    int spent = player_spend_mana(3);

    ASSERT_EQ_INT(3, spent);
    ASSERT_EQ_INT(7, player_mana());
    ASSERT_EQ_INT(5000, player_mana_fraction());
    ASSERT_EQ_INT(10, player_max_mana());
}

/* 払いきれないとき。**空にして端数も消し、実際に使えた量を返す。**
 * 呼び手（magic.c・prayer.c）はここから足りなかった量を出して麻痺の長さを
 * 決めるので、返す値が要る。 */
TEST(spending_more_than_there_is_empties_everything) {
    given(10, 4, 5000);

    int spent = player_spend_mana(9);

    ASSERT_EQ_INT(4, spent);
    ASSERT_EQ_INT(0, player_mana());
    ASSERT_EQ_INT(0, player_mana_fraction());
    /* 上限は動かない */
    ASSERT_EQ_INT(10, player_max_mana());
}

/* **ちょうど使いきるのは「払いきれた」側。** もとの条件は
 * `smana > cmana` なので、等しいときは引き算のほうへ行き、端数が残る。
 * 境目の字面を守る 1 件。 */
TEST(spending_exactly_what_is_left_is_still_paying_in_full) {
    given(10, 4, 5000);

    int spent = player_spend_mana(4);

    ASSERT_EQ_INT(4, spent);
    ASSERT_EQ_INT(0, player_mana());
    ASSERT_EQ_INT(5000, player_mana_fraction());
}

/* 空のときに使おうとしても負にならない。 */
TEST(spending_from_an_empty_store_gets_nothing) {
    given(10, 0, 0);

    ASSERT_EQ_INT(0, player_spend_mana(5));
    ASSERT_EQ_INT(0, player_mana());
}

/* --- 戻る（端数の存在理由） ----------------------------------------------- */

/* **26 ターンでは 1 点も戻らず、27 ターン目に 1 点になる。**
 * 2494 * 26 = 64844（65536 に届かない）、2494 * 27 = 67338 で 1 点と
 * 余り 1802。**端数を捨てる実装ならここで 0 点のまま止まる。** */
TEST(the_fraction_is_what_makes_a_slow_recovery_add_up) {
    given(10, 0, 0);

    for (int turn = 0; turn < 26; turn++) {
        player_regenerate_mana(PLAYER_REGEN_NORMAL);
    }

    ASSERT_EQ_INT(0, player_mana());
    ASSERT_EQ_INT(GAIN_PER_TURN * 26, player_mana_fraction());

    player_regenerate_mana(PLAYER_REGEN_NORMAL);

    ASSERT_EQ_INT(1, player_mana());
    ASSERT_EQ_INT(GAIN_PER_TURN * 27 - 0x10000L, player_mana_fraction());
}

/* 1 ターンで整数部が動く速さもある（休んでいる人物の上限が大きいとき）。
 * 上限 100・速さ 591（休み＋再生の指輪で 197 * 3 / 2 * 2）なら
 * 100 * 591 + 524 = 59624 で、まだ 1 点に届かない。上限 1000 なら
 * 591524 で 9 点と余り。 */
TEST(a_big_store_comes_back_by_whole_points) {
    given(1000, 0, 0);

    player_regenerate_mana(591);

    ASSERT_EQ_INT(9, player_mana());
    ASSERT_EQ_INT(1000 * 591 + PLAYER_REGEN_MNBASE - 9 * 0x10000L, player_mana_fraction());
}

/* **満杯で止まり、端数が消える。** もとのコードの
 * 「must set frac to zero even if equal」。呼び手（dungeon.c）は
 * `cmana < mana` のときだけ呼ぶが、窓口自身も止める。 */
TEST(recovery_stops_at_the_top_and_clears_the_fraction) {
    given(10, 10, 500);

    player_regenerate_mana(PLAYER_REGEN_NORMAL);

    ASSERT_EQ_INT(10, player_mana());
    ASSERT_EQ_INT(0, player_mana_fraction());
}

/* 満杯の手前から 1 ターンで飛びこしても、止まるのは満杯。 */
TEST(recovery_does_not_overshoot_the_top) {
    given(10, 9, 0);

    player_regenerate_mana(65536 * 5); /* 1 ターンで 5 点ぶん */

    ASSERT_EQ_INT(10, player_mana());
    ASSERT_EQ_INT(0, player_mana_fraction());
}

/* **溢れの見はり。** 足した結果が short の天井を越えて負に回ったら、
 * 天井（MAX_SHORT）に留める。**この道は実際のゲームでは通らない**
 * （速さは 591 が上限で、上限は int16_t なので積が 32767 << 16 に届かない）が、
 * もとのコードが持っていた歯なので残す。見はりが無ければ残りは
 * 負のまま（-32765）になり、満杯の判定もすり抜ける。
 * 注: int16_t に入らない値を代入したときの回りこみは処理系まかせで、
 * もとの `p_ptr->cmana += new_mana >> 16;` も同じ形だった。 */
TEST(recovery_pins_a_runaway_sum_at_the_top) {
    given(MAX_SHORT, 5, 0);

    player_regenerate_mana(65535);

    ASSERT_EQ_INT(MAX_SHORT, player_mana());
    ASSERT_EQ_INT(0, player_mana_fraction());
}

/* --- 満杯にする（魔力回復の薬） ------------------------------------------- */

TEST(restoring_fills_the_store_and_says_so) {
    given(10, 4, 777);

    ASSERT_TRUE(player_restore_mana());
    ASSERT_EQ_INT(10, player_mana());
    /* **端数は触らない** —— 薬は端数について何も言わない（もとのコードも
     * `cmana = mana;` の 1 行だけだった）。 */
    ASSERT_EQ_INT(777, player_mana_fraction());
}

/* すでに満杯なら何も起きず、**呼び手は薬を鑑定しない**。 */
TEST(restoring_a_full_store_changes_nothing) {
    given(10, 10, 777);

    ASSERT_FALSE(player_restore_mana());
    ASSERT_EQ_INT(10, player_mana());
    ASSERT_EQ_INT(777, player_mana_fraction());
}

/* --- 上限が動く（レベルが上がる・呪文を覚える） --------------------------- */

/* **残りは比例で連れていく。** 上限 10 で 5.5 点（端数 0x8000）持っている
 * 人物の上限が 20 になると、55 % のまま 11 点 —— ではなく **10 点と
 * 端数 65520**。もとのコードが「先に割って溢れを避ける。精度は少し落ちる」と
 * 言っているとおりで、その落ちかたまで写しとる。 */
TEST(a_new_maximum_carries_what_is_left_across_in_proportion) {
    given(10, 5, 0x8000);

    ASSERT_TRUE(player_change_max_mana(20));

    ASSERT_EQ_INT(20, player_max_mana());
    ASSERT_EQ_INT(10, player_mana());
    ASSERT_EQ_INT(65520, player_mana_fraction());
}

/* 上限が下がるときも同じ道（呪文を忘れた・能力値が落ちた）。
 * 上限 20 で 10 点・端数 0 なら、上限 10 で 5 点。 */
TEST(a_smaller_maximum_takes_what_is_left_down_with_it) {
    given(20, 10, 0);

    ASSERT_TRUE(player_change_max_mana(10));

    ASSERT_EQ_INT(10, player_max_mana());
    ASSERT_EQ_INT(5, player_mana());
    ASSERT_EQ_INT(0, player_mana_fraction());
}

/* **上限が 0 だった人物は満杯から始まる**（最初の呪文を覚えた瞬間。
 * 比例で連れていくと 0 で割ることになるので、こちらの道が要る）。
 * 端数は消える。 */
TEST(a_character_who_had_no_mana_starts_full) {
    given(0, 0, 1234);

    ASSERT_TRUE(player_change_max_mana(8));

    ASSERT_EQ_INT(8, player_max_mana());
    ASSERT_EQ_INT(8, player_mana());
    ASSERT_EQ_INT(0, player_mana_fraction());
}

/* 同じ上限なら何も起きない（**呼び手は PY_MANA を立てない**）。
 * calc_mana() は毎レベル・毎装備で呼ばれるので、ここが動くと画面が
 * 無駄に書きかわる。 */
TEST(the_same_maximum_changes_nothing) {
    given(10, 3, 1234);

    ASSERT_FALSE(player_change_max_mana(10));

    ASSERT_EQ_INT(10, player_max_mana());
    ASSERT_EQ_INT(3, player_mana());
    ASSERT_EQ_INT(1234, player_mana_fraction());
}

/* --- 魔力を持たない身に戻る ----------------------------------------------- */

/* 呪文を 1 つも覚えていない状態（呪文書を失った・階級が変わった）。
 * **上限も残りも 0 になるが、端数は触らない** —— もとのコードがそうだった。 */
TEST(losing_every_spell_leaves_no_mana_at_all) {
    given(8, 3, 1234);

    ASSERT_TRUE(player_lose_all_mana());

    ASSERT_EQ_INT(0, player_max_mana());
    ASSERT_EQ_INT(0, player_mana());
    ASSERT_EQ_INT(1234, player_mana_fraction());
}

/* もともと 0 なら何も起きない（**呼び手は PY_MANA を立てない**）。 */
TEST(losing_it_twice_says_nothing_changed) {
    given(8, 3, 0);

    ASSERT_TRUE(player_lose_all_mana());
    ASSERT_FALSE(player_lose_all_mana());

    ASSERT_EQ_INT(0, player_max_mana());
    ASSERT_EQ_INT(0, player_mana());
}

/* --- 通しで 1 回、呼び手の並びどおりに ------------------------------------ */

/* **呼び手の並びを写しとる**（HANDOVER 第 4 節「テスト側に写す」）。
 * 魔法使いが 1 人ぶんの時間を過ごす ——
 *
 *   最初の呪文を覚える（上限 0 → 2）→ 詠唱で 1 使う → 何ターンか休む →
 *   モンスターに吸われる → 払えない詠唱をする → 薬で満杯 →
 *   レベルが上がって上限 4
 */
TEST(a_whole_session_follows_the_callers_order) {
    given(0, 0, 0);

    /* misc3.c calc_mana(): 最初の呪文。上限 2（1 レベルの人物が 2 になる +1 込み） */
    ASSERT_TRUE(player_change_max_mana(2));
    ASSERT_EQ_INT(2, player_mana());

    /* magic.c: 値段 1 の呪文。払える */
    ASSERT_EQ_INT(1, player_spend_mana(1));
    ASSERT_EQ_INT(1, player_mana());

    /* dungeon.c: 上限 2 なので 1 ターン 2 * 197 + 524 = 918。
     * 満杯まで 65536 / 918 = 71.4 ターン。70 ターンでは戻らない */
    for (int turn = 0; turn < 70; turn++) {
        player_regenerate_mana(PLAYER_REGEN_NORMAL);
    }
    ASSERT_EQ_INT(1, player_mana());
    ASSERT_EQ_INT((2 * PLAYER_REGEN_NORMAL + PLAYER_REGEN_MNBASE) * 70, player_mana_fraction());

    /* creature.c: 吸いとり。持っているのは 1 点だけなので 1 しか取れない */
    ASSERT_EQ_INT(1, player_spend_mana(3));
    ASSERT_EQ_INT(0, player_mana());
    ASSERT_EQ_INT(0, player_mana_fraction()); /* 払いきれなかったので端数も消えた */

    /* prayer.c: 値段 2 の祈り。1 点も無いので 0 しか使えず、
     * 呼び手は 5 * (2 - 0) ターンの麻痺を決める */
    ASSERT_EQ_INT(0, player_spend_mana(2));

    /* potions.c: 魔力回復の薬 */
    ASSERT_TRUE(player_restore_mana());
    ASSERT_EQ_INT(2, player_mana());

    /* misc3.c calc_mana(): レベルが上がって上限 4。満杯のまま連れていく */
    ASSERT_TRUE(player_change_max_mana(4));
    ASSERT_EQ_INT(4, player_max_mana());
    ASSERT_EQ_INT(4, player_mana());
}

int main(void) {
    /* 走りだしの状態を見る 1 件を最初に。以降のテストが状態を動かす。 */
    RUN_TEST(the_store_starts_at_nothing);

    RUN_TEST(each_number_stands_on_its_own);

    RUN_TEST(spending_takes_the_cost_and_leaves_the_fraction);
    RUN_TEST(spending_more_than_there_is_empties_everything);
    RUN_TEST(spending_exactly_what_is_left_is_still_paying_in_full);
    RUN_TEST(spending_from_an_empty_store_gets_nothing);

    RUN_TEST(the_fraction_is_what_makes_a_slow_recovery_add_up);
    RUN_TEST(a_big_store_comes_back_by_whole_points);
    RUN_TEST(recovery_stops_at_the_top_and_clears_the_fraction);
    RUN_TEST(recovery_does_not_overshoot_the_top);
    RUN_TEST(recovery_pins_a_runaway_sum_at_the_top);

    RUN_TEST(restoring_fills_the_store_and_says_so);
    RUN_TEST(restoring_a_full_store_changes_nothing);

    RUN_TEST(a_new_maximum_carries_what_is_left_across_in_proportion);
    RUN_TEST(a_smaller_maximum_takes_what_is_left_down_with_it);
    RUN_TEST(a_character_who_had_no_mana_starts_full);
    RUN_TEST(the_same_maximum_changes_nothing);

    RUN_TEST(losing_every_spell_leaves_no_mana_at_all);
    RUN_TEST(losing_it_twice_says_nothing_changed);

    RUN_TEST(a_whole_session_follows_the_callers_order);

    return TEST_SUMMARY();
}
