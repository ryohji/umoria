// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「手が光っているか」のテスト -- 現在のふるまいを保護する
 *
 * py から出す 13 つめの問い（1 つめは持っている金、2 つめは腹の具合、3 つめは
 * 画面に出す数字、4 つめは魔力、5 つめは体力、6 つめは階級と経験値、7 つめは
 * 状態の旗、8 つめは装備で決まる耐性・能力、9 つめは一時的な状態の数えおとし、
 * 10 つめは休息の残りターン、11 つめはいまの速さ、12 つめは赤外視の届く距離）。
 * 答えは 1 バイト —— もとは py.flags.confuse_monster で、4 ファイルから
 * 8 か所が触っていた。
 *
 * **名前が関数と衝突している。** externs.h:538 に
 * `int confuse_monster(int, int, int);`（src/item/spells.c:1075。杖と巻物が向きを
 * 指定してモンスターを混乱させる呪文）があり、フィールドのほうは
 * **プレイヤーが持っている 1 回ぶんの蓄え**。別物なので、module も窓口も
 * game の message に合わせて「手」を名前にした（player_glowing_hands.h）。
 *
 * **時計ではなく蓄え。** この列でいちばん近いのは 9 つめの数えおとしだが、
 * 重心は 3 つとも違う:
 *
 *   1. **ターンで減らない。** 巻物 11 で立ったら、休んでも歩いても階段を
 *      降りても光ったまま。**消えるのは 1 撃が当たったときだけ**（自分が
 *      殴った player_melee.c と、モンスターに殴られた creature.c の両方）。
 *      dungeon.c の時計の列にこのフィールドは無い。
 *   2. **2 枚めの巻物は効かない。** すでに光っていたら `ident` も立たない
 *      （scrolls.c:206 が 0 かどうかを先に訊く）ので、**この読みは鑑定の
 *      成否を決めている**。重ねられないことに釘を打つ。
 *   3. **セーブファイルはバイト。** game が書くのは 0 か 1 だけだが、
 *      読みは rd_byte なので何でも入ってくる。**戻す窓口は 0/1 に丸めず
 *      そのまま通す**ことに釘を打つ（11・12 つめと同じ形）。
 *
 * 相手が混乱するかの判定（抵抗・m_ptr->confused・recall）と message 2 つと
 * `adesc != 99` の番は**この module の外**（player_glowing_hands.h）。
 *
 * テストは 1 プロセスで状態を共有するので、各件が最初に手を暗くしなおす。
 */
/* externs.h は要らない。窓口 4 本と、型のための types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_glowing_hands.h"

#include "minunit.h"

/* 手が暗いところから始める（走りだしと同じ）。**窓口で置きなおす** ——
 * 置き場が src/player/player_glowing_hands.c の static に入っても（#18-12-13C）
 * この足場は届く。 */
static void given_dark_hands(void) { player_glowing_hands_restore(0); }

/* 巻物 11 を読んだところから始める。 */
static void given_glowing_hands(void) {
    player_glowing_hands_restore(0);
    player_glowing_hands_begin();
}

/* ------------------------------------------------------------------
 * 蓄えそのもの -- 0 は光っていない
 * ------------------------------------------------------------------ */

TEST(the_hands_start_dark) {
    given_dark_hands();

    ASSERT_EQ_INT(0, player_glowing_hands());
}

TEST(the_scroll_lights_the_hands) {
    given_dark_hands();

    player_glowing_hands_begin();

    ASSERT_EQ_INT(1, player_glowing_hands());
}

TEST(reading_the_hands_twice_gives_the_same_answer) {
    given_glowing_hands();

    (void)player_glowing_hands();

    ASSERT_EQ_INT(1, player_glowing_hands());
}

/* **重ならない。** 2 枚めの巻物を読んでも蓄えは 1 のまま（呼び手は
 * そもそも 0 かどうかを先に訊くので、ここまで来ない）。**足しにすると
 * 巻物を読むたびに数が増え、1 撃で全部消える形が崩れる。** */
TEST(a_second_scroll_does_not_stack_another_charge) {
    given_glowing_hands();

    player_glowing_hands_begin();

    ASSERT_EQ_INT(1, player_glowing_hands());
}

/* ------------------------------------------------------------------
 * 使う -- 当たった 1 撃で消える
 * ------------------------------------------------------------------ */

/* 自分が殴って当たったとき（player_melee.c）、モンスターに殴られたとき
 * （creature.c）の両方がこれを呼ぶ。 */
TEST(a_blow_that_connects_puts_the_hands_out) {
    given_glowing_hands();

    player_glowing_hands_spend();

    ASSERT_EQ_INT(0, player_glowing_hands());
}

/* 暗い手で殴っても何も起きない（呼び手は先に訊くので、ここも通らない）。 */
TEST(spending_with_dark_hands_leaves_them_dark) {
    given_dark_hands();

    player_glowing_hands_spend();

    ASSERT_EQ_INT(0, player_glowing_hands());
}

/* 1 撃で消えるのは 1 回ぶんだけ。**次の巻物でまた光る。** */
TEST(the_hands_can_be_lit_again_after_a_blow) {
    given_glowing_hands();
    player_glowing_hands_spend();

    player_glowing_hands_begin();

    ASSERT_EQ_INT(1, player_glowing_hands());
}

/* **時計ではない。** 何度訊いても、何ターン経ったつもりでも減らない ——
 * 減らす道は「当たった 1 撃」しかない。**dungeon.c の時計の列に入れると
 * 使わないうちに消える。** */
TEST(nothing_but_a_blow_takes_the_glow_away) {
    given_glowing_hands();

    for (int turn = 0; turn < 100; turn++) {
        (void)player_glowing_hands();
    }

    ASSERT_EQ_INT(1, player_glowing_hands());
}

/* 2 回ぶん入っていても（game は作らないが、セーブファイルからは来る）
 * **1 撃で全部消える**。 */
TEST(a_blow_spends_the_whole_charge_however_big_it_was) {
    given_dark_hands();
    player_glowing_hands_restore(2);

    player_glowing_hands_spend();

    ASSERT_EQ_INT(0, player_glowing_hands());
}

/* ------------------------------------------------------------------
 * セーブファイル -- バイトがそのまま通る
 * ------------------------------------------------------------------ */

TEST(loading_a_saved_game_brings_the_glow_back) {
    given_dark_hands();

    player_glowing_hands_restore(1);

    ASSERT_EQ_INT(1, player_glowing_hands());
}

TEST(loading_a_saved_game_with_dark_hands_leaves_them_dark) {
    given_glowing_hands();

    player_glowing_hands_restore(0);

    ASSERT_EQ_INT(0, player_glowing_hands());
}

/* 戻す窓口は**置きかえ**（足しではない）。**足しにすると、セーブを読む
 * たびに蓄えが増える。** */
TEST(loading_replaces_whatever_was_there) {
    given_glowing_hands();

    player_glowing_hands_restore(0);
    player_glowing_hands_restore(1);

    ASSERT_EQ_INT(1, player_glowing_hands());
}

/* **game が書かない値もそのまま通る。** 書くのは 0 か 1 だけだが、読むのは
 * rd_byte で、丸めるとファイルを読んで書きもどしたときに中身が変わる。 */
TEST(a_byte_the_game_never_writes_travels_through_unchanged) {
    given_dark_hands();

    player_glowing_hands_restore(2);

    ASSERT_EQ_INT(2, player_glowing_hands());
}

/* 器の幅（バイト）いっぱいまで通る。 */
TEST(the_widest_byte_the_container_holds_survives) {
    given_dark_hands();

    player_glowing_hands_restore(255);

    ASSERT_EQ_INT(255, player_glowing_hands());
}

/* 出ていくバイトは入ってきたバイト（save.c は player_glowing_hands() の
 * 返す数をそのまま wr_byte する）。 */
TEST(the_byte_that_goes_out_is_the_one_that_came_in) {
    given_dark_hands();

    for (int byte = 0; byte < 256; byte++) {
        player_glowing_hands_restore(byte);

        ASSERT_EQ_INT(byte, player_glowing_hands());
    }
}

/* 巻物で立てたあとにセーブを書くなら 1 が出ていく。 */
TEST(the_scroll_writes_one_byte_into_the_saved_game) {
    given_dark_hands();

    player_glowing_hands_begin();

    ASSERT_EQ_INT(1, player_glowing_hands());
}

int main(void) {
    RUN_TEST(the_hands_start_dark);
    RUN_TEST(the_scroll_lights_the_hands);
    RUN_TEST(reading_the_hands_twice_gives_the_same_answer);
    RUN_TEST(a_second_scroll_does_not_stack_another_charge);

    RUN_TEST(a_blow_that_connects_puts_the_hands_out);
    RUN_TEST(spending_with_dark_hands_leaves_them_dark);
    RUN_TEST(the_hands_can_be_lit_again_after_a_blow);
    RUN_TEST(nothing_but_a_blow_takes_the_glow_away);
    RUN_TEST(a_blow_spends_the_whole_charge_however_big_it_was);

    RUN_TEST(loading_a_saved_game_brings_the_glow_back);
    RUN_TEST(loading_a_saved_game_with_dark_hands_leaves_them_dark);
    RUN_TEST(loading_replaces_whatever_was_there);
    RUN_TEST(a_byte_the_game_never_writes_travels_through_unchanged);
    RUN_TEST(the_widest_byte_the_container_holds_survives);
    RUN_TEST(the_byte_that_goes_out_is_the_one_that_came_in);
    RUN_TEST(the_scroll_writes_one_byte_into_the_saved_game);

    return TEST_SUMMARY();
}
