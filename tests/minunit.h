/* minunit.h -- 依存ゼロの最小 xUnit 実装
 *
 * 外部ライブラリを足さずに単体テストを始めるための足場。
 * 提供するのは xUnit の役割そのまま：テストを実装する API と、実行して
 * グリーン／レッドを報告する仕組み。
 *
 * 使い方:
 *   TEST(name) { ASSERT_EQ_INT(actual, expected); }
 *   int main(void) { RUN_TEST(name); return TEST_SUMMARY(); }
 */
#ifndef MINUNIT_H
#define MINUNIT_H

#include <stdio.h>
#include <string.h>

static int mu_tests_run = 0;
static int mu_tests_failed = 0;
static int mu_current_failed = 0;

/* テスト関数の定義。名前が assertion の説明になるように書く。 */
#define TEST(name) static void test_##name(void)

/* 1 テストにつき assertion はひとつ。失敗した assert 以降は実行されない
 * ので、複数あると原因の切り分けに何往復もかかる。 */
#define MU_FAIL(fmt, ...)                                                      \
    do {                                                                       \
        printf("  FAIL %s:%d: " fmt "\n", __FILE__, __LINE__, __VA_ARGS__);    \
        mu_current_failed = 1;                                                 \
        return;                                                                \
    } while (0)

#define ASSERT_EQ_INT(actual, expected)                                        \
    do {                                                                       \
        long mu_a = (long)(actual), mu_e = (long)(expected);                   \
        if (mu_a != mu_e)                                                      \
            MU_FAIL("%s == %s: actual %ld, expected %ld", #actual, #expected,  \
                    mu_a, mu_e);                                               \
    } while (0)

#define ASSERT_EQ_STR(actual, expected)                                        \
    do {                                                                       \
        const char *mu_a = (actual), *mu_e = (expected);                       \
        if (mu_a == NULL || mu_e == NULL || strcmp(mu_a, mu_e) != 0)           \
            MU_FAIL("%s == %s: actual \"%s\", expected \"%s\"", #actual,       \
                    #expected, mu_a ? mu_a : "(null)",                         \
                    mu_e ? mu_e : "(null)");                                   \
    } while (0)

#define ASSERT_TRUE(cond)                                                      \
    do {                                                                       \
        if (!(cond)) MU_FAIL("%s is false", #cond);                            \
    } while (0)

#define ASSERT_FALSE(cond)                                                     \
    do {                                                                       \
        if (cond) MU_FAIL("%s is true", #cond);                                \
    } while (0)

/* 疎通確認用。レッドが正しく報告されることを確かめるために使い、
 * 確認後は削除する。 */
#define ASSERT_FAIL(msg) MU_FAIL("%s", msg)

/* setUp / tearDown があれば各テストの前後で呼ぶ。先行テストの成否に
 * よらずテストごとに呼ばれるので、条件が毎回揃う。 */
#ifndef MU_SETUP
#define MU_SETUP() ((void)0)
#endif
#ifndef MU_TEARDOWN
#define MU_TEARDOWN() ((void)0)
#endif

#define RUN_TEST(name)                                                         \
    do {                                                                       \
        mu_current_failed = 0;                                                 \
        mu_tests_run++;                                                        \
        MU_SETUP();                                                            \
        test_##name();                                                         \
        MU_TEARDOWN();                                                         \
        if (mu_current_failed) {                                               \
            mu_tests_failed++;                                                 \
            printf("[FAIL] %s\n", #name);                                      \
        } else {                                                               \
            printf("[ ok ] %s\n", #name);                                      \
        }                                                                      \
    } while (0)

/* 実行結果の要約。テスト件数を必ず表示する。0 件なのにグリーンと
 * 誤認する事故を防ぐため。 */
static int mu_summary(void)
{
    printf("\n%d tests, %d passed, %d failed\n", mu_tests_run,
           mu_tests_run - mu_tests_failed, mu_tests_failed);
    if (mu_tests_run == 0) {
        printf("WARNING: テストが 1 件も実行されていません（保護は存在しない）\n");
        return 1;
    }
    return mu_tests_failed == 0 ? 0 : 1;
}

#define TEST_SUMMARY() mu_summary()

#endif /* MINUNIT_H */
