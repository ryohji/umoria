/* モンスター表の逆順走査のテスト -- 現在のふるまいを保護する
 *
 * monsters.c の monster_creature_rbegin() / rend() / prev() は、
 * c_list（モンスター定義表）を末尾から先頭へたどるための三つ組。
 * 呼びだしは help.c（モンスター図鑑）と main.c（レベル別の頭数を数える）の
 * 2 箇所で、どちらも順序が結果に出る。
 *
 * rend() が返す c_list - 1 は配列の直前を指すポインタで、C17 6.5.6p8 が
 * 定義しているのは「同じ配列の要素、または末尾の 1 つ先」だけなので、
 * 参照しなくても値を作った時点で未定義動作。これを直すので、その前に
 * 「どの要素をどの順にたどるか」を押さえる。
 *
 * 走査そのものを collect_reverse_walk() の 1 箇所に閉じこめてある。
 * 三つ組の形が変わってもここだけを書きかえれば済み、期待値は動かない。
 * 期待値の側は creature_handle 経由（monster_make_creature_handle /
 * monster_get_creature）で書いてあり、こちらは今回変えない API。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 検証に使う本物（src/monsters.c）。externs.h は本体の宣言をまるごと
 * 引きこむので、必要なものだけをここに書く。 */
creature_handle monster_make_creature_handle(uint16_t index);
creature_type *monster_get_creature(creature_handle h);
creature_type *monster_creature_rbegin(void);
creature_type *monster_creature_rend(void);
creature_type *monster_creature_prev(creature_type *p);

#include "minunit.h"

/* 逆順走査の結果を並べて返す。走査の書きかたに触るのはここだけ。
 * 戻り値はたどった数。 */
#define WALK_LIMIT (MAX_CREATURES * 2)
static creature_type *walk[WALK_LIMIT];

static int collect_reverse_walk(void)
{
    int n = 0;

    creature_type *it, *const end = monster_creature_rend();
    for (it = monster_creature_rbegin(); it != end; it = monster_creature_prev(it)) {
        if (n < WALK_LIMIT) {
            walk[n] = it;
        }
        n++;
    }

    return n;
}

/* 期待値の側。添字から要素を得る経路は今回の変更対象ではない。 */
static creature_type *creature_at(int index)
{
    return monster_get_creature(monster_make_creature_handle((uint16_t)index));
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

int main(void)
{
    RUN_TEST(reverse_walk_visits_every_creature_once);
    RUN_TEST(reverse_walk_starts_at_the_last_creature);
    RUN_TEST(reverse_walk_ends_at_the_first_creature);
    RUN_TEST(reverse_walk_steps_one_index_back_at_a_time);
    RUN_TEST(reverse_walk_reads_the_last_creature_name_first);
    return TEST_SUMMARY();
}

/* --- スタブ ---
 * monsters.c が名前を組みたてるときに使う（src/desc.c の本物）。desc.c を
 * リンクすると定数表とインベントリまで芋づるで付いてくるので代役を置く。
 * 走査の順序を見るテストでは呼ばれない。 */
bool is_a_vowel(char ch) { (void)ch; return false; }
