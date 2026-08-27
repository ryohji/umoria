/* モンスターの行動回数計算のテスト -- 現在の実装を保護する
 *
 * src/creature.c:75-86 の moves_this_turn() は、名前と戻り値の意味が
 * 乖離している。
 *
 *   speed > 0  のとき: 行動回数（speed 回。ただし休憩中は 1 回に抑える）
 *   speed <= 0 のとき: 条件式 (turn % (2 - speed)) == 0 の結果（0 か 1）
 *
 * 呼びだし元（src/creature.c:1529）は k = moves_this_turn(...) の結果を
 * while (k > 0) { k--; ... } で「回数」として消費するので、0/1 は
 * 「0 回 / 1 回」として機能している。つまり呼びだし側の解釈は一貫して
 * 「回数」であり、後者の枝は「周期的に 1 回動く」を意味している。
 *
 * ステップ B ではふるまいを変えずに次を行う予定。
 *   1. 名前を「回数を返す」と読める形に変える
 *   2. return (式) を ? 1 : 0 にして、真偽ではなく回数だと明示する
 *   3. speed > 0 / それ以外の対称性を保つ
 * このテストは、その 3 つでふるまいが変わらないことを保証する。
 *
 * 観測のしかた: movement_rate は static なので外部から呼べない。そこで
 * src/creature.c をこの翻訳単位に #include し、内部の static 関数を
 * 直接呼ぶ。写しではないので、ステップ B で実体を書きかえればこの
 * テストがそれを検証する。先例は tests/haggle_comment_test.c。
 *
 * 依存はグローバル turn（int32_t、tests/creature_stubs.c が定義）と
 * py.flags.rest の 2 つだけ。turn は fixture_reset() の対象外なので、
 * 各テストが明示的に代入する。
 *
 * 期待値はすべて現在の実装が返した実際の値。とくに負の turn に対する
 * C の % の結果は実装に教えてもらった（自分で計算していない）。
 */
/* 検証対象。static 関数を呼ぶために実体ごと取りこむ。headers.h /
 * config.h / constant.h / types.h も creature.c が連れてくる。 */
#include "creature.c"

#include "fixture.h"

/* 各テストの前に必ず呼ばれる。py がクリアされる（rest も 0 に戻る）。
 * turn は意図的に触らないので、テストごとに代入して制御する。 */
#define MU_SETUP() fixture_reset()

#include "minunit.h"

/* ------------------------------------------------------------------
 * 速度が正のとき: 行動回数はそのまま speed
 *
 * ただし py.flags.rest != 0（休憩中）なら 1 回に抑えられる。
 * 本体のコメント（creature.c:73-74）が「プレイヤーは反復ごとに
 * 少なくとも 1 回動く。遅くなったプレイヤーはモンスターを速く動かして
 * 表現する」と書いているのがこの枝。
 * ------------------------------------------------------------------ */

TEST(returns_one_move_when_speed_is_one)
{
    turn = 0;
    ASSERT_EQ_INT(moves_this_turn(1), 1);
}

/* 三角測量。1 だけでは「つねに 1 を返す」実装と区別できない。 */
TEST(returns_five_moves_when_speed_is_five)
{
    turn = 0;
    ASSERT_EQ_INT(moves_this_turn(5), 5);
}

/* 休憩中は speed が大きくても 1 回に抑えられる。 */
TEST(returns_one_move_when_speed_is_five_and_player_is_resting)
{
    turn = 0;
    py.flags.rest = 1;
    ASSERT_EQ_INT(moves_this_turn(5), 1);
}

/* 上との対比。rest を 0 に戻せば speed 回に戻る。
 * rest の判定は != 0 なので、休憩の残りターン数がいくつでも 1 になる。 */
TEST(returns_five_moves_again_when_resting_is_cleared)
{
    turn = 0;
    py.flags.rest = 0;
    ASSERT_EQ_INT(moves_this_turn(5), 5);
}

/* rest は int16_t で負も入る。判定が != 0 なので負でも抑制される。
 * 本体では「休憩ターン数が -1 のとき無限休憩」という使い方がある。 */
TEST(returns_one_move_when_resting_count_is_negative)
{
    turn = 0;
    py.flags.rest = -1;
    ASSERT_EQ_INT(moves_this_turn(5), 1);
}

/* ------------------------------------------------------------------
 * 速度が 0 以下のとき: turn の周期で 1 回動くか動かないかが決まる
 *
 * 式は (turn % (2 - speed)) == 0。周期は 2 - speed なので
 * speed = 0 で 2、speed = -1 で 3、speed = -2 で 4 と、遅くなるほど長い。
 * 周期の頭（余り 0）だけ 1 回動き、それ以外は 0 回。
 *
 * SUSPICIOUS: 本体のコメント（creature.c:83）は
 * 「// speed must be negative here」と書いているが、この枝の条件は
 * speed > 0 の否定なので speed == 0 も入る。0 のときも 2 - 0 = 2 で
 * 周期 2 として正しく動くので、コメントが誤っている。直していない。
 * ------------------------------------------------------------------ */

TEST(moves_once_at_the_start_of_the_cycle_when_speed_is_zero)
{
    turn = 0;
    ASSERT_EQ_INT(moves_this_turn(0), 1);
}

TEST(does_not_move_on_the_odd_turn_when_speed_is_zero)
{
    turn = 1;
    ASSERT_EQ_INT(moves_this_turn(0), 0);
}

/* 周期 2 なので turn = 2 でまた動く。周期性が固定される。 */
TEST(moves_again_two_turns_later_when_speed_is_zero)
{
    turn = 2;
    ASSERT_EQ_INT(moves_this_turn(0), 1);
}

/* SUSPICIOUS: turn は variable.c:69 で -1 に初期化されている。C の % は
 * 被除数が負なら 0 か負の余りを返すので、-1 % 2 は -1 になり、== 0 が
 * 成り立たず「動かない」と判定される。ゲーム開始直後にこの枝を通る
 * 可能性があるが、意図的なのか判断できない。実装が返した値をそのまま
 * 固定する（自分で計算していない）。 */
TEST(does_not_move_when_turn_is_the_initial_minus_one_and_speed_is_zero)
{
    turn = -1;
    ASSERT_EQ_INT(moves_this_turn(0), 0);
}

/* 一方で -2 % 2 は 0 なので、負の turn でも周期の頭にあたれば動く。
 * つまり負の側でも周期 2 は保たれており、位相だけがずれている。 */
TEST(moves_once_when_turn_is_minus_two_and_speed_is_zero)
{
    turn = -2;
    ASSERT_EQ_INT(moves_this_turn(0), 1);
}

/* ------------------------------------------------------------------
 * 速度が負のとき: 周期は 2 - speed で、遅いほど間隔が伸びる
 *
 * speed = -1 なら 3 ターンに 1 回、speed = -2 なら 4 ターンに 1 回。
 * 周期そのものを固定しておかないと、式を 2 + (-speed) や 2 - speed の
 * 取りちがえに書きかえても気づけない。
 * ------------------------------------------------------------------ */

TEST(moves_once_every_three_turns_at_turn_zero_when_speed_is_minus_one)
{
    turn = 0;
    ASSERT_EQ_INT(moves_this_turn(-1), 1);
}

TEST(does_not_move_at_turn_one_when_speed_is_minus_one)
{
    turn = 1;
    ASSERT_EQ_INT(moves_this_turn(-1), 0);
}

/* 周期 2 との違いはここに出る。speed = 0 なら turn = 2 で動くが、
 * speed = -1 では周期 3 なのでまだ動かない。 */
TEST(does_not_move_at_turn_two_when_speed_is_minus_one)
{
    turn = 2;
    ASSERT_EQ_INT(moves_this_turn(-1), 0);
}

TEST(moves_again_at_turn_three_when_speed_is_minus_one)
{
    turn = 3;
    ASSERT_EQ_INT(moves_this_turn(-1), 1);
}

/* 三角測量。speed = -2 なら周期 4。turn = 3 は speed = -1 では動く
 * ターンだが、周期 4 では動かない。 */
TEST(does_not_move_at_turn_three_when_speed_is_minus_two)
{
    turn = 3;
    ASSERT_EQ_INT(moves_this_turn(-2), 0);
}

TEST(moves_once_at_turn_four_when_speed_is_minus_two)
{
    turn = 4;
    ASSERT_EQ_INT(moves_this_turn(-2), 1);
}

/* ------------------------------------------------------------------
 * 戻り値の範囲: 速度が 0 以下なら必ず 0 か 1
 *
 * ステップ B は return (turn % (2 - speed)) == 0; を
 * return ((turn % (2 - speed)) == 0) ? 1 : 0; に書きかえる予定。
 * C の == は 0 か 1 しか返さないので等価なのだが、それを根拠として
 * 残しておく。周期の中のどの位置でも 0 か 1 のどちらかであることを、
 * 周期 3 の全位相と大きい turn について固定する。
 *
 * 「0 か 1 のいずれか」を条件分岐で書くとテストが読めなくなるので、
 * 各位相の値を 1 件ずつ直接固定する。
 * ------------------------------------------------------------------ */

TEST(returns_exactly_one_at_phase_zero_of_the_three_turn_cycle)
{
    turn = 9;
    ASSERT_EQ_INT(moves_this_turn(-1), 1);
}

TEST(returns_exactly_zero_at_phase_one_of_the_three_turn_cycle)
{
    turn = 10;
    ASSERT_EQ_INT(moves_this_turn(-1), 0);
}

TEST(returns_exactly_zero_at_phase_two_of_the_three_turn_cycle)
{
    turn = 11;
    ASSERT_EQ_INT(moves_this_turn(-1), 0);
}

/* 大きい turn でも 1 を超えない。回数として消費されるので、2 以上が
 * 返れば呼びだし側のループが余分に回ってしまう。 */
TEST(returns_exactly_one_at_a_large_turn_on_the_cycle)
{
    turn = 1000000;
    ASSERT_EQ_INT(moves_this_turn(-2), 1);
}

/* 速度が大きく負でも上限は 1。周期 12 の頭を外したところ。 */
TEST(returns_exactly_zero_for_a_deeply_negative_speed_off_the_cycle)
{
    turn = 5;
    ASSERT_EQ_INT(moves_this_turn(-10), 0);
}

/* ------------------------------------------------------------------
 * py.flags.rest が読まれるのは speed > 0 の枝だけ
 *
 * 速度が 0 以下のときは rest を見ない。ステップ B で 2 つの枝の対称性を
 * そろえるときに、rest の判定をうっかり外側へ引きあげるとふるまいが
 * 変わる。休憩中でも周期の判定がそのまま残ることを固定する。
 * ------------------------------------------------------------------ */

TEST(ignores_resting_when_speed_is_zero_and_the_turn_is_on_the_cycle)
{
    turn = 0;
    py.flags.rest = 1;
    ASSERT_EQ_INT(moves_this_turn(0), 1);
}

/* ここが本質。rest != 0 でも「1 回」に持ちあげられず、0 のまま。
 * 判定を外側に出すと 1 が返ってレッドになる。 */
TEST(ignores_resting_when_speed_is_zero_and_the_turn_is_off_the_cycle)
{
    turn = 1;
    py.flags.rest = 1;
    ASSERT_EQ_INT(moves_this_turn(0), 0);
}

TEST(ignores_resting_when_speed_is_negative_and_the_turn_is_off_the_cycle)
{
    turn = 1;
    py.flags.rest = 100;
    ASSERT_EQ_INT(moves_this_turn(-1), 0);
}

int main(void)
{
    RUN_TEST(returns_one_move_when_speed_is_one);
    RUN_TEST(returns_five_moves_when_speed_is_five);
    RUN_TEST(returns_one_move_when_speed_is_five_and_player_is_resting);
    RUN_TEST(returns_five_moves_again_when_resting_is_cleared);
    RUN_TEST(returns_one_move_when_resting_count_is_negative);
    RUN_TEST(moves_once_at_the_start_of_the_cycle_when_speed_is_zero);
    RUN_TEST(does_not_move_on_the_odd_turn_when_speed_is_zero);
    RUN_TEST(moves_again_two_turns_later_when_speed_is_zero);
    RUN_TEST(does_not_move_when_turn_is_the_initial_minus_one_and_speed_is_zero);
    RUN_TEST(moves_once_when_turn_is_minus_two_and_speed_is_zero);
    RUN_TEST(moves_once_every_three_turns_at_turn_zero_when_speed_is_minus_one);
    RUN_TEST(does_not_move_at_turn_one_when_speed_is_minus_one);
    RUN_TEST(does_not_move_at_turn_two_when_speed_is_minus_one);
    RUN_TEST(moves_again_at_turn_three_when_speed_is_minus_one);
    RUN_TEST(does_not_move_at_turn_three_when_speed_is_minus_two);
    RUN_TEST(moves_once_at_turn_four_when_speed_is_minus_two);
    RUN_TEST(returns_exactly_one_at_phase_zero_of_the_three_turn_cycle);
    RUN_TEST(returns_exactly_zero_at_phase_one_of_the_three_turn_cycle);
    RUN_TEST(returns_exactly_zero_at_phase_two_of_the_three_turn_cycle);
    RUN_TEST(returns_exactly_one_at_a_large_turn_on_the_cycle);
    RUN_TEST(returns_exactly_zero_for_a_deeply_negative_speed_off_the_cycle);
    RUN_TEST(ignores_resting_when_speed_is_zero_and_the_turn_is_on_the_cycle);
    RUN_TEST(ignores_resting_when_speed_is_zero_and_the_turn_is_off_the_cycle);
    RUN_TEST(ignores_resting_when_speed_is_negative_and_the_turn_is_off_the_cycle);
    return TEST_SUMMARY();
}
