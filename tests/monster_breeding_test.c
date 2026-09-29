// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「この階で増えたモンスターは何体か」のテスト -- 現在のふるまいを保護する
 *
 * ダンジョンとその中身から出す 3 つめの問い（#18-14-3）。もとは
 * monsters.c:779 の `int16_t mon_tot_mult;` で、creature.c が 2 か所で増やし、
 * 1 か所で上限と見くらべ、moria3.c が 1 か所で減らし、dungeon.c が階の頭で
 * 0 に戻し、save.c が書いて読んでいた（**別名 1 件** ——
 * `rd_short((uint16_t *)&mon_tot_mult)`）。
 *
 * **この数は 1 つの問いにしか答えない** —— **この階でもう 1 体増やしてよいか**。
 * 増えるモンスター（CM_MULTIPLY。鼠・蠅・蟲の塊）は自分の手番で隣の空きに
 * 自分の写しを置けて、その写しも増えるので、放っておくと階が埋まる。
 * それを止めているのがこの数で、**1 体生まれるごとに 1 つ使い、
 * MAX_MON_MULT を越えると増えなくなる**。
 *
 * **これは「いま何体いるか」でも「そのうち何体が生まれた子か」でもない ——
 * 予算**。テストの重心はその**噛みあわない 3 つ**:
 *   **①上限の比較は `MAX_MON_MULT >= 数` なので、許される出産は 76 回**
 *     （75 回ではない。数が上限と等しくても「よい」と答え、その 1 体で
 *     76 になる）。
 *   **②減るのは「遅れて消える道」を通った 1 体だけで、生まれた子かどうかは
 *     訊かない** —— fix1_delete_monster() は自分の手番で死んだモンスターと
 *     食われたモンスターが通る道。ふつうに消える道（delete_monster）は
 *     1 つも返さない。だから**手で置かれたモンスターを掃除すると、
 *     増えるモンスターの予算がその分だけ増える**。
 *   **③0 より下には行かない** —— もとの `if (mon_tot_mult > 0)`。
 *
 * **階ごと**。降りれば 0 に戻る（dungeon.c）。**セーブファイルには出るが、
 * 読みもどした値はそのまま捨てられる** —— main() は洞窟を作ったときも
 * 読みもどしたときも、ループの最初に dungeon() を呼び、dungeon() が最初の
 * 手番の前にこの数を 0 に戻す。**戻す窓口があるのはファイル書式が動かせない
 * から**で、読みもどした階はいつも予算まるごとから始まる（上流のふるまい）。
 *
 * テストは 1 プロセスで状態を共有するので、各件が最初に階を始めなおす。
 */
/* externs.h は要らない。窓口 6 本と、型と MAX_MON_MULT のための
 * constant.h / types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "monster_breeding.h"

#include "minunit.h"

/* 新しい階に降りたところから始める（dungeon.c の 1 行と同じ）。**窓口で
 * 戻しなおす** —— 置き場が src/monster/monster_breeding.c の static なので、この
 * 足場が唯一の道。 */
static void given_a_fresh_level(void) { monster_breeding_reset(); }

/* 予算を使いきったところ。**76 回**で尽きる（上の①）。 */
static void given_the_budget_spent(void) {
    monster_breeding_reset();
    while (monster_breeding_allowed()) {
        monster_breeding_note_birth();
    }
}

/* ------------------------------------------------------------------
 * 走りだし -- 置き場の初期値そのもの
 * ------------------------------------------------------------------ */

/* **この 1 件だけは足場を呼ばない。** 呼ぶと monster_breeding_reset() が 0 を
 * 書いてしまい、**置き場の初期値（`static int16_t the_count;` のゼロ）が
 * 観測できなくなる** —— 初期値を 5 に変えてもこの 1 件しか赤にならない。
 * だから main() の**いちばん最初**に置いてあり、ここより前に窓口を呼ぶ件を
 * 足してはいけない（#18-14-1・#18-14-2 と同じ作法）。
 *
 * **これはゲームの中では起こらない状態** —— main() は最初の手番の前に
 * dungeon() を通り、そこで 0 が書かれる。押さえているのは
 * **置き場そのものの初期値**で、初期値が 0 でなくなったら
 * 「階の頭で 0 に戻す」以外の道で予算が湧くことになる。 */
TEST(nothing_has_been_bred_before_anything_happens) {
    ASSERT_EQ_INT(0, monster_breeding_count());
    ASSERT_TRUE(monster_breeding_allowed());
}

/* ------------------------------------------------------------------
 * 階の頭 -- 0 に戻る
 * ------------------------------------------------------------------ */

TEST(a_new_level_starts_with_nothing_bred) {
    given_a_fresh_level();

    ASSERT_EQ_INT(0, monster_breeding_count());
    ASSERT_TRUE(monster_breeding_allowed());
}

/* **使いきった予算も階を降りれば戻る。** これが「階ごと」の意味そのもので、
 * 戻す 1 行を消すと**1 回の冒険で 76 体しか増えなくなる**。 */
TEST(leaving_the_level_gives_the_whole_budget_back) {
    given_the_budget_spent();
    ASSERT_FALSE(monster_breeding_allowed());

    monster_breeding_reset();

    ASSERT_EQ_INT(0, monster_breeding_count());
    ASSERT_TRUE(monster_breeding_allowed());
}

/* ------------------------------------------------------------------
 * 出産 -- 1 体で 1 つ使う
 * ------------------------------------------------------------------ */

TEST(a_birth_spends_one) {
    given_a_fresh_level();

    monster_breeding_note_birth();

    ASSERT_EQ_INT(1, monster_breeding_count());
}

TEST(births_add_up) {
    given_a_fresh_level();

    for (int i = 1; i <= 10; i++) {
        monster_breeding_note_birth();

        ASSERT_EQ_INT(i, monster_breeding_count());
    }
}

/* ------------------------------------------------------------------
 * 上限 -- 境目は「上限と等しいとき」
 * ------------------------------------------------------------------ */

/* **上限と等しくてもまだ増やしてよい。** 比較は `MAX_MON_MULT >= 数` で、
 * 等しいときは真 —— **ここを `>` にすると 1 体ぶん少なくなる。** */
TEST(the_cap_itself_still_allows_one_more) {
    set_monster_breeding_count(MAX_MON_MULT);

    ASSERT_TRUE(monster_breeding_allowed());
}

/* 1 つ越えたら止まる。 */
TEST(one_past_the_cap_stops_the_breeding) {
    set_monster_breeding_count(MAX_MON_MULT + 1);

    ASSERT_FALSE(monster_breeding_allowed());
}

/* 境目を 1 か所にまとめて見る。 */
TEST(the_line_falls_just_above_the_cap) {
    set_monster_breeding_count(MAX_MON_MULT - 1);
    ASSERT_TRUE(monster_breeding_allowed());

    set_monster_breeding_count(MAX_MON_MULT);
    ASSERT_TRUE(monster_breeding_allowed());

    set_monster_breeding_count(MAX_MON_MULT + 1);
    ASSERT_FALSE(monster_breeding_allowed());
}

/* **許される出産は MAX_MON_MULT + 1 回 = 76 回。** 上限 75 と 1 つずれて
 * いるのが上流のふるまいで、数えなおすとここに出る。 */
TEST(the_budget_allows_one_more_birth_than_the_cap) {
    given_a_fresh_level();

    int births = 0;
    while (monster_breeding_allowed()) {
        monster_breeding_note_birth();
        births++;
    }

    ASSERT_EQ_INT(MAX_MON_MULT + 1, births);
    ASSERT_EQ_INT(76, births);
    ASSERT_EQ_INT(76, monster_breeding_count());
}

/* 尽きたあとは何度訊いても止まったまま（訊くだけでは戻らない）。 */
TEST(the_budget_stays_spent_however_often_it_is_asked) {
    given_the_budget_spent();

    for (int i = 0; i < 100; i++) {
        ASSERT_FALSE(monster_breeding_allowed());
    }

    ASSERT_EQ_INT(76, monster_breeding_count());
}

/* ------------------------------------------------------------------
 * 死 -- 1 つ返る（生まれた子かは訊かない）
 * ------------------------------------------------------------------ */

TEST(a_death_gives_one_back) {
    given_a_fresh_level();
    monster_breeding_note_birth();
    monster_breeding_note_birth();

    monster_breeding_note_death();

    ASSERT_EQ_INT(1, monster_breeding_count());
}

/* **尽きた予算が死 1 つで開く。** 増えるモンスターを 1 体倒すと、
 * その階でまた 1 体増やせるようになる。 */
TEST(a_death_reopens_a_spent_budget) {
    given_the_budget_spent();
    ASSERT_FALSE(monster_breeding_allowed());

    monster_breeding_note_death();

    ASSERT_TRUE(monster_breeding_allowed());
    ASSERT_EQ_INT(MAX_MON_MULT, monster_breeding_count());
}

/* **1 体も生まれていない階でも死は 1 つ返そうとする** —— そして 0 で止まる。
 * **これが上の②** で、返るのは「生まれた子が死んだから」ではなく
 * 「遅れて消える道を通ったから」。数が 0 のまま動かないことが、
 * **手で置かれたモンスターを倒しても予算が増えない**ことになっている。 */
TEST(deaths_never_take_the_count_below_nothing) {
    given_a_fresh_level();

    for (int i = 0; i < 100; i++) {
        monster_breeding_note_death();
    }

    ASSERT_EQ_INT(0, monster_breeding_count());
    ASSERT_TRUE(monster_breeding_allowed());
}

/* 0 で止まる床は 1 体ぶんだけ下にある（1 → 0 は返り、0 → −1 は返らない）。 */
TEST(the_floor_sits_just_below_one) {
    given_a_fresh_level();
    monster_breeding_note_birth();

    monster_breeding_note_death();
    ASSERT_EQ_INT(0, monster_breeding_count());

    monster_breeding_note_death();
    ASSERT_EQ_INT(0, monster_breeding_count());
}

/* 出産と死を混ぜても数は素直に上下する。 */
TEST(births_and_deaths_move_the_count_both_ways) {
    given_a_fresh_level();

    for (int i = 0; i < 20; i++) {
        monster_breeding_note_birth();
    }
    ASSERT_EQ_INT(20, monster_breeding_count());

    for (int i = 0; i < 5; i++) {
        monster_breeding_note_death();
    }
    ASSERT_EQ_INT(15, monster_breeding_count());

    monster_breeding_note_birth();
    ASSERT_EQ_INT(16, monster_breeding_count());
}

/* ------------------------------------------------------------------
 * セーブファイル -- 使った予算は残る
 * ------------------------------------------------------------------ */

/* 入れた数がそのまま出てくる（丸めも囲いもしない。missile_serial と同じ形）。 */
TEST(the_count_that_goes_in_is_the_one_that_comes_out) {
    given_a_fresh_level();

    const int16_t values[] = {0, 1, 7, MAX_MON_MULT, MAX_MON_MULT + 1, 1000};
    for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
        set_monster_breeding_count(values[i]);

        ASSERT_EQ_INT(values[i], monster_breeding_count());
    }
}

/* 戻す窓口は数だけでなく答えも変える（上限を越えた数を入れれば止まる）。
 * **ただしゲームの中ではこの効果は見えない** —— 読みもどした直後に
 * dungeon() が 0 に戻すので、**ファイルの中のこの short は誰も読まない枠**。
 * それでも窓口の対を残しているのはファイル書式を動かせないからで、
 * ここで押さえているのは**入れた数がそのまま効く**ということだけ。 */
TEST(putting_a_count_back_can_stop_the_breeding) {
    given_the_budget_spent();
    const int16_t saved = monster_breeding_count();

    monster_breeding_reset(); /* dungeon() が階の頭でするのと同じ */
    ASSERT_TRUE(monster_breeding_allowed());

    set_monster_breeding_count(saved);

    ASSERT_EQ_INT(saved, monster_breeding_count());
    ASSERT_FALSE(monster_breeding_allowed());
}

/* ------------------------------------------------------------------
 * 読みは副作用を持たない
 * ------------------------------------------------------------------ */

TEST(asking_does_not_change_the_answer) {
    given_a_fresh_level();
    monster_breeding_note_birth();

    for (int i = 0; i < 100; i++) {
        (void)monster_breeding_allowed();
        (void)monster_breeding_count();
    }

    ASSERT_EQ_INT(1, monster_breeding_count());
}

int main(void) {
    /* 足場を呼ばない 1 件。**この行より前に何も足さないこと**（上の註）。 */
    RUN_TEST(nothing_has_been_bred_before_anything_happens);

    RUN_TEST(a_new_level_starts_with_nothing_bred);
    RUN_TEST(leaving_the_level_gives_the_whole_budget_back);

    RUN_TEST(a_birth_spends_one);
    RUN_TEST(births_add_up);

    RUN_TEST(the_cap_itself_still_allows_one_more);
    RUN_TEST(one_past_the_cap_stops_the_breeding);
    RUN_TEST(the_line_falls_just_above_the_cap);
    RUN_TEST(the_budget_allows_one_more_birth_than_the_cap);
    RUN_TEST(the_budget_stays_spent_however_often_it_is_asked);

    RUN_TEST(a_death_gives_one_back);
    RUN_TEST(a_death_reopens_a_spent_budget);
    RUN_TEST(deaths_never_take_the_count_below_nothing);
    RUN_TEST(the_floor_sits_just_below_one);
    RUN_TEST(births_and_deaths_move_the_count_both_ways);

    RUN_TEST(the_count_that_goes_in_is_the_one_that_comes_out);
    RUN_TEST(putting_a_count_back_can_stop_the_breeding);

    RUN_TEST(asking_does_not_change_the_answer);

    return TEST_SUMMARY();
}
