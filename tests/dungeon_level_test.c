// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「いま何階か」のテスト -- 現在のふるまいを保護する
 *
 * ダンジョンとその中身から出す 6 つめの問い（#18-14-6）。もとは
 * variable.c:74 の `int16_t dun_level = 0;` 1 行で、12 ファイル 40 参照。
 *
 * 押さえたいことの重心は 3 つ:
 *   **①0 は町で、それが走りだしの値** —— 5 問めの「この階の広さ」と違って、
 *     **初期値そのものがゲームの中身**（新しいゲームは町から始まる）。
 *     最初の階はこの数から作られるので、誰かが先に置くわけではない。
 *   **②同じ問いが 3 通りに書かれていた** —— 「町にいるか」を訊く 6 か所が
 *     `!= 0`（店の品が入れかわるか）・`> 0`（呼びもどしの巻きあげ・部屋を
 *     照らす・破壊の呪文）・`== 0`（次の階の広さ）と書いていた。
 *     **3 通りが同じ答えになること**を 1 件で押さえる —— 負の深さは
 *     入らない（町に上り階段が無いから。比べている場所は 1 つも無い）。
 *   **③窓口は上限を持たない・何も検めない** —— 階を名ざす側の規則
 *     （魔法使いの 0〜99、深降りの巻物の 1 どまり）は呼び手に残っている。
 *     セーブファイルから来た数をそのまま置くのが復元の道でもある。
 *
 * セーブファイルには short 1 つで出る。読みもどした値は**本当に使われる**
 * （いる階そのもの。復元では階を作りなおさない）。
 *
 * テストは 1 プロセスで状態を共有するので、各件が最初に階を置きなおす。
 */
/* externs.h は要らない。窓口 3 本と、深さの読み手が当てにしている
 * 定数のための constant.h と、セーブの往復を写すための <stdint.h> だけ。 */
#include <stdint.h>

#include "config.h"
#include "constant.h"

#include "dungeon_level.h"

#include "minunit.h"

/* 町（新しいゲームの走りだしと同じ数）。 */
static void given_the_town(void) { set_dungeon_level(0); }

/* ダンジョンの n 階。 */
static void given_dungeon_level(int level) { set_dungeon_level(level); }

/* ------------------------------------------------------------------
 * ①走りだし -- 置き場の初期値は「町」という中身を持っている
 * ------------------------------------------------------------------ */

/* **この 1 件だけは足場を呼ばない。** 呼ぶと階が入ってしまい、**置き場の
 * 初期値が観測できなくなる** —— 初期値を 5 に変えてもこの 1 件しか
 * 赤にならない。だから main() の**いちばん最初**に置いてあり、ここより前に
 * 窓口を呼ぶ件を足してはいけない（#18-14-1〜5 と同じ作法。所見 52）。
 *
 * **5 問めの「広さ 0」と違って、ここの初期値はゲームの中の状態**
 * —— 新しいゲームは町から始まり、**最初の階はこの数から作られる**
 * （generate_cave() がこの数を見て町かダンジョンかを決める）。
 * だから 0 が消えると「新しいゲームがどこから始まるか」が変わる。 */
TEST(a_new_game_starts_in_the_town) {
    ASSERT_EQ_INT(0, dungeon_level());
    ASSERT_TRUE(player_is_in_town());
}

/* ------------------------------------------------------------------
 * ②「町にいるか」は 1 つの問い
 * ------------------------------------------------------------------ */

/* 町は 0 階。 */
TEST(the_town_is_level_zero) {
    given_the_town();

    ASSERT_EQ_INT(0, dungeon_level());
    ASSERT_TRUE(player_is_in_town());
}

/* **いちばん浅いダンジョンの階でも、もう町ではない。** */
TEST(the_first_dungeon_level_is_not_the_town) {
    given_dungeon_level(1);

    ASSERT_EQ_INT(1, dungeon_level());
    ASSERT_FALSE(player_is_in_town());
}

/* **3 通りの書きかたが 1 つの問いだったこと。** `!= 0`・`> 0`・`== 0` は
 * どの階でも同じ答えを出す —— 窓口が `== 0` 以外（たとえば `<= 0`）に
 * なってもここは赤にならないが、**負の深さが入る道ができた日には赤になる**。
 * それが「3 通りは同じ」と言えている理由そのもの。 */
TEST(the_three_hand_written_town_tests_agree) {
    const int levels[] = {0, 1, 2, WIN_MON_APPEAR, 99};

    for (int i = 0; i < (int)(sizeof(levels) / sizeof(levels[0])); i++) {
        given_dungeon_level(levels[i]);

        ASSERT_TRUE(player_is_in_town() == (dungeon_level() == 0));
        ASSERT_TRUE(player_is_in_town() == !(dungeon_level() != 0));
        ASSERT_TRUE(player_is_in_town() == !(dungeon_level() > 0));
    }
}

/* 訊いても答えは変わらない（窓口は読むだけ）。 */
TEST(asking_does_not_change_the_answer) {
    given_dungeon_level(12);

    ASSERT_EQ_INT(dungeon_level(), dungeon_level());
    ASSERT_TRUE(player_is_in_town() == player_is_in_town());
    ASSERT_EQ_INT(12, dungeon_level());
}

/* ------------------------------------------------------------------
 * 階を移る -- 呼び手が書いている形をそのまま写す
 * ------------------------------------------------------------------ */

/* 新しい階は前の階を消す。 */
TEST(a_new_level_replaces_the_old_one) {
    given_dungeon_level(3);

    set_dungeon_level(7);

    ASSERT_EQ_INT(7, dungeon_level());
}

/* **階段の形**（`leave_for_level(depth + 1)` と `(depth - 1)`）。
 * 下りて下りて上がると元の階に戻る。 */
TEST(the_staircases_step_by_one) {
    given_dungeon_level(10);

    set_dungeon_level(dungeon_level() + 1);
    set_dungeon_level(dungeon_level() + 1);
    ASSERT_EQ_INT(12, dungeon_level());

    set_dungeon_level(dungeon_level() - 1);
    ASSERT_EQ_INT(11, dungeon_level());
}

/* **町から出る 1 歩めは 1 階**（町の下り階段）。 */
TEST(leaving_the_town_goes_to_the_first_level) {
    given_the_town();

    set_dungeon_level(dungeon_level() + 1);

    ASSERT_EQ_INT(1, dungeon_level());
    ASSERT_FALSE(player_is_in_town());
}

/* **町に帰る書きかたは 2 か所あって、どちらも同じ 1 つの書き** ——
 * 王になったとき（death.c の kingly()）と、死んだキャラクターを
 * セーブファイルから生きかえらせたとき（save.c）。どちらも 0 を置く。 */
TEST(the_two_ways_back_to_the_town_write_the_same_level) {
    given_dungeon_level(50);

    set_dungeon_level(0);

    ASSERT_EQ_INT(0, dungeon_level());
    ASSERT_TRUE(player_is_in_town());
}

/* ------------------------------------------------------------------
 * ③窓口は上限を持たない・何も検めない
 * ------------------------------------------------------------------ */

/* **ありえない階もはじかない。** 0〜99 は魔法使いの入口の規則、
 * 1 どまりは深降りの巻物の規則で、**どちらも呼び手に残っている**
 * （`leave_for_level()` が限りを持たないのと同じ分けかた）。
 * 検める側に回るとセーブファイルの復元が壊れる。 */
TEST(the_window_does_not_reject_an_impossible_level) {
    given_dungeon_level(10);

    set_dungeon_level(100);
    ASSERT_EQ_INT(100, dungeon_level());

    set_dungeon_level(-1);
    ASSERT_EQ_INT(-1, dungeon_level());

    given_the_town();
    ASSERT_TRUE(player_is_in_town());
}

/* ------------------------------------------------------------------
 * 深さとしての読み -- 13 か所が当てにしている形
 * ------------------------------------------------------------------ */

/* **床にばらまく量の割り算**（generate.c の `depth / 3` を 2〜10 に挟む）。
 * 窓口が int を返すので、切りすては呼び手の側で起きる。 */
TEST(the_scatter_level_comes_from_a_third_of_the_depth) {
    given_dungeon_level(30);
    ASSERT_EQ_INT(10, dungeon_level() / 3);

    given_dungeon_level(29);
    ASSERT_EQ_INT(9, dungeon_level() / 3);

    given_the_town();
    ASSERT_EQ_INT(0, dungeon_level() / 3);
}

/* **勝ちのモンスターが出はじめる深さ**（`>= WIN_MON_APPEAR`）。
 * 1 階ずれると 49 階で出てしまう。 */
TEST(the_winning_monster_appears_from_a_fixed_depth) {
    given_dungeon_level(WIN_MON_APPEAR - 1);
    ASSERT_FALSE(dungeon_level() >= WIN_MON_APPEAR);

    given_dungeon_level(WIN_MON_APPEAR);
    ASSERT_TRUE(dungeon_level() >= WIN_MON_APPEAR);
}

/* **呼びだされたモンスターはこの階より深いところから来る**
 * （`get_mons_num(depth + MON_SUMMON_ADJ)`）。足す前の数を窓口が返す。 */
TEST(a_summoned_monster_comes_from_deeper_than_this_level) {
    given_dungeon_level(20);

    ASSERT_EQ_INT(22, dungeon_level() + MON_SUMMON_ADJ);
    ASSERT_TRUE(dungeon_level() + MON_SUMMON_ADJ > dungeon_level());
}

/* **`depth * 50` は 2 か所にあって、単位が違う** —— 画面の状態行は
 * **フィート**（1 階 50 フィート。町は数ではなく "Town level" と出る）、
 * 得点は**点**。同じ式でも写しではないので窓口にしていない
 * （所見 54 の「畳む前に、写しが本当に写しか確かめる」）。
 * ここで押さえるのは**町が 0 を出すこと**だけ —— そこが動くと
 * 状態行が「0 feet」と言いはじめる。 */
TEST(the_town_has_no_depth_to_print) {
    given_the_town();
    ASSERT_EQ_INT(0, dungeon_level() * 50);

    given_dungeon_level(1);
    ASSERT_EQ_INT(50, dungeon_level() * 50);

    given_dungeon_level(99);
    ASSERT_EQ_INT(4950, dungeon_level() * 50);
}

/* ------------------------------------------------------------------
 * セーブファイルの往復
 * ------------------------------------------------------------------ */

/* **short で足りる。** save.c は `wr_short((uint16_t)dun_level)` で書き、
 * 読みもどしは 16 ビットをそのまま置きなおす。いちばん深い階でも 99 なので、
 * 書いて読んだ値は元と同じ。 */
TEST(the_deepest_level_goes_through_a_short_unchanged) {
    given_dungeon_level(99);

    /* save.c の書きと読みをそのまま写す。 */
    const uint16_t written = (uint16_t)dungeon_level();
    given_the_town();
    set_dungeon_level((int16_t)written);

    ASSERT_EQ_INT(99, dungeon_level());
    ASSERT_FALSE(player_is_in_town());
}

/* **復元では階を作りなおさない**ので、ファイルから入れた数がそのまま
 * 「いる階」になる（#18-14-3 の予算と違って捨てられない）。 */
TEST(a_restored_level_is_the_level_that_is_played) {
    given_the_town();

    set_dungeon_level(37);

    ASSERT_EQ_INT(37, dungeon_level());
    ASSERT_FALSE(player_is_in_town());
}

int main(void) {
    /* 足場を呼ばない 1 件。**この行より前に何も足さないこと**（上の註）。 */
    RUN_TEST(a_new_game_starts_in_the_town);

    RUN_TEST(the_town_is_level_zero);
    RUN_TEST(the_first_dungeon_level_is_not_the_town);
    RUN_TEST(the_three_hand_written_town_tests_agree);
    RUN_TEST(asking_does_not_change_the_answer);

    RUN_TEST(a_new_level_replaces_the_old_one);
    RUN_TEST(the_staircases_step_by_one);
    RUN_TEST(leaving_the_town_goes_to_the_first_level);
    RUN_TEST(the_two_ways_back_to_the_town_write_the_same_level);

    RUN_TEST(the_window_does_not_reject_an_impossible_level);

    RUN_TEST(the_scatter_level_comes_from_a_third_of_the_depth);
    RUN_TEST(the_winning_monster_appears_from_a_fixed_depth);
    RUN_TEST(a_summoned_monster_comes_from_deeper_than_this_level);
    RUN_TEST(the_town_has_no_depth_to_print);

    RUN_TEST(the_deepest_level_goes_through_a_short_unchanged);
    RUN_TEST(a_restored_level_is_the_level_that_is_played);

    return TEST_SUMMARY();
}
