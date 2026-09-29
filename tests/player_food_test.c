// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「腹の具合」のテスト -- 現在のふるまいを保護する
 *
 * py から出す 2 つめの問い（1 つめは持っている金）。フィールドは 2 つあるが
 * 1 つの問いに答えている —— food は腹の具合の数で、food_digested は 1 ターンで
 * そのうちいくら減るかの速さ。速さを読む唯一の場所が、数から速さを引いている。
 *
 * 保護したい性質は 5 つ。
 *
 *   1. **走りだしの腹は 0** であること（もとの py.flags.food は初期化子なしの
 *      構造体の中にあった）。本物の 7500 と速さ 2 は、人物を作りおわった
 *      main.c が置く。
 *
 *   2. **1 ターンの消化は「速さのぶん引く」だけ**であること。dungeon.c の
 *      毎ターンの処理はこれを 1 行でやっていて、その前後で「速さが負なら
 *      余分に燃やす」「0 を割ったら飢えの傷」が続くが、**どちらも窓口の外**。
 *
 *   3. **食べはじめは飢えの借金を消す**こと（もとの add_food() の先頭。
 *      これは腹だけの規則で読み手が 1 つなので窓口の内側に入れた。
 *      所見 24・25）。-50 の腹に 100 食べたら 100 で、50 ではない。
 *
 *   4. **窓口は上も下も見ない**こと。満腹（PLAYER_FOOD_FULL）と食べすぎ
 *      （PLAYER_FOOD_MAX と速さの罰）の規則は misc1.c、空腹の境目と
 *      飢えの傷は dungeon.c、吐いたときの 150 は potions.c、蘇生のときの 0 は
 *      save.c。**どれも理由が腹の外にあり読み手が 1 つずつ**なので外に残す。
 *      ここで丸めるとふるまいが変わる。
 *
 *   5. **速さは行ったり戻ったりする**こと。calc_bonuses() は指輪のぶんを
 *      先に戻してから数えなおし、探索と休憩も入る・出るで ±1 する。
 *      **足した順と引いた順が違っても元に戻る**（呼び手が対で呼ぶから）。
 *
 * 置き場は #18-12-2C で src/player/player_food.c の static になり、初期値もそこに
 * 入った（それまで要っていた足場 tests/player_food_fixture.c は消した）。
 * テストは 1 プロセスで状態を共有するので、走りだしを見る 2 件は main() の
 * 先頭に置き、以降は各件が最初に窓口で足場を作る。
 */
/* externs.h は要らない。窓口と、int16_t のための types.h、
 * それに腹の境目の数（PLAYER_FOOD_*）のための constant.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_food.h"

#include "minunit.h"

/* --- 走りだし ------------------------------------------------------------- */
/* この 2 件は main() の先頭で走らせる。ほかのテストが状態を動かすので。 */

TEST(the_stomach_starts_empty) {
    ASSERT_EQ_INT(0, player_food());
}

TEST(digestion_starts_at_nothing) {
    ASSERT_EQ_INT(0, player_digestion());
}

/* --- 置きかえる ----------------------------------------------------------- */

/* main.c が人物を作りおわってから置く 7500。save の読みも同じ形。 */
TEST(setting_replaces_what_was_there) {
    player_set_food(7500);
    ASSERT_EQ_INT(7500, player_food());

    player_set_food(150);
    ASSERT_EQ_INT(150, player_food());
}

/* --- 食べる --------------------------------------------------------------- */

TEST(eating_adds_to_the_stomach) {
    player_set_food(1000);
    player_gain_food(500);
    ASSERT_EQ_INT(1500, player_food());
}

/* **飢えの借金は最初のひと口で消える。** もとの add_food() は足す前に
 * 負の腹を 0 に戻していた。これが無いと、飢えて -2000 まで落ちた人物は
 * その 2000 を食べ戻すまで腹の数が意味を持たない。 */
TEST(the_first_bite_forgives_a_starvation_debt) {
    player_set_food(-50);
    player_gain_food(100);
    ASSERT_EQ_INT(100, player_food());
}

/* 借金が無いときは何も起きない（0 は負ではない）。 */
TEST(an_empty_stomach_is_not_a_debt) {
    player_set_food(0);
    player_gain_food(100);
    ASSERT_EQ_INT(100, player_food());
}

/* **窓口は満腹も食べすぎも見ない。** PLAYER_FOOD_FULL を超えたら「満腹」と
 * 言い、PLAYER_FOOD_MAX を超えたら速さの罰を与えるのは misc1.c の add_food()。
 * ここで丸めを足したら、この 1 件が赤くなって気づける。 */
TEST(overeating_is_not_stopped_here) {
    player_set_food(PLAYER_FOOD_MAX);
    player_gain_food(1000);
    ASSERT_EQ_INT(PLAYER_FOOD_MAX + 1000, player_food());
}

/* --- 燃やす --------------------------------------------------------------- */

/* 速さが負の人物は 1 ターンに余分に燃やす（dungeon.c）。 */
TEST(burning_takes_food_out_of_the_stomach) {
    player_set_food(1000);
    player_burn_food(9);
    ASSERT_EQ_INT(991, player_food());
}

/* **0 を割っても窓口は止めない。** 飢えの傷（-food / 16）を与えるのは
 * dungeon.c で、負の数そのものがその傷の大きさになっている。 */
TEST(the_stomach_can_go_below_empty) {
    player_set_food(10);
    player_burn_food(90);
    ASSERT_EQ_INT(-80, player_food());
}

/* --- 消化の速さ ----------------------------------------------------------- */

TEST(setting_the_rate_replaces_what_was_there) {
    player_set_digestion(2);
    ASSERT_EQ_INT(2, player_digestion());
}

/* 指輪（遅い消化で +1・再生で -3）・探索（±1）・休憩（±1）。
 * **足した順と引いた順が違っても元に戻る。** */
TEST(the_rate_moves_up_and_down_and_comes_back) {
    player_set_digestion(2);

    player_adjust_digestion(1);  /* 遅い消化の指輪 */
    player_adjust_digestion(-3); /* 再生の指輪 */
    ASSERT_EQ_INT(0, player_digestion());

    player_adjust_digestion(1);  /* 戻す順は逆 */
    player_adjust_digestion(-1);
    player_adjust_digestion(3);
    player_adjust_digestion(-1);
    ASSERT_EQ_INT(2, player_digestion());
}

/* 速さは負にもなる（再生の指輪 1 つで -1。もとの計算もそうなる）。 */
TEST(the_rate_can_go_negative) {
    player_set_digestion(2);
    player_adjust_digestion(-3);
    ASSERT_EQ_INT(-1, player_digestion());
}

/* --- 1 ターン分の消化 ----------------------------------------------------- */

/* dungeon.c の毎ターン 1 行ぶん。**速さを読むのはここだけ。** */
TEST(a_turn_of_digestion_takes_the_rate_off_the_stomach) {
    player_set_food(1000);
    player_set_digestion(2);

    player_digest();
    ASSERT_EQ_INT(998, player_food());

    player_digest();
    ASSERT_EQ_INT(996, player_food());
}

/* 速さ 0 なら腹は動かない。 */
TEST(digestion_without_a_rate_leaves_the_stomach_alone) {
    player_set_food(500);
    player_set_digestion(0);
    player_digest();
    ASSERT_EQ_INT(500, player_food());
}

/* 速さが負なら消化で腹が増える（再生の指輪を着けた人物。もとの
 * food -= food_digested がそうなっていた。**窓口が向きを変えていない**
 * ことの保護）。 */
TEST(a_negative_rate_fills_the_stomach) {
    player_set_food(500);
    player_set_digestion(-1);
    player_digest();
    ASSERT_EQ_INT(501, player_food());
}

int main(void) {
    /* 走りだしの状態を見る 2 件を最初に。以降のテストが状態を動かす。 */
    RUN_TEST(the_stomach_starts_empty);
    RUN_TEST(digestion_starts_at_nothing);

    RUN_TEST(setting_replaces_what_was_there);

    RUN_TEST(eating_adds_to_the_stomach);
    RUN_TEST(the_first_bite_forgives_a_starvation_debt);
    RUN_TEST(an_empty_stomach_is_not_a_debt);
    RUN_TEST(overeating_is_not_stopped_here);

    RUN_TEST(burning_takes_food_out_of_the_stomach);
    RUN_TEST(the_stomach_can_go_below_empty);

    RUN_TEST(setting_the_rate_replaces_what_was_there);
    RUN_TEST(the_rate_moves_up_and_down_and_comes_back);
    RUN_TEST(the_rate_can_go_negative);

    RUN_TEST(a_turn_of_digestion_takes_the_rate_off_the_stomach);
    RUN_TEST(digestion_without_a_rate_leaves_the_stomach_alone);
    RUN_TEST(a_negative_rate_fills_the_stomach);

    return TEST_SUMMARY();
}
