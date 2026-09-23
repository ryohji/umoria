/* 重さに負けているかどうかの判定（misc3.c:975 の check_strength と
 * misc3.c:957 の inven_check_weight）のテスト
 *
 * check_strength() は「いまの装備と持ち物に体力が足りているか」を 1 か所で
 * 判定し、答えを 2 つ覚える唯一の場所（#18-7-4 の窓口 burden.h が預かる
 * weapon_heavy と pack_heavy）。戻り値が無く、結果は
 *
 *   ・メッセージ（msg_print で 4 通り）
 *   ・速度（change_speed に**段数の差**を渡す）
 *   ・能力の再計算（calc_bonuses）
 *   ・覚えた 2 つの値
 *
 * にしか現れない。単体テストが 1 件も届いていなかったので、窓口を通す
 * 書きかえ（ステップ B）の前にここで押さえる。この 1 本は**本体を変えずに**
 * 足す（テストが通ることを先に確かめ、そのあとで本体を直す）。
 *
 * 保護したい仕掛けは 5 つ。
 *
 *   1. メッセージは**変わったときだけ**出る。武器を持ちかえずに何度
 *      check_strength を呼んでも「重い」と言われつづけることはない。
 *      旗を覚えるのはこのためで、旗を捨てると毎 turn 文句を言う。
 *
 *   2. 荷は旗ではなく**段数**で、速度は change_speed(新しい段数 − 覚えた段数)
 *      という**差**で動く。覚えた数が丸められると速度が狂う。差の符号が
 *      向き（遅くなる／速くなる）で、メッセージの出しわけも同じ向きを見る。
 *
 *   3. 素手（TV_NOTHING）は重すぎにならない。ただし「重すぎの武器を外した」
 *      ときは旗を降ろして再計算するが、**メッセージは出さない**
 *      （外した手に「持てるようになった」と言っても意味がないので）。
 *
 *   4. 武器と荷は別の記録。両方が重ければ両方について言う。
 *
 *   5. inven_check_weight() は「拾うと速度が変わるか」を**覚えた段数と
 *      比べて**答える。覚えた値は書きかえない（問うだけ）。
 *
 * 期待値はすべて現在の実装が返した実際の値。仕様書はないので、いまどう
 * 振るまうかを固定することが目的。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "burden.h"
#include "equipment.h"
#include "fixture.h"
#include "inventory.h"

extern player_type py;

/* 検証対象（src/misc3.c）。externs.h は ncurses まで引きこむので、
 * 必要な宣言だけをここに書く。 */
void check_strength(void);
bool inven_check_weight(inven_type *i_ptr);
int weight_limit(void);

#define MU_SETUP() fixture_reset()

#include "minunit.h"

/* ------------------------------------------------------------------
 * 条件づくりの補助関数
 *
 * 数はすべて本体の 2 つの式から逆算したもの。
 *
 *   武器が重すぎる条件: use_stat[A_STR] * 15 < 武器の重さ
 *   持てる重さの上限:   use_stat[A_STR] * PLAYER_WEIGHT_CAP + py.misc.wt
 *                       （PLAYER_WEIGHT_CAP は 130、3000 で打ち切り）
 *
 * STR 10・体重 0 なら、武器の境目は 150、上限は 1300 になる。以下の関数は
 * この STR 10 を前提に組んである。段数は 持ち物の重さ / (上限 + 1) の
 * 整数除算なので、1300 を超えたところから 1 段ずつ増える。
 * ------------------------------------------------------------------ */

/* STR 10・体重 0。武器の境目 150、持てる重さの上限 1300。
 * fixture_reset() が py を 0 で埋めるので、STR だけ入れれば足りる。 */
static void given_a_character_of_average_strength(void) {
    py.stats.use_stat[A_STR] = 10;
}

/* 手に持っている武器。重さ 150 までなら振れる。 */
static void given_a_wielded_weapon_weighing(int weight) {
    inven_type *i_ptr = equipment_at(INVEN_WIELD);
    i_ptr->tval = TV_SWORD;
    i_ptr->weight = (uint16_t)weight;
}

/* 何も持っていない手。fixture_reset() の直後と同じ状態（TV_NOTHING は 0）。 */
static void given_an_empty_weapon_hand(void) {
    equipment_at(INVEN_WIELD)->tval = TV_NOTHING;
}

/* 持ち物の重さ。合計だけを見るので、品物を並べる必要はない
 * （check_strength も inven_check_weight も inventory_weight() しか読まない）。 */
static void given_a_pack_weighing(int weight) {
    inventory_set_weight(weight);
}

/* いま覚えている答えを、テストが望む状態に置く。本体の代わりに窓口から
 * 直に入れる（check_strength を 2 回呼んで作ると、1 回目のメッセージと
 * 速度の記録が 2 回目の観測に混ざる）。 */
static void given_the_remembered_answers(bool weapon_too_heavy, int pack_steps) {
    set_weapon_too_heavy(weapon_too_heavy);
    set_pack_speed_penalty(pack_steps);
}

/* 「2 度目は黙る」を見るテストは check_strength を 2 回呼んで、メッセージの
 * **合計**が 1 件のままであることを見る（記録を途中で消す窓口は無く、
 * fixture_reset() では py まで消えてしまうので、合計で見るのが確実）。 */

/* 上限の計算そのもの。段数の期待値がどこから来たのかを読む人に示す。 */
static int the_weight_limit(void) { return weight_limit(); }

/* ------------------------------------------------------------------
 * 何も起きない場合
 * ------------------------------------------------------------------ */

/* 振れる武器と軽い荷。メッセージも速度の変更も再計算も無い。 */
TEST(a_weapon_that_fits_and_a_light_pack_are_not_worth_mentioning) {
    given_a_character_of_average_strength();
    given_a_wielded_weapon_weighing(100);
    given_a_pack_weighing(1000);
    given_the_remembered_answers(false, 0);

    check_strength();

    ASSERT_EQ_INT(0, fixture_message_count());
    ASSERT_EQ_INT(0, fixture_speed_change_count());
    ASSERT_EQ_INT(0, fixture_calc_bonuses_count());
    ASSERT_TRUE(!weapon_is_too_heavy());
    ASSERT_EQ_INT(0, pack_speed_penalty());
}

/* 素手も「重すぎる」にはならない。武器の重さの検査は TV_NOTHING を除く。 */
TEST(an_empty_hand_is_never_too_heavy) {
    given_a_character_of_average_strength();
    given_an_empty_weapon_hand();
    given_the_remembered_answers(false, 0);

    check_strength();

    ASSERT_TRUE(!weapon_is_too_heavy());
    ASSERT_EQ_INT(0, fixture_message_count());
}

/* ------------------------------------------------------------------
 * 武器の側
 * ------------------------------------------------------------------ */

/* 境目は STR * 15。10 * 15 = 150 なので、151 から重すぎになる。 */
TEST(a_weapon_just_over_fifteen_times_the_strength_is_too_heavy) {
    given_a_character_of_average_strength();
    given_a_wielded_weapon_weighing(151);
    given_the_remembered_answers(false, 0);

    check_strength();

    ASSERT_TRUE(weapon_is_too_heavy());
}

/* ちょうど 150 は振れる（比較は < なので境目そのものは重すぎない）。 */
TEST(a_weapon_of_exactly_fifteen_times_the_strength_still_fits) {
    given_a_character_of_average_strength();
    given_a_wielded_weapon_weighing(150);
    given_the_remembered_answers(false, 0);

    check_strength();

    ASSERT_TRUE(!weapon_is_too_heavy());
}

/* 重すぎになったときは文句を言い、旗を立て、能力を計算しなおす
 * （重すぎの武器は命中が落ちるので、再計算が要る）。 */
TEST(taking_up_too_heavy_a_weapon_is_announced_and_recalculated) {
    given_a_character_of_average_strength();
    given_a_wielded_weapon_weighing(300);
    given_the_remembered_answers(false, 0);

    check_strength();

    ASSERT_EQ_INT(1, fixture_message_count());
    ASSERT_EQ_STR("You have trouble wielding such a heavy weapon.",
                  fixture_message_text(0));
    ASSERT_TRUE(weapon_is_too_heavy());
    ASSERT_EQ_INT(1, fixture_calc_bonuses_count());
}

/* 覚えているから 2 度は言わない。旗を捨てると毎 turn 文句が出る。 */
TEST(the_complaint_about_the_weapon_is_not_repeated) {
    given_a_character_of_average_strength();
    given_a_wielded_weapon_weighing(300);
    given_the_remembered_answers(false, 0);

    check_strength();
    check_strength();

    ASSERT_EQ_INT(1, fixture_message_count());
    ASSERT_EQ_INT(1, fixture_calc_bonuses_count());
}

/* 強くなって振れるようになったら、そう言って旗を降ろす。 */
TEST(becoming_strong_enough_for_the_weapon_is_announced) {
    given_a_character_of_average_strength();
    given_a_wielded_weapon_weighing(200);
    given_the_remembered_answers(true, 0);
    py.stats.use_stat[A_STR] = 20; /* 境目が 300 に上がる */

    check_strength();

    ASSERT_EQ_INT(1, fixture_message_count());
    ASSERT_EQ_STR("You are strong enough to wield your weapon.",
                  fixture_message_text(0));
    ASSERT_TRUE(!weapon_is_too_heavy());
    ASSERT_EQ_INT(1, fixture_calc_bonuses_count());
}

/* 重すぎる武器を外したときは、旗を降ろして再計算するが**黙っている**。
 * 空の手に「振れるようになった」と言っても意味がないので。 */
TEST(putting_down_too_heavy_a_weapon_lowers_the_flag_without_a_word) {
    given_a_character_of_average_strength();
    given_an_empty_weapon_hand();
    given_the_remembered_answers(true, 0);

    check_strength();

    ASSERT_TRUE(!weapon_is_too_heavy());
    ASSERT_EQ_INT(0, fixture_message_count());
    ASSERT_EQ_INT(1, fixture_calc_bonuses_count());
}

/* ------------------------------------------------------------------
 * 荷の側
 *
 * 段数 = 持ち物の重さ / (上限 + 1) の整数除算。上限を超えていなければ 0。
 * STR 10・体重 0 なら上限は 1300 なので、割る数は 1301 になる。
 *   2000 / 1301 = 1、4000 / 1301 = 3、6000 / 1301 = 4。
 * ------------------------------------------------------------------ */

/* 上限そのものは超えても、段数は「上限の何倍か」で決まる。
 * 1301 でちょうど 1 段。期待値の出どころを式で確かめておく。 */
TEST(the_limit_for_an_average_character_is_thirteen_hundred) {
    given_a_character_of_average_strength();

    ASSERT_EQ_INT(1300, the_weight_limit());
}

/* 上限ぴったりは重くない（比較は 上限 < 重さ）。 */
TEST(a_pack_at_exactly_the_limit_costs_no_speed) {
    given_a_character_of_average_strength();
    given_a_pack_weighing(1300);
    given_the_remembered_answers(false, 0);

    check_strength();

    ASSERT_EQ_INT(0, pack_speed_penalty());
    ASSERT_EQ_INT(0, fixture_speed_change_count());
}

/* 上限を超えたら遅くなると言い、段数を覚えて、速度をその段数だけ落とす。 */
TEST(a_pack_over_the_limit_slows_the_character_down) {
    given_a_character_of_average_strength();
    given_a_pack_weighing(2000);
    given_the_remembered_answers(false, 0);

    check_strength();

    ASSERT_EQ_INT(1, fixture_message_count());
    ASSERT_EQ_STR("Your pack is so heavy that it slows you down.",
                  fixture_message_text(0));
    ASSERT_EQ_INT(1, pack_speed_penalty());
    ASSERT_EQ_INT(1, fixture_speed_change_count());
    ASSERT_EQ_INT(1, fixture_speed_change_last_steps());
}

/* 旗ではなく段数。重ければ段数が増える（2000 は 1 段、4000 は 3 段）。 */
TEST(a_heavier_pack_costs_more_steps_of_speed) {
    given_a_character_of_average_strength();
    given_a_pack_weighing(4000);
    given_the_remembered_answers(false, 0);

    check_strength();

    ASSERT_EQ_INT(3, pack_speed_penalty());
    ASSERT_EQ_INT(3, fixture_speed_change_last_steps());
}

/* change_speed に渡るのは段数そのものではなく**差**。1 段だった荷が 3 段に
 * なったら、渡すのは 2。ここを段数そのものにすると速度が二重に落ちる。 */
TEST(the_speed_changes_by_the_difference_not_by_the_whole_penalty) {
    given_a_character_of_average_strength();
    given_a_pack_weighing(4000);
    given_the_remembered_answers(false, 1);

    check_strength();

    ASSERT_EQ_INT(2, fixture_speed_change_last_steps());
    ASSERT_EQ_INT(3, pack_speed_penalty());
}

/* 荷を降ろしたら「楽になった」と言い、差は負になる（速度が戻る）。 */
TEST(lightening_the_pack_gives_the_speed_back) {
    given_a_character_of_average_strength();
    given_a_pack_weighing(1000);
    given_the_remembered_answers(false, 3);

    check_strength();

    ASSERT_EQ_INT(1, fixture_message_count());
    ASSERT_EQ_STR("You move more easily under the weight of your pack.",
                  fixture_message_text(0));
    ASSERT_EQ_INT(-3, fixture_speed_change_last_steps());
    ASSERT_EQ_INT(0, pack_speed_penalty());
}

/* 重いままでも、段数が変わらなければ黙っている。速度にも触らない。 */
TEST(a_pack_that_has_not_changed_is_not_mentioned_again) {
    given_a_character_of_average_strength();
    given_a_pack_weighing(4000);
    given_the_remembered_answers(false, 0);

    check_strength();
    check_strength();

    ASSERT_EQ_INT(1, fixture_message_count());
    ASSERT_EQ_INT(1, fixture_speed_change_count());
}

/* ------------------------------------------------------------------
 * 2 つは別の記録
 * ------------------------------------------------------------------ */

/* 両方が重ければ、武器のことも荷のことも言う（この順で）。 */
TEST(a_heavy_weapon_and_a_heavy_pack_are_both_reported) {
    given_a_character_of_average_strength();
    given_a_wielded_weapon_weighing(300);
    given_a_pack_weighing(2000);
    given_the_remembered_answers(false, 0);

    check_strength();

    ASSERT_EQ_INT(2, fixture_message_count());
    ASSERT_EQ_STR("You have trouble wielding such a heavy weapon.",
                  fixture_message_text(0));
    ASSERT_EQ_STR("Your pack is so heavy that it slows you down.",
                  fixture_message_text(1));
    ASSERT_TRUE(weapon_is_too_heavy());
    ASSERT_EQ_INT(1, pack_speed_penalty());
}

/* 武器だけが重いときに荷の段数を動かさない（別の記録であること）。 */
TEST(a_heavy_weapon_does_not_touch_the_speed_the_pack_costs) {
    given_a_character_of_average_strength();
    given_a_wielded_weapon_weighing(300);
    given_a_pack_weighing(4000);
    given_the_remembered_answers(false, 3);

    check_strength();

    ASSERT_EQ_INT(3, pack_speed_penalty());
    ASSERT_EQ_INT(0, fixture_speed_change_count());
}

/* 判定が済んだら「重さを見なおせ」の要求（PY_STR_WGT）を降ろす。
 * ほかの要求は残す（降ろすのはこの 1 bit だけ）。 */
TEST(checking_clears_only_the_request_to_look_at_the_weight) {
    given_a_character_of_average_strength();
    given_a_wielded_weapon_weighing(100);
    given_the_remembered_answers(false, 0);
    py.flags.status = PY_STR_WGT | PY_HUNGRY;

    check_strength();

    ASSERT_EQ_INT(0, (int)(py.flags.status & PY_STR_WGT));
    ASSERT_TRUE((py.flags.status & PY_HUNGRY) != 0);
}

/* ------------------------------------------------------------------
 * 拾うと速度が変わるか（inven_check_weight）
 *
 * 「変わらないなら true」を返す。check_strength と同じ式で段数を出し、
 * **覚えている段数**と比べるだけで、覚えた値は書きかえない。
 * 呼び手は拾う前の確認（moria1.c の床の品物、store1.c の買い物）。
 * ------------------------------------------------------------------ */

/* 検査したい品物。number と weight の積が効くので両方を渡す。 */
static inven_type an_item_to_pick_up(int number, int weight) {
    inven_type item = {0};
    item.number = (uint8_t)number;
    item.weight = (uint16_t)weight;
    return item;
}

/* 上限に収まる品物は速度を変えない。 */
TEST(a_light_item_can_be_picked_up_without_changing_speed) {
    given_a_character_of_average_strength();
    given_a_pack_weighing(1000);
    given_the_remembered_answers(false, 0);
    inven_type item = an_item_to_pick_up(1, 100);

    ASSERT_TRUE(inven_check_weight(&item));
}

/* 上限を超えさせる品物は「変わる」＝ false。 */
TEST(an_item_that_would_slow_the_character_down_is_refused) {
    given_a_character_of_average_strength();
    given_a_pack_weighing(1000);
    given_the_remembered_answers(false, 0);
    inven_type item = an_item_to_pick_up(1, 500);

    ASSERT_TRUE(!inven_check_weight(&item));
}

/* 個数が効く。1 個なら収まる品物が、10 個だと上限を超える
 * （重さだけを見ていると 10 本の矢束を 1 本ぶんと数えてしまう）。 */
TEST(the_number_carried_counts_towards_the_weight) {
    given_a_character_of_average_strength();
    given_a_pack_weighing(0);
    given_the_remembered_answers(false, 0);
    inven_type one = an_item_to_pick_up(1, 200);
    inven_type ten = an_item_to_pick_up(10, 200);

    ASSERT_TRUE(inven_check_weight(&one));
    ASSERT_TRUE(!inven_check_weight(&ten));
}

/* すでに重い荷でも、段数が上がらないなら拾える
 * （旗で覚えていたら、一度重くなった後は何も拾えなくなる）。 */
TEST(an_item_that_keeps_the_pack_at_the_same_penalty_is_accepted) {
    given_a_character_of_average_strength();
    given_a_pack_weighing(4000);
    given_the_remembered_answers(false, 3);
    inven_type item = an_item_to_pick_up(1, 10);

    ASSERT_TRUE(inven_check_weight(&item));
}

/* 次の段に押しあげる品物は断る。 */
TEST(an_item_that_pushes_the_pack_to_the_next_penalty_is_refused) {
    given_a_character_of_average_strength();
    given_a_pack_weighing(4000);
    given_the_remembered_answers(false, 3);
    inven_type item = an_item_to_pick_up(1, 2000);

    ASSERT_TRUE(!inven_check_weight(&item));
}

/* 問うだけ。覚えた段数も速度も動かさない（動かすのは check_strength だけ）。 */
TEST(asking_about_an_item_changes_nothing) {
    given_a_character_of_average_strength();
    given_a_pack_weighing(4000);
    given_the_remembered_answers(false, 3);
    inven_type item = an_item_to_pick_up(1, 2000);

    (void)inven_check_weight(&item);

    ASSERT_EQ_INT(3, pack_speed_penalty());
    ASSERT_EQ_INT(0, fixture_speed_change_count());
    ASSERT_EQ_INT(0, fixture_message_count());
}

int main(void) {
    RUN_TEST(a_weapon_that_fits_and_a_light_pack_are_not_worth_mentioning);
    RUN_TEST(an_empty_hand_is_never_too_heavy);

    RUN_TEST(a_weapon_just_over_fifteen_times_the_strength_is_too_heavy);
    RUN_TEST(a_weapon_of_exactly_fifteen_times_the_strength_still_fits);
    RUN_TEST(taking_up_too_heavy_a_weapon_is_announced_and_recalculated);
    RUN_TEST(the_complaint_about_the_weapon_is_not_repeated);
    RUN_TEST(becoming_strong_enough_for_the_weapon_is_announced);
    RUN_TEST(putting_down_too_heavy_a_weapon_lowers_the_flag_without_a_word);

    RUN_TEST(the_limit_for_an_average_character_is_thirteen_hundred);
    RUN_TEST(a_pack_at_exactly_the_limit_costs_no_speed);
    RUN_TEST(a_pack_over_the_limit_slows_the_character_down);
    RUN_TEST(a_heavier_pack_costs_more_steps_of_speed);
    RUN_TEST(the_speed_changes_by_the_difference_not_by_the_whole_penalty);
    RUN_TEST(lightening_the_pack_gives_the_speed_back);
    RUN_TEST(a_pack_that_has_not_changed_is_not_mentioned_again);

    RUN_TEST(a_heavy_weapon_and_a_heavy_pack_are_both_reported);
    RUN_TEST(a_heavy_weapon_does_not_touch_the_speed_the_pack_costs);
    RUN_TEST(checking_clears_only_the_request_to_look_at_the_weight);

    RUN_TEST(a_light_item_can_be_picked_up_without_changing_speed);
    RUN_TEST(an_item_that_would_slow_the_character_down_is_refused);
    RUN_TEST(the_number_carried_counts_towards_the_weight);
    RUN_TEST(an_item_that_keeps_the_pack_at_the_same_penalty_is_accepted);
    RUN_TEST(an_item_that_pushes_the_pack_to_the_next_penalty_is_refused);
    RUN_TEST(asking_about_an_item_changes_nothing);

    return TEST_SUMMARY();
}
