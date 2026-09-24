/* 明かりについて覚えている 2 つの答えの置き場のテスト -- 現在のふるまいを保護する
 *
 * 2 つとは「明かりが燃えているか」（player_has_light）と「その輪がいま地図に
 * 描かれているか」（player_light_is_drawn。#18-11-1 で足した）。**別の問い**で、
 * 読む場所（move_light、moria1.c）が同じなので 1 本の module に同居している。
 * 食いちがう場合が 2 つあることは第 3 節で固定する。
 *
 * ここにも計算は無い。player_light は旗 1 本で、保護するのは「置き場としての
 * ふるまい」に尽きる。それでも固定しておきたい性質が 3 つある。
 *
 *   1. **覚えた答えをそのまま返す**こと。dungeon.c:112 は「前の turn は
 *      明かりがあったか」と「いま燃料が残っているか」を比べて、変わり目だけで
 *      「Your light has gone out!」を出し、モンスターを消し／出しなおす。
 *      窓口が勝手に導出したり、読んだときに落としたりすると、変わり目が
 *      検出できなくなる（それは保護でなく書きかえになる）。
 *
 *   2. 明かりの有無を**装備の欄から導出しない**こと。値の出どころは
 *      equipment_at(INVEN_LIGHT)->p1 だが、そこを読むのは dungeon.c だけで、
 *      この module は装備を 1 度も見ない（player_light.h を include しても
 *      inventory.h / equipment.h は付いてこない）。だからこのテストの
 *      リンクにも装備は出てこない。
 *
 *   3. **同じ値を 2 度入れても何も起きない**こと。旗の置き場に「変わり目を
 *      数える」ような仕掛けは無い。変わり目の判断は呼び手（dungeon.c）の側に
 *      あり、こちらには持ちこまない。
 *
 * 走りだしは「明かりなし」。変更前の bool player_light;（variable.c:99）は
 * 初期値を書いていないので false から始まり、dungeon() が最初に装備から
 * 入れなおす（dungeon.c:55）。その順番までは置き場では見られないので、
 * ここでは「false から始まる」ことだけを固定する。
 *
 * テストは 1 プロセスで状態を共有する。走りだしの状態を見る 1 件は
 * main() の先頭に置いてある。
 */
/* externs.h は要らない。窓口と、型のための types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_light.h"

#include "minunit.h"

/* --- 走りだしの状態 ----------------------------------------------------- */
/* この 1 件は main() の先頭で走らせる。ほかのテストが書きこむので。 */

TEST(the_character_starts_out_without_a_light) {
    ASSERT_TRUE(!player_has_light());
}

/* --- 置き場としてのふるまい --------------------------------------------- */

TEST(a_burning_light_is_remembered) {
    set_player_has_light(true);
    ASSERT_TRUE(player_has_light());
}

TEST(a_light_that_has_gone_out_is_remembered) {
    set_player_has_light(true);
    set_player_has_light(false);
    ASSERT_TRUE(!player_has_light());
}

/* 明かりは戻ってくる（新しいランプに火を入れる、松明を拾う）。一度暗く
 * なったら二度と明るくならない、という覚えかたではない。 */
TEST(the_light_can_come_back_after_going_out) {
    set_player_has_light(false);
    set_player_has_light(true);
    ASSERT_TRUE(player_has_light());
}

/* 読んでも消えない。dungeon.c は 1 turn のうちに読んでから書くので、
 * 「読んだら落ちる」置き場だと変わり目が毎 turn 起きてしまう。 */
TEST(asking_twice_gives_the_same_answer) {
    set_player_has_light(true);
    (void)player_has_light();
    ASSERT_TRUE(player_has_light());
}

/* --- 変わり目を数えないこと --------------------------------------------- */

/* 同じ値を 2 度。変わり目の判断は呼び手の側にあるので、置き場は 2 度目を
 * 特別扱いしない。 */
TEST(setting_the_light_twice_leaves_it_lit) {
    set_player_has_light(true);
    set_player_has_light(true);
    ASSERT_TRUE(player_has_light());
}

TEST(setting_the_dark_twice_leaves_it_dark) {
    set_player_has_light(false);
    set_player_has_light(false);
    ASSERT_TRUE(!player_has_light());
}

/* --- 明かりの輪が地図に描かれているか（#18-11-1）----------------------- */

/* 走りだしは「描かれていない」。変更前の light_flag（variable.c にあった）は
 * = false と明示されていたので、その初期値をそのまま持ってきている。
 * この 1 件も main() の先頭。 */
TEST(the_glow_starts_out_undrawn) {
    ASSERT_TRUE(!player_light_is_drawn());
}

TEST(a_drawn_glow_is_remembered) {
    set_player_light_drawn(true);
    ASSERT_TRUE(player_light_is_drawn());
}

TEST(an_erased_glow_is_remembered) {
    set_player_light_drawn(true);
    set_player_light_drawn(false);
    ASSERT_TRUE(!player_light_is_drawn());
}

/* 輪は描きなおされる（歩くたびに前の位置から消して新しい位置に置く）。 */
TEST(the_glow_can_be_drawn_again_after_being_erased) {
    set_player_light_drawn(false);
    set_player_light_drawn(true);
    ASSERT_TRUE(player_light_is_drawn());
}

/* 読んでも消えない。sub1_move_light() は 1 度の呼びだしの中で 2 度読む
 * （前の位置を消すときと、新しい位置に置くとき）。 */
TEST(asking_twice_gives_the_same_glow) {
    set_player_light_drawn(true);
    (void)player_light_is_drawn();
    ASSERT_TRUE(player_light_is_drawn());
}

/* --- 2 つの答えは別物 --------------------------------------------------- */

/* **ここがこの節の要**。2 つの窓口が同じ置き場を指していたら（書きまちがい
 * でも、あとの整理でうっかり片方を消しても）、以下の 2 件が落ちる。 */

/* 明かりは燃えているのに輪は描かれていない —— 走っているとき
 * （find_prself が偽なら、輪が画面を横切ってちらつかないように置かない）。 */
TEST(a_burning_light_can_leave_no_glow_on_the_map) {
    set_player_has_light(true);
    set_player_light_drawn(false);
    ASSERT_TRUE(player_has_light());
}

/* 逆向き。明かりが消えた（または目が見えなくなった）直後は、前の turn に
 * 置いた輪がまだ地図にあり、それを消すのは sub3_move_light() の仕事。 */
TEST(a_glow_can_outlive_the_light_that_made_it) {
    set_player_light_drawn(true);
    set_player_has_light(false);
    ASSERT_TRUE(player_light_is_drawn());
}

int main(void) {
    /* 走りだしの状態を見る 2 件を最初に。以降のテストが書きこむ。 */
    RUN_TEST(the_character_starts_out_without_a_light);
    RUN_TEST(the_glow_starts_out_undrawn);

    RUN_TEST(a_burning_light_is_remembered);
    RUN_TEST(a_light_that_has_gone_out_is_remembered);
    RUN_TEST(the_light_can_come_back_after_going_out);
    RUN_TEST(asking_twice_gives_the_same_answer);

    RUN_TEST(setting_the_light_twice_leaves_it_lit);
    RUN_TEST(setting_the_dark_twice_leaves_it_dark);

    RUN_TEST(a_drawn_glow_is_remembered);
    RUN_TEST(an_erased_glow_is_remembered);
    RUN_TEST(the_glow_can_be_drawn_again_after_being_erased);
    RUN_TEST(asking_twice_gives_the_same_glow);

    RUN_TEST(a_burning_light_can_leave_no_glow_on_the_map);
    RUN_TEST(a_glow_can_outlive_the_light_that_made_it);

    return TEST_SUMMARY();
}
