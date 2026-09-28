// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「いま誰の手番か」のテスト -- 現在のふるまいを保護する
 *
 * ダンジョンとその中身から出す 1 つめの問い（#18-14-1）。もとは
 * variable.c:39 の `int hack_monptr = -1;` で、creature.c が 10 か所で
 * 置きかえ、misc1.c と moria3.c が `hack_monptr < i` と手で書いて読み、
 * game_state.c が写しとっていた。
 *
 * **この数は 1 つの問いにしか答えない** ——
 * **モンスター表をいま詰めなおしてよいか**。m_list は隙間なく詰めてあり、
 * delete_monster() は空いた席を末尾の 1 体で埋めて mfptr を減らす。つまり
 * **その席より上の番号が全部ずれる**。手番と手番のあいだならそれで構わない
 * が、creatures() が表をたどっている最中は違う —— たどっている側は番号を
 * 手に持っていて、その番号が指す相手が入れかわってしまう。
 *
 * だからモンスターを消す 2 か所（misc1.c の詰めなおしと moria3.c の死）が
 * ここに訊く。**たどりがまだその席まで来ていなければ詰めてよく**
 * （delete_monster）、**来ていたら隙間を隙間のまま残す**
 * （fix1_delete_monster。こちらは mfptr を減らさない）。
 *
 * **-1 はモンスターではありえない。** 番号は MIN_MONIX（2）から始まるので、
 * `the_turn < index` の 1 つの比較が「まだ来ていない」と「そもそも
 * たどっていない」の両方を兼ねる。テストの重心はここ ——
 * **境目（自分の席・1 つ下・1 つ上）**と、**休みの値でどう答えるか**。
 *
 * セーブファイルには無い（save.c は書かない）ので、戻す窓口も無い。
 *
 * テストは 1 プロセスで状態を共有するので、各件が最初に手番を閉じなおす。
 */
/* externs.h は要らない。窓口 4 本と、型と MIN_MONIX のための
 * constant.h / types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "monster_turn.h"

#include "minunit.h"

/* 誰の手番でもないところから始める（走りだしと同じ）。**窓口で閉じなおす**
 * —— 置き場が src/monster_turn.c の static なので、この足場が唯一の道。 */
static void given_nobodys_turn(void) { monster_turn_end(); }

/* creatures() が index 番のモンスターを処理している最中。 */
static void given_the_turn_of(int index) {
    monster_turn_end();
    monster_turn_begin(index);
}

/* ------------------------------------------------------------------
 * 走りだし -- 置き場の初期値そのもの
 * ------------------------------------------------------------------ */

/* **この 1 件だけは足場を呼ばない。** 呼ぶと monster_turn_end() が -1 を
 * 書いてしまい、**置き場の初期値（`static int the_turn = -1;`）が観測でき
 * なくなる** —— 初期値を 2 に変えても他の 13 件はグリーンのままだった。
 * だから main() の**いちばん最初**に置いてあり、ここより前に窓口を呼ぶ件を
 * 足してはいけない。
 *
 * 見ているのはゲームの走りだしと、セーブを読んだ直後の両方（save.c は
 * この数を書かないので、どちらも初期値のまま始まる）。 */
TEST(nobodys_turn_before_anything_has_happened) {
    ASSERT_EQ_INT(-1, monster_turn_index());
    ASSERT_TRUE(monster_delete_may_shift(MIN_MONIX));
}

/* ------------------------------------------------------------------
 * 休みの値 -- 誰の手番でもないときは何でも詰めてよい
 * ------------------------------------------------------------------ */

/* 閉じたあとも -1（もとの `hack_monptr = -1;` の代入）。 */
TEST(nobodys_turn_to_start_with) {
    given_nobodys_turn();

    ASSERT_EQ_INT(-1, monster_turn_index());
}

/* **いちばん小さい番号でも詰めてよい。** -1 < 2 なので、休みの値は
 * 「ぜんぶ許す」と同じ意味になる。**休みの値が MIN_MONIX 以上になると
 * その席が詰められなくなる**（-1 を 2 に変えるとこの 1 件が落ちる。
 * 0 や 1 では落ちない —— そちらを押さえているのは下の
 * ending_the_turn_puts_the_resting_value_back で、生の -1 を
 * game_state.c が写しとるから）。 */
TEST(the_first_monster_can_be_shifted_when_nobody_is_acting) {
    given_nobodys_turn();

    ASSERT_TRUE(monster_delete_may_shift(MIN_MONIX));
}

/* 表のどこでも同じ。 */
TEST(any_monster_can_be_shifted_when_nobody_is_acting) {
    given_nobodys_turn();

    for (int index = MIN_MONIX; index < MIN_MONIX + 100; index++) {
        ASSERT_TRUE(monster_delete_may_shift(index));
    }
}

/* ------------------------------------------------------------------
 * 手番のあいだ -- 境目は「自分の席」
 * ------------------------------------------------------------------ */

/* **自分の席は詰めさせない。** 比較は `<` で、等しいときは偽。
 * もとの moria3.c は自分が死んだときこの答えで fix1_delete_monster() を
 * 選ぶ —— **ここを `<=` にすると、いま処理中の 1 体を消したときに
 * 末尾の 1 体がその席に来て、creatures() が同じ番号でそれを二度処理する。** */
TEST(the_monster_being_acted_for_must_not_be_shifted) {
    given_the_turn_of(5);

    ASSERT_FALSE(monster_delete_may_shift(5));
}

/* **すでに通りすぎた席も詰めさせない。** たどりは番号の小さいほうから
 * 来ているので、5 番の手番なら 4 番はもう処理済み。詰めると末尾の 1 体が
 * そこへ降りてきて、**処理済みの席に未処理の 1 体が座る**。 */
TEST(a_monster_the_walk_has_passed_must_not_be_shifted) {
    given_the_turn_of(5);

    ASSERT_FALSE(monster_delete_may_shift(4));
    ASSERT_FALSE(monster_delete_may_shift(MIN_MONIX));
}

/* **まだ来ていない席は詰めてよい。** これがあるおかげで、召喚で表が
 * あふれたときの詰めなおし（misc1.c）が手番の最中でも働く。 */
TEST(a_monster_the_walk_has_not_reached_can_be_shifted) {
    given_the_turn_of(5);

    ASSERT_TRUE(monster_delete_may_shift(6));
    ASSERT_TRUE(monster_delete_may_shift(1000));
}

/* 境目を 1 か所にまとめて見る（4・5 が偽、6 が真）。 */
TEST(the_line_falls_just_above_the_monster_being_acted_for) {
    given_the_turn_of(5);

    ASSERT_FALSE(monster_delete_may_shift(5 - 1));
    ASSERT_FALSE(monster_delete_may_shift(5));
    ASSERT_TRUE(monster_delete_may_shift(5 + 1));
}

/* いちばん小さい番号の手番でも同じ形。**MIN_MONIX の手番では
 * 詰めなおせる席が 1 つも下に無い。** */
TEST(the_line_holds_at_the_first_monster) {
    given_the_turn_of(MIN_MONIX);

    ASSERT_FALSE(monster_delete_may_shift(MIN_MONIX));
    ASSERT_TRUE(monster_delete_may_shift(MIN_MONIX + 1));
}

/* ------------------------------------------------------------------
 * 窓の開け閉め -- 5 対が守っていること
 * ------------------------------------------------------------------ */

/* 閉じれば休みの値に戻る（creature.c の 5 対の後ろ半分）。 */
TEST(ending_the_turn_lets_the_list_be_shifted_again) {
    given_the_turn_of(5);

    monster_turn_end();

    ASSERT_TRUE(monster_delete_may_shift(5));
    ASSERT_TRUE(monster_delete_may_shift(MIN_MONIX));
}

TEST(ending_the_turn_puts_the_resting_value_back) {
    given_the_turn_of(5);

    monster_turn_end();

    ASSERT_EQ_INT(-1, monster_turn_index());
}

/* **開けるのは置きかえで、入れ子ではない。** 5 対はどれも 1 つの呼びを
 * 挟むだけで重ならないが、重ねたときに前の値が戻る作りにはしていない
 * （もとの代入も戻さなかった）。**数えあげる形にすると閉じたときに
 * -1 に戻らない。** */
TEST(beginning_a_turn_replaces_whatever_was_there) {
    given_the_turn_of(5);

    monster_turn_begin(9);

    ASSERT_EQ_INT(9, monster_turn_index());
    ASSERT_FALSE(monster_delete_may_shift(9));
    ASSERT_TRUE(monster_delete_may_shift(10));
}

/* creatures() は表を 1 体ずつ進む。進めても答えは同じ形。 */
TEST(the_line_moves_up_with_the_walk) {
    given_nobodys_turn();

    for (int monptr = MIN_MONIX; monptr < MIN_MONIX + 20; monptr++) {
        monster_turn_begin(monptr);

        ASSERT_FALSE(monster_delete_may_shift(monptr));
        ASSERT_TRUE(monster_delete_may_shift(monptr + 1));

        monster_turn_end();
    }
}

/* ------------------------------------------------------------------
 * 写しとり -- game_state.c だけが生の数を見る
 * ------------------------------------------------------------------ */

/* 入れた番号がそのまま出てくる（丸めも囲いもしない。11・12 つめの
 * 窓口と同じ形）。 */
TEST(the_index_that_goes_in_is_the_one_that_comes_out) {
    given_nobodys_turn();

    for (int index = MIN_MONIX; index < MIN_MONIX + 50; index++) {
        monster_turn_begin(index);

        ASSERT_EQ_INT(index, monster_turn_index());
    }
}

/* 訊いても減らない・変わらない（読みは副作用を持たない）。 */
TEST(asking_does_not_change_the_answer) {
    given_the_turn_of(5);

    for (int i = 0; i < 100; i++) {
        (void)monster_delete_may_shift(5);
        (void)monster_delete_may_shift(6);
    }

    ASSERT_EQ_INT(5, monster_turn_index());
}

int main(void) {
    /* 足場を呼ばない 1 件。**この行より前に何も足さないこと**（上の註）。 */
    RUN_TEST(nobodys_turn_before_anything_has_happened);

    RUN_TEST(nobodys_turn_to_start_with);
    RUN_TEST(the_first_monster_can_be_shifted_when_nobody_is_acting);
    RUN_TEST(any_monster_can_be_shifted_when_nobody_is_acting);

    RUN_TEST(the_monster_being_acted_for_must_not_be_shifted);
    RUN_TEST(a_monster_the_walk_has_passed_must_not_be_shifted);
    RUN_TEST(a_monster_the_walk_has_not_reached_can_be_shifted);
    RUN_TEST(the_line_falls_just_above_the_monster_being_acted_for);
    RUN_TEST(the_line_holds_at_the_first_monster);

    RUN_TEST(ending_the_turn_lets_the_list_be_shifted_again);
    RUN_TEST(ending_the_turn_puts_the_resting_value_back);
    RUN_TEST(beginning_a_turn_replaces_whatever_was_there);
    RUN_TEST(the_line_moves_up_with_the_walk);

    RUN_TEST(the_index_that_goes_in_is_the_one_that_comes_out);
    RUN_TEST(asking_does_not_change_the_answer);

    return TEST_SUMMARY();
}
