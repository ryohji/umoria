// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「持っている金」のテスト -- 現在のふるまいを保護する
 *
 * py の 86 個のフィールドのうち、いちばん独立した問い。22 か所が触るだけで、
 * どの 1 か所もほかのフィールドと一緒には読んでいない。だから py 全体を
 * 1 本の module にはせず、問いごとに出す —— これがその 1 つめ。
 *
 * 保護したい性質は 4 つ。
 *
 *   1. **走りだしの財布は空**であること（もとの py.misc.au は初期化子なしの
 *      構造体の中にあった、つまり 0）。金を持って始めるのではなく、
 *      create.c が最後に置いてくれる。
 *
 *   2. **増える・減る・置きかえるの 3 つしか動きがない**こと。22 か所の
 *      書きこみは「拾った・売れた（増）」「払った・盗られた（減）」
 *      「作った・wizard が入れた・save から読んだ・空にされた（置きかえ）」の
 *      どれか。読む 9 か所は数をそのまま使う。
 *
 *   3. **窓口は数を検めない**こと。払えるかを確かめるのは店（store_ui.c が
 *      払える額かを先に見る）、盗られる額を抑えるのは monster_melee.c、
 *      80 の下限は create.c、負を断るのは wizard.c。**どれも理由が違い、
 *      読み手が 1 つずつしかいない**ので、窓口の外に残す（所見 24・25）。
 *      ここで丸めるとふるまいが変わる。
 *
 *   4. **持てる額は short に収まらない**こと（置き場は int32_t）。
 *      店の高い品も「老いて死ぬ」wizard コマンドの 250000 も 16 bit を
 *      超える。
 *
 * 置き場は #18-12-1C で src/player/player_gold.c の static になり、初期値もそこに
 * 入った（それまで要っていた足場 tests/player_gold_fixture.c は消した）。
 * テストは 1 プロセスで状態を共有するので、走りだしを見る 1 件は main() の
 * 先頭に置き、以降は各件が最初に窓口で足場を作る。
 */
/* externs.h は要らない。窓口と、int32_t のための types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_gold.h"

#include "minunit.h"

/* --- 走りだし ------------------------------------------------------------- */
/* この 1 件は main() の先頭で走らせる。ほかのテストが状態を動かすので。 */

TEST(the_purse_starts_empty) {
    ASSERT_EQ_INT(0, player_gold());
}

/* --- 置きかえる ----------------------------------------------------------- */

/* create.c が作りおわりに置く額（最低 80）。wizard.c と save の読みも同じ形。 */
TEST(setting_replaces_what_was_there) {
    player_set_gold(80);
    ASSERT_EQ_INT(80, player_gold());

    player_set_gold(3000);
    ASSERT_EQ_INT(3000, player_gold());
}

/* 盗人が財布を空にする形（monster_melee.c は、持ち額より多く盗ったことになる
 * ときだけ 0 を置く）。 */
TEST(the_purse_can_be_emptied) {
    player_set_gold(120);
    player_set_gold(0);
    ASSERT_EQ_INT(0, player_gold());
}

/* --- 増える --------------------------------------------------------------- */

/* 床で拾う（player_move.c）・店が買ってくれる（store_ui.c）。 */
TEST(found_gold_is_added_to_the_purse) {
    player_set_gold(100);
    player_gain_gold(45);
    ASSERT_EQ_INT(145, player_gold());
}

TEST(gold_can_come_in_more_than_once) {
    player_set_gold(0);
    player_gain_gold(10);
    player_gain_gold(20);
    player_gain_gold(30);
    ASSERT_EQ_INT(60, player_gold());
}

/* --- 減る ----------------------------------------------------------------- */

/* 店で買う（store_ui.c）・盗られる（monster_melee.c）。 */
TEST(a_payment_is_taken_out_of_the_purse) {
    player_set_gold(500);
    player_pay_gold(175);
    ASSERT_EQ_INT(325, player_gold());
}

TEST(paying_everything_leaves_the_purse_empty) {
    player_set_gold(250);
    player_pay_gold(250);
    ASSERT_EQ_INT(0, player_gold());
}

/* **窓口は払えるかを確かめない。** 店が先に払える額かを見ているから
 * 実際には起きないが、確かめる側を窓口の中に移すのはふるまいの変更になる。
 * ここで丸めを足したら、この 1 件が赤くなって気づける。 */
TEST(paying_more_than_you_have_is_not_stopped_here) {
    player_set_gold(30);
    player_pay_gold(100);
    ASSERT_EQ_INT(-70, player_gold());
}

/* --- 持てる額 ------------------------------------------------------------- */

/* 16 bit には収まらない。上は store_ui.c の高い品、下は death.c の
 * 「老いて死ぬ」wizard コマンドが足す 250000。 */
TEST(the_purse_holds_more_than_a_short) {
    player_set_gold(1000000);
    ASSERT_EQ_INT(1000000, player_gold());

    player_gain_gold(250000L);
    ASSERT_EQ_INT(1250000, player_gold());
}

int main(void) {
    /* 走りだしの状態を見る 1 件を最初に。以降のテストが状態を動かす。 */
    RUN_TEST(the_purse_starts_empty);

    RUN_TEST(setting_replaces_what_was_there);
    RUN_TEST(the_purse_can_be_emptied);

    RUN_TEST(found_gold_is_added_to_the_purse);
    RUN_TEST(gold_can_come_in_more_than_once);

    RUN_TEST(a_payment_is_taken_out_of_the_purse);
    RUN_TEST(paying_everything_leaves_the_purse_empty);
    RUN_TEST(paying_more_than_you_have_is_not_stopped_here);

    RUN_TEST(the_purse_holds_more_than_a_short);

    return TEST_SUMMARY();
}
