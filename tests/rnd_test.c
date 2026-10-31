// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 乱数（src/core/rnd.c）のテスト -- 現在の実装を保護する
 *
 * ほかのテストはすべて代役の randint()（shared_stubs.c）を使うので、本物の
 * 乱数はこのテストだけが通す。守りたいのは 3 つ：
 *   - 列そのもの。町の並び（generate.c の set_seed）と品物の色（desc.c）は
 *     種から作りなおすので、列が変わるとセーブを読んだときの町と色が変わる。
 *   - set_seed() → reset_seed() で元の列へ戻る手順。
 *   - randint・randnor・magik が rnd() の値をどう使うか。
 *
 * 期待値はどれも、いまの実装が返した値か、rnd() の列から手で導いた値。
 * randnor(mean, 0) は平均をそのまま返す（バグ候補 B29 を直した。前は表の外の
 * 枝で randint(0) が 0 で割った）。
 * 呼び手はどれも 0 を渡さない。
 */
#include "rnd.h"
#include "minunit.h"

#define RNG_M 2147483647L

/* rnd.c の註にある自己検査：z[1] = 1 から 10000 回進めると 1043618065 */
TEST(rnd_from_one_reaches_the_published_value_at_step_10001)
{
    set_rnd_seed(0);
    int32_t z = 0;
    for (int i = 0; i < 10000; i++) {
        z = rnd();
    }
    ASSERT_EQ_INT(z, 1043618065);
}

/* 種は 1〜m-1 に畳まれる。0 と m-1 はどちらも z = 1 から始まる */
TEST(set_rnd_seed_folds_zero_and_m_minus_one_to_the_same_start)
{
    set_rnd_seed(0);
    int32_t a = rnd();
    set_rnd_seed((uint32_t)(RNG_M - 1));
    int32_t b = rnd();
    ASSERT_TRUE(a == 16807 && b == 16807);
}

/* 種 m-2 は z = m-1 になり、次は 16807 * (m-1) mod m = m - 16807 */
TEST(set_rnd_seed_of_m_minus_two_starts_from_the_top)
{
    set_rnd_seed((uint32_t)(RNG_M - 2));
    ASSERT_EQ_INT(rnd(), (int32_t)(RNG_M - 16807));
}

/* 値は 1〜m-1 に収まる */
TEST(rnd_stays_between_one_and_m_minus_one)
{
    set_rnd_seed(12345);
    int out_of_range = 0;
    for (int i = 0; i < 100000; i++) {
        int32_t z = rnd();
        if (z < 1 || z > RNG_M - 1) {
            out_of_range++;
        }
    }
    ASSERT_EQ_INT(out_of_range, 0);
}

/* set_seed(s) のあとの列は set_rnd_seed(s) のあとの列と同じ（町の並びが
 * 毎回同じになる理由） */
TEST(set_seed_gives_the_same_stream_as_set_rnd_seed)
{
    int32_t expected[5];
    set_rnd_seed(777);
    for (int i = 0; i < 5; i++) {
        expected[i] = rnd();
    }

    set_rnd_seed(1);
    (void)rnd();
    set_seed(777);
    int mismatch = 0;
    for (int i = 0; i < 5; i++) {
        if (rnd() != expected[i]) {
            mismatch++;
        }
    }
    reset_seed();
    ASSERT_EQ_INT(mismatch, 0);
}

/* reset_seed() は覚えた z を種として set_rnd_seed() に渡しなおすので、
 * 戻った列は z + 1 から始まる。set_rnd_seed(123) の z は 124 なので、
 * 戻った先は set_rnd_seed(124) の列 */
TEST(reset_seed_resumes_from_the_saved_state_plus_one)
{
    set_rnd_seed(124);
    int32_t expected = rnd();

    set_rnd_seed(123);
    set_seed(9);
    (void)rnd();
    (void)rnd();
    reset_seed();
    ASSERT_EQ_INT(rnd(), expected);
}

/* randint(n) は rnd() % n + 1 */
TEST(randint_is_rnd_modulo_maxval_plus_one)
{
    static const int maxvals[] = {1, 2, 3, 6, 100, 32767, 2147483647};
    int mismatch = 0;
    for (size_t k = 0; k < sizeof maxvals / sizeof maxvals[0]; k++) {
        set_rnd_seed(2024);
        int32_t z[50];
        for (int i = 0; i < 50; i++) {
            z[i] = rnd();
        }
        set_rnd_seed(2024);
        for (int i = 0; i < 50; i++) {
            if (randint(maxvals[k]) != (int)(z[i] % maxvals[k]) + 1) {
                mismatch++;
            }
        }
    }
    ASSERT_EQ_INT(mismatch, 0);
}

/* 小さい上限では 1〜n のどれもが出て、外は出ない */
TEST(randint_of_six_covers_one_to_six_only)
{
    int seen[8] = {0};
    set_rnd_seed(5);
    for (int i = 0; i < 6000; i++) {
        int v = randint(6);
        seen[(v >= 1 && v <= 6) ? v : 0]++;
    }
    int missing = 0;
    for (int v = 1; v <= 6; v++) {
        if (seen[v] == 0) {
            missing++;
        }
    }
    ASSERT_TRUE(seen[0] == 0 && missing == 0);
}

/* randnor の最初の 8 個（種 42、平均 100、偏差 10） */
TEST(randnor_first_values_from_seed_42)
{
    static const int expected[8] = {99, 101, 106, 100, 104, 103, 110, 101};
    set_rnd_seed(42);
    int mismatch = 0;
    for (int i = 0; i < 8; i++) {
        if (randnor(100, 10) != expected[i]) {
            mismatch++;
        }
    }
    ASSERT_EQ_INT(mismatch, 0);
}

/* 偏差 0 なら、乱数を引かずに平均を返す。 */
TEST(randnor_with_no_spread_returns_the_mean)
{
    set_rnd_seed(42);
    int mean = randnor(100, 0);
    ASSERT_EQ_INT(mean, 100);
}

/* 偏差 0 は乱数を引かない：次の randnor(100, 10) は種 42 の最初の値。 */
TEST(randnor_with_no_spread_draws_nothing)
{
    set_rnd_seed(42);
    (void)randnor(100, 0);
    ASSERT_EQ_INT(randnor(100, 10), 99);
}

/* 10 万個の和と両端（種 42、平均 0、偏差 64＝表の偏差）。表の 1 つの値や
 * 二分探索の 1 歩が変われば、どこかで変わる */
TEST(randnor_sum_and_extremes_over_100000_draws)
{
    set_rnd_seed(42);
    long sum = 0;
    int lo = 0;
    int hi = 0;
    for (int i = 0; i < 100000; i++) {
        int v = randnor(0, 64);
        sum += v;
        if (v < lo) {
            lo = v;
        }
        if (v > hi) {
            hi = v;
        }
    }
    ASSERT_TRUE(sum == -8299 && lo == -316 && hi == 305);
}

/* 表の外の枝：最初の randint(MAX_SHORT) が MAX_SHORT になる種 131067 では、
 * 偏差の 4 倍に randint(stand) を足す。平均 1000、偏差 10 で 1043 */
TEST(randnor_off_the_table_adds_four_to_five_deviations)
{
    set_rnd_seed(131067);
    ASSERT_EQ_INT(randnor(1000, 10), 1043);
}

/* magik(c) は randint(100) <= c。c = 0 は決して真にならず、100 は必ず真 */
TEST(magik_is_randint_100_at_most_chance)
{
    int mismatch = 0;
    for (int chance = 0; chance <= 100; chance += 5) {
        set_rnd_seed((uint32_t)(chance + 1));
        int expected[40];
        for (int i = 0; i < 40; i++) {
            expected[i] = randint(100) <= chance;
        }
        set_rnd_seed((uint32_t)(chance + 1));
        for (int i = 0; i < 40; i++) {
            if (magik(chance) != expected[i]) {
                mismatch++;
            }
        }
    }
    ASSERT_EQ_INT(mismatch, 0);
}

int main(void)
{
    RUN_TEST(rnd_from_one_reaches_the_published_value_at_step_10001);
    RUN_TEST(set_rnd_seed_folds_zero_and_m_minus_one_to_the_same_start);
    RUN_TEST(set_rnd_seed_of_m_minus_two_starts_from_the_top);
    RUN_TEST(rnd_stays_between_one_and_m_minus_one);
    RUN_TEST(set_seed_gives_the_same_stream_as_set_rnd_seed);
    RUN_TEST(reset_seed_resumes_from_the_saved_state_plus_one);
    RUN_TEST(randint_is_rnd_modulo_maxval_plus_one);
    RUN_TEST(randint_of_six_covers_one_to_six_only);
    RUN_TEST(randnor_first_values_from_seed_42);
    RUN_TEST(randnor_with_no_spread_returns_the_mean);
    RUN_TEST(randnor_with_no_spread_draws_nothing);
    RUN_TEST(randnor_sum_and_extremes_over_100000_draws);
    RUN_TEST(randnor_off_the_table_adds_four_to_five_deviations);
    RUN_TEST(magik_is_randint_100_at_most_chance);
    return TEST_SUMMARY();
}
