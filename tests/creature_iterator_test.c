// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* モンスター表の逆順・正順走査のテスト -- 現在のふるまいを保護する
 *
 * monsters.c の monster_creature_rbegin() / rend() / rnext() / rget() は、
 * c_list（モンスター定義表）を末尾から先頭へたどるための組。
 * 呼びだしは help.c（モンスター図鑑）と app/main.c（レベル別の頭数を数える）の
 * 2 箇所で、どちらも順序が結果に出る。
 *
 * 以前の rend() は c_list - 1、つまり配列の直前を指すポインタを返していた。
 * C17 6.5.6p8 が定義しているのは「同じ配列の要素、または末尾の 1 つ先」だけ
 * なので、参照しなくても値を作った時点で未定義動作（-Warray-bounds が指して
 * いたのはこれ）。これを直すのに先だって、このテストで「どの要素をどの順に
 * たどるか」を押さえた。
 *
 * 走査そのものを collect_reverse_walk() の 1 箇所に閉じこめてある。
 * API の形が変わってもここだけを書きかえれば済み、期待値は動かない。
 * 期待値の側は creature_iterator 経由（monster_make_creature_handle /
 * monster_get_creature）で書いてあり、こちらは変えていない API。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 検証に使う本物（src/model/data/monsters.c）。 */
#include "monsters.h"

#include "minunit.h"

/* 逆順走査の結果を並べて返す。走査の書きかたに触るのはここだけ。
 * 戻り値はたどった数。 */
#define WALK_LIMIT (MAX_CREATURES * 2)
static creature_type *walk[WALK_LIMIT];

static int collect_reverse_walk(void)
{
    int n = 0;

    const creature_rev_iterator end = monster_creature_rend();
    for (creature_rev_iterator it = monster_creature_rbegin(); !monster_creature_rsame(it, end); it = monster_creature_rnext(it)) {
        if (n < WALK_LIMIT) {
            walk[n] = monster_creature_rget(it);
        }
        n++;
    }

    return n;
}

/* 正順走査の結果を並べて返す。逆順と同じく、走査の書きかたに触るのはここだけ。 */
static int collect_forward_walk(void)
{
    int n = 0;

    const creature_iterator end = monster_creature_end();
    for (creature_iterator it = monster_creature_begin(); !monster_creature_same(it, end); it = monster_creature_next(it)) {
        if (n < WALK_LIMIT) {
            walk[n] = monster_creature_get(it);
        }
        n++;
    }

    return n;
}

/* 期待値の側。添字から要素を得る経路は今回の変更対象ではない。 */
static creature_type *creature_at(int index)
{
    return monster_creature_get(monster_creature_at((uint16_t)index));
}

TEST(reverse_walk_visits_every_creature_once)
{
    ASSERT_EQ_INT(collect_reverse_walk(), MAX_CREATURES);
}

TEST(reverse_walk_starts_at_the_last_creature)
{
    (void)collect_reverse_walk();
    ASSERT_TRUE(walk[0] == creature_at(MAX_CREATURES - 1));
}

TEST(reverse_walk_ends_at_the_first_creature)
{
    int n = collect_reverse_walk();
    ASSERT_TRUE(walk[n - 1] == creature_at(0));
}

TEST(reverse_walk_steps_one_index_back_at_a_time)
{
    int n = collect_reverse_walk();

    int mismatch = -1;
    for (int i = 0; i < n; i++) {
        if (walk[i] != creature_at(MAX_CREATURES - 1 - i)) {
            mismatch = i;
            break;
        }
    }

    ASSERT_EQ_INT(mismatch, -1);
}

TEST(reverse_walk_reads_the_last_creature_name_first)
{
    /* ポインタの一致だけだと、指す先が読めているかは見ていない。
     * 実際に中身を読んで、末尾の要素から始まっていることを確かめる。 */
    (void)collect_reverse_walk();
    ASSERT_EQ_STR(walk[0]->name, creature_at(MAX_CREATURES - 1)->name);
}

TEST(forward_walk_visits_every_creature_once)
{
    ASSERT_EQ_INT(collect_forward_walk(), MAX_CREATURES);
}

TEST(forward_walk_starts_at_the_first_creature)
{
    (void)collect_forward_walk();
    ASSERT_TRUE(walk[0] == creature_at(0));
}

TEST(forward_walk_ends_at_the_last_creature)
{
    int n = collect_forward_walk();
    ASSERT_TRUE(walk[n - 1] == creature_at(MAX_CREATURES - 1));
}

TEST(forward_walk_steps_one_index_on_at_a_time)
{
    int n = collect_forward_walk();

    int mismatch = -1;
    for (int i = 0; i < n; i++) {
        if (walk[i] != creature_at(i)) {
            mismatch = i;
            break;
        }
    }

    ASSERT_EQ_INT(mismatch, -1);
}

TEST(forward_walk_gives_back_the_handle_of_each_creature)
{
    /* save.c は正順にたどって、要素から添字（handle）を取りもどして書く。 */
    int n = collect_forward_walk();

    int mismatch = -1;
    for (int i = 0; i < n; i++) {
        if (monster_get_creature_handle(walk[i]).place != i) {
            mismatch = i;
            break;
        }
    }

    ASSERT_EQ_INT(mismatch, -1);
}

TEST(advance_from_begin_reaches_every_index)
{
    const creature_iterator begin = monster_creature_begin();

    int mismatch = -1;
    for (int i = 0; i < MAX_CREATURES; i++) {
        if (monster_creature_get(monster_creature_advance(begin, i)) != creature_at(i)) {
            mismatch = i;
            break;
        }
    }

    ASSERT_EQ_INT(mismatch, -1);
}

TEST(advance_by_the_table_size_reaches_end)
{
    ASSERT_TRUE(monster_creature_same(monster_creature_advance(monster_creature_begin(), MAX_CREATURES),
                                      monster_creature_end()));
}

TEST(advance_backwards_undoes_advance)
{
    const creature_iterator it = monster_creature_advance(monster_creature_begin(), 100);
    ASSERT_TRUE(monster_creature_get(monster_creature_advance(it, -37)) == creature_at(63));
}

TEST(distance_counts_entries_between)
{
    const creature_iterator a = monster_creature_advance(monster_creature_begin(), 10);
    const creature_iterator b = monster_creature_advance(monster_creature_begin(), 25);
    ASSERT_EQ_INT(monster_creature_distance(a, b), 15);
}

TEST(distance_is_negative_backwards)
{
    const creature_iterator a = monster_creature_advance(monster_creature_begin(), 10);
    const creature_iterator b = monster_creature_advance(monster_creature_begin(), 25);
    ASSERT_EQ_INT(monster_creature_distance(b, a), -15);
}

TEST(distance_from_begin_to_end_is_the_table_size)
{
    ASSERT_EQ_INT(monster_creature_distance(monster_creature_begin(), monster_creature_end()), MAX_CREATURES);
}

TEST(at_reaches_the_entry_of_every_index)
{
    int mismatch = -1;
    for (int i = 0; i < MAX_CREATURES; i++) {
        if (monster_creature_get(monster_creature_at(i)) != creature_at(i)) {
            mismatch = i;
            break;
        }
    }
    ASSERT_EQ_INT(mismatch, -1);
}

TEST(index_gives_back_what_at_was_given)
{
    int mismatch = -1;
    for (int i = 0; i <= MAX_CREATURES; i++) {
        if (monster_creature_index(monster_creature_at(i)) != i) {
            mismatch = i;
            break;
        }
    }
    ASSERT_EQ_INT(mismatch, -1);
}

TEST(at_the_table_size_is_end)
{
    ASSERT_TRUE(monster_creature_same(monster_creature_at(MAX_CREATURES), monster_creature_end()));
}

TEST(from_reverse_refers_to_the_same_entry)
{
    int n = 0;
    int mismatch = -1;
    const creature_rev_iterator end = monster_creature_rend();
    for (creature_rev_iterator it = monster_creature_rbegin(); !monster_creature_rsame(it, end); it = monster_creature_rnext(it)) {
        if (monster_creature_get(monster_creature_from_reverse(it)) != monster_creature_rget(it) && mismatch < 0) {
            mismatch = n;
        }
        n++;
    }
    ASSERT_EQ_INT(mismatch, -1);
}

int main(void)
{
    RUN_TEST(reverse_walk_visits_every_creature_once);
    RUN_TEST(reverse_walk_starts_at_the_last_creature);
    RUN_TEST(reverse_walk_ends_at_the_first_creature);
    RUN_TEST(reverse_walk_steps_one_index_back_at_a_time);
    RUN_TEST(reverse_walk_reads_the_last_creature_name_first);
    RUN_TEST(forward_walk_visits_every_creature_once);
    RUN_TEST(forward_walk_starts_at_the_first_creature);
    RUN_TEST(forward_walk_ends_at_the_last_creature);
    RUN_TEST(forward_walk_steps_one_index_on_at_a_time);
    RUN_TEST(forward_walk_gives_back_the_handle_of_each_creature);
    RUN_TEST(advance_from_begin_reaches_every_index);
    RUN_TEST(advance_by_the_table_size_reaches_end);
    RUN_TEST(advance_backwards_undoes_advance);
    RUN_TEST(distance_counts_entries_between);
    RUN_TEST(distance_is_negative_backwards);
    RUN_TEST(distance_from_begin_to_end_is_the_table_size);
    RUN_TEST(at_reaches_the_entry_of_every_index);
    RUN_TEST(index_gives_back_what_at_was_given);
    RUN_TEST(at_the_table_size_is_end);
    RUN_TEST(from_reverse_refers_to_the_same_entry);
    return TEST_SUMMARY();
}

/* --- スタブ ---
 * monsters.c が名前を組みたてるときに使う（src/world/item/desc.c の本物）。desc.c を
 * リンクすると定数表とインベントリまで芋づるで付いてくるので代役を置く。
 * 走査の順序を見るテストでは呼ばれない。 */
bool is_a_vowel(char ch) { (void)ch; return false; }
