// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* セーブファイルの版の比較のテスト -- 置きかえる前の式と同じ答えを返すこと
 *
 * get_char() は読んだ版（5.<version_min>.<patch_level>）で読む項目を変える。
 * その条件を savefile_before() 1 本に寄せた。どの条件も 2 つの uint8_t
 * だけで答えが決まり副作用も無いので、置きかえる前の式（下の legacy_*。
 * 変更前の save.c の字面の写し）と 256 × 256 の全組で突きあわせれば、
 * 置きかえが同値であることの証明になる。
 *
 * savefile_before() は save.c の static 関数なので、save.c を #include する
 * （save_bool_test と同じ手）。
 */
#include "save.c"
#include "minunit.h"

/* 変更前の save.c の条件の写し。行は置きかえる前のもの */
static bool legacy_too_old(int version_min, int patch_level) /* :586 */
{
    return (version_min == 0 && patch_level < 14);
}

static bool legacy_before_5_2_2(int version_min, int patch_level) /* :610, :624 */
{
    return ((version_min < 2) || (version_min == 2 && patch_level < 2));
}

static bool legacy_from_5_1_3(int version_min, int patch_level) /* :950, :958 */
{
    return ((version_min >= 2) || (version_min == 1 && patch_level >= 3));
}

static bool legacy_from_5_2_2(int version_min, int patch_level) /* :966, :977 */
{
    return ((version_min >= 3) || (version_min == 2 && patch_level >= 2));
}

static bool legacy_before_5_1_3(int version_min, int patch_level) /* :1141 */
{
    return ((version_min == 1 && patch_level < 3) || (version_min == 0));
}

static bool legacy_no_time_saved(int version_min, int patch_level) /* :1150 */
{
    return (version_min == 0 && patch_level < 16);
}

/* 全組で legacy と new の答えが食いちがった数 */
static int mismatches(bool (*legacy)(int, int), uint8_t min, uint8_t patch, bool negate)
{
    int mismatch = 0;
    for (int v = 0; v < 256; v++) {
        for (int p = 0; p < 256; p++) {
            bool now = savefile_before((uint8_t)v, (uint8_t)p, min, patch);
            if (negate) {
                now = !now;
            }
            if (now != legacy(v, p)) {
                mismatch++;
            }
        }
    }
    return mismatch;
}

TEST(too_old_matches_before_5_0_14)
{
    ASSERT_EQ_INT(0, mismatches(legacy_too_old, 0, 14, false));
}

TEST(before_5_2_2_matches)
{
    ASSERT_EQ_INT(0, mismatches(legacy_before_5_2_2, 2, 2, false));
}

TEST(from_5_1_3_matches_not_before_5_1_3)
{
    ASSERT_EQ_INT(0, mismatches(legacy_from_5_1_3, 1, 3, true));
}

TEST(from_5_2_2_matches_not_before_5_2_2)
{
    ASSERT_EQ_INT(0, mismatches(legacy_from_5_2_2, 2, 2, true));
}

TEST(before_5_1_3_matches)
{
    ASSERT_EQ_INT(0, mismatches(legacy_before_5_1_3, 1, 3, false));
}

TEST(no_time_saved_matches_before_5_0_16)
{
    ASSERT_EQ_INT(0, mismatches(legacy_no_time_saved, 0, 16, false));
}

int main(void)
{
    RUN_TEST(too_old_matches_before_5_0_14);
    RUN_TEST(before_5_2_2_matches);
    RUN_TEST(from_5_1_3_matches_not_before_5_1_3);
    RUN_TEST(from_5_2_2_matches_not_before_5_2_2);
    RUN_TEST(before_5_1_3_matches);
    RUN_TEST(no_time_saved_matches_before_5_0_16);
    return TEST_SUMMARY();
}
