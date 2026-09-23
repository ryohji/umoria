/* 明かりを持っているかどうかの置き場のテスト -- 現在のふるまいを保護する
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

int main(void) {
    /* 走りだしの状態を見る 1 件を最初に。以降のテストが書きこむ。 */
    RUN_TEST(the_character_starts_out_without_a_light);

    RUN_TEST(a_burning_light_is_remembered);
    RUN_TEST(a_light_that_has_gone_out_is_remembered);
    RUN_TEST(the_light_can_come_back_after_going_out);
    RUN_TEST(asking_twice_gives_the_same_answer);

    RUN_TEST(setting_the_light_twice_leaves_it_lit);
    RUN_TEST(setting_the_dark_twice_leaves_it_dark);

    return TEST_SUMMARY();
}
