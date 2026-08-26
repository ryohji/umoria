/* 魔法道具（杖・魔法棒）の使用成功判定のテスト -- 現在の実装を保護する
 *
 * staffs.c:35-46（杖）と wands.c:42-54（魔法棒）に同じ chance 計算が
 * 重複している。違いは杖側の定数 -5 のペナルティだけ。
 * この重複を将来ひとつに抽出するときの回帰検知としてテストで固定する。
 *
 * staffs.c / wands.c をそのままリンクすると msg_print・inventory・py など
 * 画面表示とグローバル状態への依存が芋づるで付いてくるので、
 * distance_test.c と同じ方式で、chance 計算部分だけをこのファイルに写す。
 *
 * 乱数の切りはなし：本体は misc1.c:59 の randint() を 2 回呼び、
 * グローバルな乱数状態を使うので結果が再現しない。写した実装では
 * randint を関数ポインター（randint_fn）で受けとる形にし、
 * テストから返す値を固定する。本体のコードは変更していない。
 *
 * 期待値はすべてこの実装が返した実際の値。仕様書はないので、
 * いまどう判定されるかを固定することが目的。
 */

/* ------------------------------------------------------------------
 * randint のモック。呼びだしごとに固定値を返し、渡された引数を記録する。
 * MU_SETUP は minunit.h より前に定義する必要がある（minunit.h 側が
 * #ifndef で既定の空実装を用意しているため）。
 * ------------------------------------------------------------------ */
static int mock_returns = 1;
static int mock_last_maxval = 0;

static int mock_randint(int maxval)
{
    mock_last_maxval = maxval;
    return mock_returns;
}

/* 各テストの前に必ず呼ばれる。先行テストの成否によらず条件が毎回揃う。 */
#define MU_SETUP()                                                             \
    do {                                                                       \
        mock_returns = 1;                                                      \
        mock_last_maxval = 0;                                                  \
    } while (0)

#include "minunit.h"

/* constant.h:168 より。x> なら難しい装置、x< なら易しい装置 */
#define USE_DEVICE 3

typedef int (*randint_fn)(int maxval);

/* --- staffs.c:35-45 / wands.c:42-53 の chance 計算を写したもの
 *     （演算の順序・型・分岐は変更していない）
 *
 * 元は inven_type / struct misc / グローバル py を直接読むが、
 * 計算に使う値だけを引数にした。item_level のキャストは元コードのまま。
 * penalty は杖なら 5、魔法棒なら 0。これが 2 箇所の唯一の差分。
 * confused は py.flags.confused。rnd は misc1.c:59 randint() の差しかえ口。
 */
static int device_chance(int save, int stat_adj_int, int item_level,
                        int penalty, int class_adj, int lev, int confused,
                        randint_fn rnd)
{
    int chance = save + stat_adj_int - (int)item_level - penalty +
                 (class_adj * lev / 3);

    if (confused > 0) {
        chance = chance / 2;
    }
    if ((chance < USE_DEVICE) && (rnd(USE_DEVICE - chance + 1) == 1)) {
        chance = USE_DEVICE; // Give everyone a slight chance
    }
    if (chance <= 0) {
        chance = 1;
    }
    return chance;
}

/* staffs.c:46 / wands.c:54 の成功判定。randint(chance) < USE_DEVICE が失敗。 */
static int device_use_succeeds(int chance, randint_fn rnd)
{
    return !(rnd(chance) < USE_DEVICE);
}
/* --- ここまで --- */

/* 杖のペナルティ 5、魔法棒は 0。以下のテストで使う共通の呼びだし形。 */
#define STAFF_PENALTY 5
#define WAND_PENALTY 0

/* 代表値：save 20, INT 補正 3, 道具 level 10, class 補正 0
 * → 20 + 3 - 10 - 5 = 8。USE_DEVICE(3) 以上なので下限補正は働かない。 */
TEST(staff_chance_subtracts_item_level_and_penalty_five)
{
    ASSERT_EQ_INT(device_chance(20, 3, 10, STAFF_PENALTY, 0, 1, 0, mock_randint), 8);
}

/* 三角測量：同じ条件でも魔法棒はペナルティがないので 5 大きい。
 * これが staffs.c と wands.c の唯一の差分。 */
TEST(wand_chance_has_no_penalty_and_is_five_higher_than_staff)
{
    ASSERT_EQ_INT(device_chance(20, 3, 10, WAND_PENALTY, 0, 1, 0, mock_randint), 13);
}

/* 三角測量：道具の level が上がるほど chance は下がる */
TEST(staff_chance_decreases_as_item_level_rises)
{
    ASSERT_EQ_INT(device_chance(20, 3, 15, STAFF_PENALTY, 0, 1, 0, mock_randint), 3);
}

/* 三角測量：save が上がれば chance も同じだけ上がる */
TEST(staff_chance_increases_with_saving_throw)
{
    ASSERT_EQ_INT(device_chance(30, 3, 10, STAFF_PENALTY, 0, 1, 0, mock_randint), 18);
}

/* stat_adj(A_INT) は負にもなる（低い INT）。そのまま減算される */
TEST(staff_chance_is_reduced_by_negative_intelligence_adjustment)
{
    ASSERT_EQ_INT(device_chance(20, -2, 10, STAFF_PENALTY, 0, 1, 0, mock_randint), 3);
}

/* class 補正の項は class_adj * lev / 3 で、整数除算により切り捨てられる。
 * 2 * 5 / 3 = 10/3 = 3。20 + 0 - 10 - 5 + 3 = 8 */
TEST(staff_chance_class_bonus_is_truncated_by_integer_division)
{
    ASSERT_EQ_INT(device_chance(20, 0, 10, STAFF_PENALTY, 2, 5, 0, mock_randint), 8);
}

/* 三角測量：レベルが上がると class 補正の項も増える。
 * 2 * 6 / 3 = 4。20 + 0 - 10 - 5 + 4 = 9 */
TEST(staff_chance_class_bonus_grows_with_character_level)
{
    ASSERT_EQ_INT(device_chance(20, 0, 10, STAFF_PENALTY, 2, 6, 0, mock_randint), 9);
}

/* 混乱していると chance は半分になる。8 / 2 = 4。
 * 4 は USE_DEVICE(3) 以上なので下限補正は働かない。 */
TEST(staff_chance_is_halved_when_confused)
{
    ASSERT_EQ_INT(device_chance(20, 3, 10, STAFF_PENALTY, 0, 1, 1, mock_randint), 4);
}

/* 三角測量：混乱していなければ半分にならない（同じ引数で 8 のまま）。
 * confused は「> 0」で判定されるので 0 は非混乱。 */
TEST(staff_chance_is_not_halved_when_not_confused)
{
    ASSERT_EQ_INT(device_chance(20, 3, 10, STAFF_PENALTY, 0, 1, 0, mock_randint), 8);
}

/* 混乱時の半分も整数除算。9 / 2 = 4 に切り捨てられる。 */
TEST(staff_chance_halving_truncates_odd_value)
{
    ASSERT_EQ_INT(device_chance(21, 3, 10, STAFF_PENALTY, 0, 1, 1, mock_randint), 4);
}

/* 下限補正：chance が USE_DEVICE 未満のとき randint(USE_DEVICE - chance + 1)
 * が 1 なら chance は USE_DEVICE に引きあげられる。
 * 20 + 0 - 18 - 5 = -3 だが、モックが 1 を返すので 3 になる。 */
TEST(staff_chance_below_use_device_is_raised_to_use_device_on_lucky_roll)
{
    ASSERT_EQ_INT(device_chance(20, 0, 18, STAFF_PENALTY, 0, 1, 0, mock_randint), 3);
}

/* 三角測量：同じ条件でも randint が 1 以外を返すと引きあげは起きない。
 * -3 のまま chance <= 0 の分岐に落ち、1 になる。 */
TEST(staff_chance_stays_at_one_when_lucky_roll_misses)
{
    mock_returns = 2;
    ASSERT_EQ_INT(device_chance(20, 0, 18, STAFF_PENALTY, 0, 1, 0, mock_randint), 1);
}

/* 境界：chance がちょうど USE_DEVICE のときは「< USE_DEVICE」が偽なので
 * 引きあげの判定に入らない。3 のまま。 */
TEST(staff_chance_exactly_use_device_is_left_untouched)
{
    mock_returns = 2;
    ASSERT_EQ_INT(device_chance(20, 0, 12, STAFF_PENALTY, 0, 1, 0, mock_randint), 3);
}

/* 境界：chance が 2（USE_DEVICE 未満）なら引きあげの判定に入り、
 * randint が 1 以外なので 2 のまま。chance > 0 なので 1 にもならない。 */
TEST(staff_chance_two_survives_when_lucky_roll_misses)
{
    mock_returns = 2;
    ASSERT_EQ_INT(device_chance(20, 0, 13, STAFF_PENALTY, 0, 1, 0, mock_randint), 2);
}

/* 境界：chance がちょうど 0 のとき「chance <= 0」で 1 に引きあげられる。 */
TEST(staff_chance_of_zero_is_raised_to_one)
{
    mock_returns = 2;
    ASSERT_EQ_INT(device_chance(20, 0, 15, STAFF_PENALTY, 0, 1, 0, mock_randint), 1);
}

/* 下限補正の抽選幅は chance が低いほど広い（当たりにくい）。
 * chance = -3 のとき randint(3 - (-3) + 1) = randint(7)。 */
TEST(staff_lucky_roll_range_widens_as_chance_falls)
{
    mock_returns = 2;
    device_chance(20, 0, 18, STAFF_PENALTY, 0, 1, 0, mock_randint);
    ASSERT_EQ_INT(mock_last_maxval, 7);
}

/* 三角測量：chance = 2 なら randint(3 - 2 + 1) = randint(2)。 */
TEST(staff_lucky_roll_range_is_two_when_chance_is_two)
{
    mock_returns = 2;
    device_chance(20, 0, 13, STAFF_PENALTY, 0, 1, 0, mock_randint);
    ASSERT_EQ_INT(mock_last_maxval, 2);
}

/* 混乱時に chance が負だと、C の整数除算は 0 方向に丸める。
 * -9 / 2 は -4（-5 ではない）。下限補正の抽選幅 randint(3-(-4)+1)=randint(8)
 * にその丸めが現れるので、ここで固定しておく。 */
TEST(negative_chance_halving_truncates_toward_zero)
{
    mock_returns = 2;
    device_chance(0, 0, 4, STAFF_PENALTY, 0, 1, 1, mock_randint);
    ASSERT_EQ_INT(mock_last_maxval, 8);
}

/* TODO: 仕様確認 -- 混乱すると chance が改善する場合がある。
 * chance が負のとき chance/2 は 0 方向に丸められて負の絶対値が小さくなり、
 * 下限補正の抽選幅 randint(USE_DEVICE - chance + 1) が狭くなる。
 * 抽選幅が狭いほど 1 を引きやすい＝USE_DEVICE に引きあげられやすいので、
 * 「混乱しているほうが使いやすい」という逆転が起きる。
 * 非混乱では chance = -9 → randint(13)、混乱では -4 → randint(8)。 */
TEST(confusion_narrows_lucky_roll_range_when_chance_is_negative)
{
    mock_returns = 2;
    device_chance(0, 0, 4, STAFF_PENALTY, 0, 1, 0, mock_randint);
    ASSERT_EQ_INT(mock_last_maxval, 13);
}

/* 下限補正の抽選幅は必ず 2 以上になる。
 * 判定に入る条件が chance < USE_DEVICE なので USE_DEVICE - chance + 1 >= 2。
 * つまり randint(0) や randint(負) には到達しない。
 * chance を極端に低くしても幅は広がるだけで、0 以下にはならない。 */
TEST(lucky_roll_range_never_reaches_zero_even_for_very_low_chance)
{
    mock_returns = 2;
    device_chance(0, 0, 100, STAFF_PENALTY, 0, 1, 0, mock_randint);
    ASSERT_EQ_INT(mock_last_maxval, 109);
}

/* 成功判定：randint(chance) が USE_DEVICE 以上なら成功。
 * chance = 10 で 3 が出れば成功（3 < 3 は偽）。 */
TEST(device_use_succeeds_when_roll_reaches_use_device)
{
    mock_returns = 3;
    ASSERT_TRUE(device_use_succeeds(10, mock_randint));
}

/* 三角測量：同じ chance でも 2 が出れば失敗（2 < 3）。 */
TEST(device_use_fails_when_roll_is_below_use_device)
{
    mock_returns = 2;
    ASSERT_FALSE(device_use_succeeds(10, mock_randint));
}

/* chance が 1 のとき randint(1) は必ず 1 を返すので、成功はありえない。
 * 下限補正に外れた不利な状況は必ず失敗するということ。 */
TEST(device_use_always_fails_when_chance_is_one)
{
    ASSERT_FALSE(device_use_succeeds(1, mock_randint));
}

/* 成功判定に渡る引数は chance そのもの。判定側で加工されない。 */
TEST(device_use_rolls_against_chance_itself)
{
    mock_returns = 3;
    device_use_succeeds(10, mock_randint);
    ASSERT_EQ_INT(mock_last_maxval, 10);
}

int main(void)
{
    RUN_TEST(staff_chance_subtracts_item_level_and_penalty_five);
    RUN_TEST(wand_chance_has_no_penalty_and_is_five_higher_than_staff);
    RUN_TEST(staff_chance_decreases_as_item_level_rises);
    RUN_TEST(staff_chance_increases_with_saving_throw);
    RUN_TEST(staff_chance_is_reduced_by_negative_intelligence_adjustment);
    RUN_TEST(staff_chance_class_bonus_is_truncated_by_integer_division);
    RUN_TEST(staff_chance_class_bonus_grows_with_character_level);
    RUN_TEST(staff_chance_is_halved_when_confused);
    RUN_TEST(staff_chance_is_not_halved_when_not_confused);
    RUN_TEST(staff_chance_halving_truncates_odd_value);
    RUN_TEST(staff_chance_below_use_device_is_raised_to_use_device_on_lucky_roll);
    RUN_TEST(staff_chance_stays_at_one_when_lucky_roll_misses);
    RUN_TEST(staff_chance_exactly_use_device_is_left_untouched);
    RUN_TEST(staff_chance_two_survives_when_lucky_roll_misses);
    RUN_TEST(staff_chance_of_zero_is_raised_to_one);
    RUN_TEST(staff_lucky_roll_range_widens_as_chance_falls);
    RUN_TEST(staff_lucky_roll_range_is_two_when_chance_is_two);
    RUN_TEST(negative_chance_halving_truncates_toward_zero);
    RUN_TEST(confusion_narrows_lucky_roll_range_when_chance_is_negative);
    RUN_TEST(lucky_roll_range_never_reaches_zero_even_for_very_low_chance);
    RUN_TEST(device_use_succeeds_when_roll_reaches_use_device);
    RUN_TEST(device_use_fails_when_roll_is_below_use_device);
    RUN_TEST(device_use_always_fails_when_chance_is_one);
    RUN_TEST(device_use_rolls_against_chance_itself);
    return TEST_SUMMARY();
}
