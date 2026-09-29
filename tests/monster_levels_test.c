// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* レベルごとのモンスター定義の索引のテスト -- 現在のふるまいを保護する
 *
 * ダンジョンとその中身から出す 2 つめの問い（#18-14-2）。もとは
 * monsters.c:765 の `int16_t m_level[MAX_MONS_LEVEL + 1];` で、組みたてるのは
 * main.c の **static な** init_m_level() 1 か所だけ、読むのは misc1.c の 4 か所
 * （勝ちのモンスターの置き場・get_mons_num() の 3 か所）と spells.c の 2 か所
 * （変身と大虐殺の置きなおし）。
 *
 * **モンスター定義表はもともとレベルの昇順に書かれている**ので、この索引が
 * 返す数は**そのままモンスターの番号**になる（品物のほうは並べなおした本体
 * sorted_objects が別に要った → object_levels.h）。レベル L のモンスターは
 * first_monster_at_level(L) 番から monsters_up_to_level(L) - 1 番までに並ぶ。
 *
 * **組みたてる側はテストから届いていなかった** —— main.c は main() を持つので
 * リンクできない。#18-10 の init_t_level() とまったく同じ形で、module に移して
 * 初めて数え上げが届く（このファイルの第 1 節）。
 *
 * 押さえたいことの重心は 3 つ。
 *   **①帯が表の並びと合っていること** —— L の帯にいるモンスターの level が
 *     本当に L か。ここが崩れると「ダンジョンに出るモンスターが全部ちがう」
 *     という形で出る（表がレベル順に書かれているという前提そのもの）。
 *   **②帯が隙間なく足し合わさること** —— 町の 8 体＋各レベルの帯＝277 体。
 *   **③勝ちのモンスターは帯の外** —— level が MAX_MONS_LEVEL より深いので
 *     どの帯にも数えられず、その手前で帯が終わる。place_win_monster() は
 *     その終わりの番号を起点に 2 体を指す。
 *
 * セーブファイルには無い（起動のたびに組みなおす）ので、戻す窓口も無い。
 *
 * 期待値はすべて現在の表（monsters.c の 279 体）が返した実際の値。
 */
/* externs.h は要らない。窓口と、型と定数のための constant.h / types.h だけ。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "monster_levels.h"

/* 表そのものは #17 から monsters.c の static で、番号から定義を引く窓口しか
 * 無い。帯の中身を確かめるのに使うので、必要な 2 本だけ宣言する。 */
creature_handle monster_make_creature_handle(uint16_t index);
creature_type *monster_get_creature(creature_handle h);

/* monsters.c の monster_name_indefinite() が呼ぶ。名前は 1 件も見ないので
 * 代役で足りる（tests/creature_iterator_test.c と同じ）。 */
bool is_a_vowel(char ch) { (void)ch; return false; }

#include "minunit.h"

/* 起動時に main() が一度呼ぶのと同じ（窓口が唯一の道）。 */
static void given_the_index_built(void) { monster_levels_init(); }

static int level_of(int number) {
    return monster_get_creature(monster_make_creature_handle((uint16_t)number))->level;
}

/* ------------------------------------------------------------------
 * 組みたてる前 -- 置き場の初期値そのもの
 * ------------------------------------------------------------------ */

/* **この 1 件だけは前置きを呼ばない。** 呼ぶと索引が組まれてしまい、
 * **組みたてる前は帯が空**（`static int16_t m_level[...]` のゼロ）だという
 * ことが観測できなくなる。だから main() の**いちばん最初**に置いてあり、
 * ここより前に窓口を呼ぶ件を足してはいけない（#18-14-1 と同じ形）。
 *
 * これが「main() が最初の洞窟を作る前に monster_levels_init() を呼ばねば
 * ならない」理由そのもの —— 呼ばなければ randint(0) に落ちる。 */
TEST(every_band_is_empty_before_the_index_is_built) {
    ASSERT_EQ_INT(0, monsters_up_to_level(0));
    ASSERT_EQ_INT(0, monsters_up_to_level(MAX_MONS_LEVEL));
    ASSERT_EQ_INT(0, monsters_at_level(1));
    ASSERT_EQ_INT(0, first_monster_at_level(1));
}

/* ------------------------------------------------------------------
 * 町 -- レベル 0 の帯
 * ------------------------------------------------------------------ */

/* **町のモンスターは 8 体で、表のいちばん前**（浮浪児から傷だらけの傭兵まで）。
 * get_mons_num() はレベル 0 のときだけこの帯から選び、それ以外のときは
 * first_monster_at_level(1) から先だけを見る。 */
TEST(the_town_monsters_are_the_level_zero_band) {
    given_the_index_built();

    ASSERT_EQ_INT(8, monsters_up_to_level(0));
    ASSERT_EQ_INT(8, first_monster_at_level(1));
}

TEST(the_level_zero_band_holds_only_town_monsters) {
    given_the_index_built();

    for (int number = 0; number < monsters_up_to_level(0); number++) {
        ASSERT_EQ_INT(0, level_of(number));
    }
}

/* ------------------------------------------------------------------
 * 帯 -- 索引と表の並びが合っていること
 * ------------------------------------------------------------------ */

/* **これがこの module のいちばん大事な不変条件** —— L の帯にいるのは
 * level が L のモンスターだけ。表がレベルの昇順に書かれているから成りたつ
 * ことで、崩れると「出るモンスターが全部ちがう」という形で出る。 */
TEST(every_monster_in_a_band_really_has_that_level) {
    given_the_index_built();

    for (int level = 1; level <= MAX_MONS_LEVEL; level++) {
        for (int number = first_monster_at_level(level); number < monsters_up_to_level(level); number++) {
            ASSERT_EQ_INT(level, level_of(number));
        }
    }
}

/* 帯は 2 つの累計の差で、始まりは 1 つ手前の累計（窓口 3 本の関係）。 */
TEST(a_band_is_the_gap_between_two_counts) {
    given_the_index_built();

    for (int level = 1; level <= MAX_MONS_LEVEL; level++) {
        ASSERT_EQ_INT(monsters_up_to_level(level) - monsters_up_to_level(level - 1), monsters_at_level(level));
        ASSERT_EQ_INT(monsters_up_to_level(level - 1), first_monster_at_level(level));
    }
}

/* 深くなるほど累計は減らない（累積索引であることそのもの）。 */
TEST(counts_never_go_down_as_the_level_deepens) {
    given_the_index_built();

    for (int level = 1; level <= MAX_MONS_LEVEL; level++) {
        ASSERT_TRUE(monsters_up_to_level(level) >= monsters_up_to_level(level - 1));
    }
}

/* **どのレベルにも 1 体はいる。** get_mons_num() は
 * `randint(monsters_at_level(level))` で帯の中から選ぶので、**空の帯があると
 * randint(0) を呼ぶ**ことになる。表がそうなっていないことを固定しておく。 */
TEST(no_level_is_empty) {
    given_the_index_built();

    for (int level = 1; level <= MAX_MONS_LEVEL; level++) {
        ASSERT_TRUE(monsters_at_level(level) >= 1);
    }
}

/* 町の 8 体＋各レベルの帯＝いちばん深い累計。隙間も重なりも無い。 */
TEST(the_bands_add_up_to_the_whole_table) {
    given_the_index_built();

    int total = monsters_up_to_level(0);
    for (int level = 1; level <= MAX_MONS_LEVEL; level++) {
        total += monsters_at_level(level);
    }

    ASSERT_EQ_INT(monsters_up_to_level(MAX_MONS_LEVEL), total);
}

/* 実際の数を 2 本だけ釘で打つ（いちばん浅い帯といちばん深い帯）。 */
TEST(the_first_and_last_bands_have_the_numbers_the_table_gives) {
    given_the_index_built();

    ASSERT_EQ_INT(11, monsters_at_level(1));
    ASSERT_EQ_INT(8, first_monster_at_level(1));

    ASSERT_EQ_INT(7, monsters_at_level(MAX_MONS_LEVEL));
    ASSERT_EQ_INT(270, first_monster_at_level(MAX_MONS_LEVEL));
}

/* ------------------------------------------------------------------
 * 帯の外 -- 勝ちのモンスター
 * ------------------------------------------------------------------ */

/* **level が MAX_MONS_LEVEL より深いモンスターはどの帯にも数えられない。**
 * 表の末尾 2 体（Evil Iggy は level 50・Balrog は 100）がそれで、
 * place_win_monster() は monsters_up_to_level(MAX_MONS_LEVEL) を起点に
 * WIN_MON_TOT 体を指す。**深さで出ることは一度も無い。** */
TEST(the_win_monsters_sit_past_the_last_band) {
    given_the_index_built();

    ASSERT_EQ_INT(277, monsters_up_to_level(MAX_MONS_LEVEL));
    ASSERT_EQ_INT(WIN_MON_TOT, MAX_CREATURES - monsters_up_to_level(MAX_MONS_LEVEL));

    for (int number = monsters_up_to_level(MAX_MONS_LEVEL); number < MAX_CREATURES; number++) {
        ASSERT_TRUE(level_of(number) > MAX_MONS_LEVEL);
    }
}

/* ------------------------------------------------------------------
 * 呼び手の言いかた -- 「レベル 1 以上から 1 体」
 * ------------------------------------------------------------------ */

/* spells.c の変身と大虐殺は「町を除いた全部から 1 体」を選ぶ。その幅と
 * 起点を窓口 2 本で言えることを固定する（もとは
 * `m_level[MAX_MONS_LEVEL] - m_level[0]` と `+ m_level[0]`）。 */
TEST(the_band_of_level_one_and_deeper_skips_the_town) {
    given_the_index_built();

    const int first = first_monster_at_level(1);
    const int width = monsters_up_to_level(MAX_MONS_LEVEL) - first;

    ASSERT_EQ_INT(8, first);
    ASSERT_EQ_INT(269, width);
    ASSERT_EQ_INT(monsters_up_to_level(MAX_MONS_LEVEL), first + width);
    ASSERT_TRUE(level_of(first) >= 1);
    ASSERT_TRUE(level_of(first + width - 1) <= MAX_MONS_LEVEL);
}

/* ------------------------------------------------------------------
 * 組みなおし -- 起動のたびに一度
 * ------------------------------------------------------------------ */

/* **2 度組んでも同じ。** 数えはじめに 0 に戻す行がこれを守っている ——
 * その行を消すと 2 度めで数が倍になる。セーブを読みもどしたときも
 * （表は同じなので）同じ索引になる。 */
TEST(building_the_index_twice_gives_the_same_bands) {
    given_the_index_built();

    const int before_town = monsters_up_to_level(0);
    const int before_all = monsters_up_to_level(MAX_MONS_LEVEL);

    monster_levels_init();

    ASSERT_EQ_INT(before_town, monsters_up_to_level(0));
    ASSERT_EQ_INT(before_all, monsters_up_to_level(MAX_MONS_LEVEL));
}

int main(void) {
    /* 前置きを呼ばない 1 件。**この行より前に何も足さないこと**（上の註）。 */
    RUN_TEST(every_band_is_empty_before_the_index_is_built);

    RUN_TEST(the_town_monsters_are_the_level_zero_band);
    RUN_TEST(the_level_zero_band_holds_only_town_monsters);

    RUN_TEST(every_monster_in_a_band_really_has_that_level);
    RUN_TEST(a_band_is_the_gap_between_two_counts);
    RUN_TEST(counts_never_go_down_as_the_level_deepens);
    RUN_TEST(no_level_is_empty);
    RUN_TEST(the_bands_add_up_to_the_whole_table);
    RUN_TEST(the_first_and_last_bands_have_the_numbers_the_table_gives);

    RUN_TEST(the_win_monsters_sit_past_the_last_band);

    RUN_TEST(the_band_of_level_one_and_deeper_skips_the_town);

    RUN_TEST(building_the_index_twice_gives_the_same_bands);

    return TEST_SUMMARY();
}
