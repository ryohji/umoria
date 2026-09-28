// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「階級と経験値」のテスト -- 現在のふるまいを保護する
 *
 * py から出す 6 つめの問い（1 つめは持っている金、2 つめは腹の具合、3 つめは
 * 画面に出す数字、4 つめは魔力、5 つめは体力）。フィールドは 5 つあるが
 * 1 つの問いに答えている —— もとは py.misc.lev（階級）・py.misc.exp
 * （経験値）・py.misc.max_exp（これまでの最高）・py.misc.exp_frac（まだ
 * 1 点に届いていない端数）・py.misc.expfact（この人物の値段の係数）。
 *
 * **この単位で新しいのは「5 つのうち 1 つが結論でもある」ということ。**
 * 階級はセーブファイルに書かれる置き場なのに、同時に経験値から決まる:
 *
 *     値段(階級) = player_exp[階級 - 1] * expfact / 100
 *     lev       = 値段をまだ払いきっていない、いちばん下の階級
 *
 * **表の値は「その階級を出る値段」**で、その階級に着く値段ではない
 * （だから読み手はどこでも添字を 1 引く）。
 *
 * その 1 本の約束を、本体は 2 つの違う道で守っていた ——
 * prt_experience()（misc3.c）は 1 段ずつ登り、lose_exp()（spells.c）は
 * 表の下から数えなおす。**2 つが同じ答えを出すことを保証するものが
 * どこにも無かった。** ここに釘を打つのがこのテストの主眼。
 *
 * 保護したい性質は 8 つ。
 *
 *   1. **走りだしは 5 つとも 0**（もとの 5 つは初期化子なしの構造体の中に
 *      あった）。本物の値は create.c が人物を作るときに、あるいは save.c が
 *      ファイルから読んで置く。
 *
 *   2. **値段の式は 1 つだけ**（もとは 5 か所に写されていた）。表の値に
 *      係数を掛けて 100 で割り、**割り算は切りすてる**。
 *
 *   3. **経験値の入りかたは 2 通り。** そのまま足すのと、いまの階級で
 *      割って分けまえを取るの（怪物を倒したとき）。後者は**割りきれない
 *      ぶんを 65536 分の端数で持ちこす** —— でなければ階級の高い人物が
 *      小さい怪物を倒しても永久に 1 点も入らない。
 *
 *   4. **経験値は 0 で下げ止まる**（もとの `if (amount > exp) exp = 0;`）。
 *      端数は触らない。
 *
 *   5. **登りの問いは「いまの階級を出る値段を払ったか」**で、
 *      **ちょうど払いきった時も上がる**（もとの `<=`）。最上位では
 *      経験値がいくらあっても上がらない。
 *
 *   6. **段を上がったあと、払いすぎの半分は捨てる**（もとのコメント
 *      「lose some of the 'extra' exp when gaining several levels at once」）。
 *
 *   7. **数えなおしは表の下から**で、上にも下にも動く（lose_exp の道）。
 *      **表の外を読まないのは MAX_EXP 9999999 と表の最後 10000000 の
 *      1 の差だけが支え。**
 *
 *   8. **最高記録は下がらない**。restore_level はそこへ戻すだけ。
 *
 * テストは 1 プロセスで状態を共有するので、走りだしを見る 1 件は main() の
 * 先頭に置き、以降は各件が最初に窓口で足場を作る。
 */
/* externs.h は要らない。窓口と、int32_t・MAX_EXP・MAX_PLAYER_LEVEL のための
 * types.h と constant.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_level.h"

#include "minunit.h"

/* 値段表。A の段では本体（src/player.c:94）と足場
 * （tests/player_level_fixture.c）が持っているので、テストは直に置く。
 * **#18-12-6C で表が module の中に入るなら、ここは窓口越しになる**
 * （src/hp_table.h の set_hp_total_at_level() と同じ形）。 */
extern uint32_t player_exp[MAX_PLAYER_LEVEL];

/* 5 つをまとめて置く（各テストの足場作り）。 */
static void given(uint16_t level, int32_t exp, int32_t max_exp, uint16_t fraction, uint8_t factor) {
    player_set_level(level);
    player_set_experience(exp);
    player_set_max_experience(max_exp);
    player_set_experience_fraction(fraction);
    player_set_experience_factor(factor);
}

/* 表を空にする。**空の表は値段 0 を意味するので、数えなおしの窓口は
 * 止まらない** —— 数えなおしを呼ぶテストは必ず止まる段を置くこと。 */
static void given_empty_price_list(void) {
    for (int i = 0; i < MAX_PLAYER_LEVEL; i++) {
        player_exp[i] = 0;
    }
}

/* 表の 1 段。level は 1 起点（添字ではない）。 */
static void given_price_to_leave_level(int level, uint32_t entry) {
    player_exp[level - 1] = entry;
}

/* --- 走りだし ------------------------------------------------------------- */
/* この 1 件は main() の先頭で走らせる。ほかのテストが状態を動かすので。 */

TEST(the_five_numbers_start_at_nothing) {
    ASSERT_EQ_INT(0, player_level());
    ASSERT_EQ_INT(0, player_experience());
    ASSERT_EQ_INT(0, player_max_experience());
    ASSERT_EQ_INT(0, player_experience_fraction());
    ASSERT_EQ_INT(0, player_experience_factor());
}

/* 階級の器は 1 バイトでは足りない。**セーブファイルは短整数で読み書きする**
 * （save.c の wr_short / rd_short）ので、255 を越える値も往復できなければ
 * ならない —— 実の上限は 40 だが、wizard の細工は MAX_PLAYER_LEVEL を
 * 足すし、器の幅を狭める変異はこの 1 件でしか捕まらない
 * （#18-12-6C の掃きで分かった）。 */
TEST(the_level_holds_more_than_a_byte) {
    player_set_level(300);

    ASSERT_EQ_INT(300, player_level());
}

/* 5 つが別々の器であること。save.c の読みが取りちがえたら気づけるように
 * （5 つが続けて並んでいるので入れかえが起きやすい）。 */
TEST(each_number_stands_on_its_own) {
    given(7, 1234, 5678, 999, 125);

    ASSERT_EQ_INT(7, player_level());
    ASSERT_EQ_INT(1234, player_experience());
    ASSERT_EQ_INT(5678, player_max_experience());
    ASSERT_EQ_INT(999, player_experience_fraction());
    ASSERT_EQ_INT(125, player_experience_factor());
}

/* --- 値段の式（もとは 5 か所に写されていた） ------------------------------ */

/* 係数 100 なら表の値そのまま。 */
TEST(a_factor_of_a_hundred_is_the_table_entry_itself) {
    given(1, 0, 0, 0, 100);
    given_price_to_leave_level(1, 10);

    ASSERT_EQ_INT(10, player_experience_to_advance_from(1));
}

/* 引数は階級（1 起点）で添字ではない。1 段ずれを捕まえる。 */
TEST(the_argument_is_a_level_not_a_subscript) {
    given(1, 0, 0, 0, 100);
    given_empty_price_list();
    given_price_to_leave_level(1, 10);
    given_price_to_leave_level(2, 25);
    given_price_to_leave_level(3, 45);

    ASSERT_EQ_INT(10, player_experience_to_advance_from(1));
    ASSERT_EQ_INT(25, player_experience_to_advance_from(2));
    ASSERT_EQ_INT(45, player_experience_to_advance_from(3));
}

/* 係数はそのまま百分率。 */
TEST(the_factor_scales_the_price) {
    given(1, 0, 0, 0, 125);
    given_price_to_leave_level(1, 400);

    ASSERT_EQ_INT(500, player_experience_to_advance_from(1));
}

/* 割り算は切りすてる（もとの整数演算どおり）。 */
TEST(the_price_truncates_instead_of_rounding) {
    given(1, 0, 0, 0, 110);
    given_price_to_leave_level(1, 25);

    /* 25 * 110 / 100 = 27.5 → 27 */
    ASSERT_EQ_INT(27, player_experience_to_advance_from(1));
}

/* いちばん高い係数（種族 125 ＋ 職業 40）と表の最後（10000000）でも
 * 符号つき 32 ビットに収まる。1.65e9 < 2.147e9 の釘。 */
TEST(the_dearest_price_still_fits_in_a_signed_number) {
    given(MAX_PLAYER_LEVEL, 0, 0, 0, 165);
    given_price_to_leave_level(MAX_PLAYER_LEVEL, 10000000L);

    ASSERT_EQ_INT(16500000L, player_experience_to_advance_from(MAX_PLAYER_LEVEL));
    ASSERT_TRUE(player_experience_to_advance_from(MAX_PLAYER_LEVEL) > 0);
}

/* 引数なしの窓口はいまの階級を出る値段（人物画面と書きだしの
 * 「Exp to Adv.」）。 */
TEST(the_price_to_advance_is_the_price_of_leaving_the_current_level) {
    given(3, 0, 0, 0, 100);
    given_empty_price_list();
    given_price_to_leave_level(2, 25);
    given_price_to_leave_level(3, 45);

    ASSERT_EQ_INT(45, player_experience_needed_to_advance());
}

/* --- 経験値が入る（そのまま足す） ----------------------------------------- */

TEST(experience_arriving_whole_is_added) {
    given(1, 100, 0, 0, 100);

    player_gain_experience(5);

    ASSERT_EQ_INT(105, player_experience());
}

/* 上限で頭打ちにするのはここではない（登りの先頭で 1 度だけ訊く）。 */
TEST(experience_arriving_whole_is_not_capped) {
    given(1, MAX_EXP, 0, 0, 100);

    player_gain_experience(1000);

    ASSERT_EQ_INT(MAX_EXP + 1000, player_experience());
}

TEST(experience_arriving_whole_leaves_the_fraction_alone) {
    given(1, 100, 0, 777, 100);

    player_gain_experience(5);

    ASSERT_EQ_INT(777, player_experience_fraction());
}

/* --- 経験値が入る（いまの階級で分ける） ----------------------------------- */

/* 割りきれるときは商がそのまま入り、端数は動かない。 */
TEST(a_share_that_divides_evenly_adds_the_whole_quotient) {
    given(4, 0, 0, 0, 100);

    player_gain_shared_experience(40);

    ASSERT_EQ_INT(10, player_experience());
    ASSERT_EQ_INT(0, player_experience_fraction());
}

/* 割りきれないぶんは 65536 分の端数として残る（捨てない）。 */
TEST(the_remainder_of_the_share_is_kept_as_a_fraction) {
    given(4, 0, 0, 0, 100);

    player_gain_shared_experience(42);

    /* 42 / 4 = 10 あまり 2。2 * 65536 / 4 = 32768 */
    ASSERT_EQ_INT(10, player_experience());
    ASSERT_EQ_INT(32768, player_experience_fraction());
}

/* 端数が 1 点に届いたら繰りあがり、余りは持ちこす。 */
TEST(fractions_adding_up_to_a_point_carry_over) {
    given(4, 0, 0, 32768, 100);

    player_gain_shared_experience(42);

    /* 32768 + 32768 = 65536 → 1 点繰りあがって端数は 0 */
    ASSERT_EQ_INT(11, player_experience());
    ASSERT_EQ_INT(0, player_experience_fraction());
}

/* **この単位の主役。** 分けまえが階級より小さくても 1 点も入らないのでは
 * なく、端数が少しずつ進む。 */
TEST(a_share_smaller_than_the_level_still_moves_the_fraction) {
    given(10, 0, 0, 0, 100);

    player_gain_shared_experience(1);

    ASSERT_EQ_INT(0, player_experience());
    /* 1 * 65536 / 10 = 6553 */
    ASSERT_EQ_INT(6553, player_experience_fraction());
}

/* その端数が積みあがれば、いつかは 1 点になる。 */
TEST(small_shares_eventually_add_up_to_a_point) {
    given(10, 0, 0, 0, 100);

    for (int i = 0; i < 10; i++) {
        player_gain_shared_experience(1);
    }

    /* 6553 * 10 = 65530 < 65536 なので、10 回ではまだ届かない */
    ASSERT_EQ_INT(0, player_experience());
    ASSERT_EQ_INT(65530, player_experience_fraction());

    player_gain_shared_experience(1);

    /* 11 回めで越える。65530 + 6553 - 65536 = 6547 */
    ASSERT_EQ_INT(1, player_experience());
    ASSERT_EQ_INT(6547, player_experience_fraction());
}

/* 階級が高いほど分けまえは小さい（÷ lev の釘）。 */
TEST(the_share_is_divided_by_how_far_the_character_has_come) {
    given(2, 0, 0, 0, 100);
    player_gain_shared_experience(100);
    ASSERT_EQ_INT(50, player_experience());

    given(20, 0, 0, 0, 100);
    player_gain_shared_experience(100);
    ASSERT_EQ_INT(5, player_experience());
}

/* --- 経験値が出る --------------------------------------------------------- */

TEST(losing_experience_subtracts) {
    given(5, 100, 100, 0, 100);

    player_lose_experience(30);

    ASSERT_EQ_INT(70, player_experience());
}

/* 0 で下げ止まる（もとの `if (amount > exp) exp = 0;`）。 */
TEST(losing_more_than_there_is_leaves_nothing) {
    given(5, 100, 100, 0, 100);

    player_lose_experience(1000);

    ASSERT_EQ_INT(0, player_experience());
}

/* ちょうど全部は 0 になる（境目が `>` であって `>=` でないことの釘）。 */
TEST(losing_exactly_everything_leaves_nothing) {
    given(5, 100, 100, 0, 100);

    player_lose_experience(100);

    ASSERT_EQ_INT(0, player_experience());
}

/* 端数は残る —— 次の 1 点への道のりを失わない。 */
TEST(losing_experience_leaves_the_fraction_alone) {
    given(5, 100, 100, 4321, 100);

    player_lose_experience(1000);

    ASSERT_EQ_INT(4321, player_experience_fraction());
}

/* 下げ止まらずに引けたときも端数は残る。**変異の掃きで見つけた穴**
 * （上の 1 件は 0 で止まる側の枝しか通っていなかったので、else の枝で
 * 端数を消す変異が生き残った）。 */
TEST(losing_part_of_the_experience_leaves_the_fraction_alone) {
    given(5, 100, 100, 4321, 100);

    player_lose_experience(30);

    ASSERT_EQ_INT(70, player_experience());
    ASSERT_EQ_INT(4321, player_experience_fraction());
}

/* 最高記録は下がらない。 */
TEST(losing_experience_does_not_touch_the_high_water_mark) {
    given(5, 100, 100, 0, 100);

    player_lose_experience(100);

    ASSERT_EQ_INT(100, player_max_experience());
}

/* --- 上限 ---------------------------------------------------------------- */

TEST(the_ceiling_bites_above_the_maximum) {
    given(5, MAX_EXP + 1, 0, 0, 100);

    ASSERT_TRUE(player_cap_experience());
    ASSERT_EQ_INT(MAX_EXP, player_experience());
}

/* ちょうど上限は触らない（`>` であって `>=` でないことの釘）。 */
TEST(exactly_the_maximum_is_left_alone) {
    given(5, MAX_EXP, 0, 0, 100);

    ASSERT_TRUE(!player_cap_experience());
    ASSERT_EQ_INT(MAX_EXP, player_experience());
}

/* --- 登り（prt_experience の道） ------------------------------------------ */

TEST(an_unpaid_price_does_not_deserve_the_next_level) {
    given(1, 9, 0, 0, 100);
    given_price_to_leave_level(1, 10);

    ASSERT_TRUE(!player_deserves_next_level());
}

/* ちょうど払いきった時も上がる（もとの `<=`）。 */
TEST(exactly_paying_the_price_deserves_the_next_level) {
    given(1, 10, 0, 0, 100);
    given_price_to_leave_level(1, 10);

    ASSERT_TRUE(player_deserves_next_level());
}

/* 見るのはいまの階級を出る値段（1 段ずれの釘）。 */
TEST(the_climb_looks_at_the_price_of_leaving_the_current_level) {
    given(2, 25, 0, 0, 100);
    given_empty_price_list();
    given_price_to_leave_level(1, 10);
    given_price_to_leave_level(2, 25);
    given_price_to_leave_level(3, 45);

    ASSERT_TRUE(player_deserves_next_level());

    player_set_experience(24);
    ASSERT_TRUE(!player_deserves_next_level());
}

/* 最上位では経験値がいくらあっても上がらない。 */
TEST(the_top_level_never_deserves_another) {
    given(MAX_PLAYER_LEVEL, MAX_EXP, 0, 0, 100);
    given_empty_price_list();

    ASSERT_TRUE(!player_deserves_next_level());
}

TEST(advancing_raises_the_level_by_one) {
    given(3, 0, 0, 0, 100);

    player_advance_level();

    ASSERT_EQ_INT(4, player_level());
}

/* 払いすぎの半分は捨てる（何段も一度に上がったとき）。 */
TEST(half_of_any_surplus_is_thrown_away) {
    given(2, 45, 0, 0, 100);
    given_empty_price_list();
    given_price_to_leave_level(2, 25);

    player_trim_surplus_experience();

    /* 45 - 25 = 20 の半分を捨てて 25 + 10 */
    ASSERT_EQ_INT(35, player_experience());
}

TEST(without_a_surplus_nothing_is_thrown_away) {
    given(2, 20, 0, 0, 100);
    given_empty_price_list();
    given_price_to_leave_level(2, 25);

    player_trim_surplus_experience();

    ASSERT_EQ_INT(20, player_experience());
}

/* ちょうど値段のときも触らない（`>` であって `>=` でないことの釘）。 */
TEST(exactly_the_price_is_left_alone_by_the_trim) {
    given(2, 25, 0, 0, 100);
    given_empty_price_list();
    given_price_to_leave_level(2, 25);

    player_trim_surplus_experience();

    ASSERT_EQ_INT(25, player_experience());
}

/* --- 数えなおし（lose_exp の道） ------------------------------------------ */

/* 1 段めの値段も払えていなければ階級 1。 */
TEST(an_experience_below_the_first_price_deserves_level_one) {
    given(9, 9, 0, 0, 100);
    given_empty_price_list();
    given_price_to_leave_level(1, 10);
    /* 2 段めにも止まる値を置く。**数えあげの出発点を 1 から 2 にずらす変異が
     * 「表の外を読んで落ちる」のではなく「答えが 2 になる」で捕まるように**
     * ―― 落ちるのも赤だが、読める赤のほうが良い。 */
    given_price_to_leave_level(2, 10000000L);

    ASSERT_EQ_INT(1, player_level_deserved_by_experience());
}

/* 表を下から数えあげる。 */
TEST(the_recount_walks_up_the_price_list) {
    given(1, 44, 0, 0, 100);
    given_empty_price_list();
    given_price_to_leave_level(1, 10);
    given_price_to_leave_level(2, 25);
    given_price_to_leave_level(3, 45);
    given_price_to_leave_level(4, 70);

    /* 10 と 25 は払えていて 45 は払えていない → 階級 3 */
    ASSERT_EQ_INT(3, player_level_deserved_by_experience());
}

/* ちょうど払いきった段は「出られる」と数える（`<=` の釘）。 */
TEST(exactly_paying_a_price_counts_as_leaving_that_level) {
    given(1, 45, 0, 0, 100);
    given_empty_price_list();
    given_price_to_leave_level(1, 10);
    given_price_to_leave_level(2, 25);
    given_price_to_leave_level(3, 45);
    given_price_to_leave_level(4, 70);

    ASSERT_EQ_INT(4, player_level_deserved_by_experience());
}

/* **表の外を読まないことの釘。** 本物の表と本物の上限で、最上位まで
 * 数えあげても止まる —— 支えているのは MAX_EXP 9999999 と表の最後
 * 10000000 の 1 の差だけ。 */
TEST(the_recount_stops_inside_the_price_list_at_the_ceiling) {
    given(1, MAX_EXP, 0, 0, 100);
    for (int i = 0; i < MAX_PLAYER_LEVEL; i++) {
        /* 本物の表の最後は 10000000。手前は全部それより安い。 */
        player_exp[i] = (i == MAX_PLAYER_LEVEL - 1) ? 10000000L : 1000L * (uint32_t)(i + 1);
    }

    ASSERT_EQ_INT(MAX_PLAYER_LEVEL, player_level_deserved_by_experience());
}

TEST(the_recount_can_raise_the_level) {
    given(1, 45, 0, 0, 100);
    given_empty_price_list();
    given_price_to_leave_level(1, 10);
    given_price_to_leave_level(2, 25);
    given_price_to_leave_level(3, 45);
    given_price_to_leave_level(4, 70);

    ASSERT_TRUE(player_recompute_level());
    ASSERT_EQ_INT(4, player_level());
}

/* lose_exp の本来の向き —— 下がる。 */
TEST(the_recount_can_lower_the_level) {
    given(4, 9, 0, 0, 100);
    given_empty_price_list();
    given_price_to_leave_level(1, 10);
    given_price_to_leave_level(2, 25);
    given_price_to_leave_level(3, 45);

    ASSERT_TRUE(player_recompute_level());
    ASSERT_EQ_INT(1, player_level());
}

/* 動かなかったときは false（呼び手はそれで画面を書きなおさない）。 */
TEST(a_level_that_already_agrees_does_not_move) {
    given(3, 30, 0, 0, 100);
    given_empty_price_list();
    given_price_to_leave_level(1, 10);
    given_price_to_leave_level(2, 25);
    given_price_to_leave_level(3, 45);

    ASSERT_TRUE(!player_recompute_level());
    ASSERT_EQ_INT(3, player_level());
}

/* 係数が上がれば同じ経験値でも階級は下がる（値段の式を通っている証拠）。 */
TEST(the_recount_goes_through_the_factor) {
    given_empty_price_list();
    given_price_to_leave_level(1, 10);
    given_price_to_leave_level(2, 25);
    given_price_to_leave_level(3, 45);

    given(1, 25, 0, 0, 100);
    ASSERT_EQ_INT(3, player_level_deserved_by_experience());

    given(1, 25, 0, 0, 200);
    /* 値段が 20 と 50 になるので、25 では 2 段めから出られない */
    ASSERT_EQ_INT(2, player_level_deserved_by_experience());
}

/* --- 最高記録 ------------------------------------------------------------- */

TEST(a_new_high_water_mark_is_recorded) {
    given(5, 200, 100, 0, 100);

    ASSERT_TRUE(player_record_max_experience());
    ASSERT_EQ_INT(200, player_max_experience());
}

/* 同じ値では記録しない（`>` であって `>=` でないことの釘）。 */
TEST(an_equal_experience_is_not_a_new_mark) {
    given(5, 100, 100, 0, 100);

    ASSERT_TRUE(!player_record_max_experience());
    ASSERT_EQ_INT(100, player_max_experience());
}

TEST(the_high_water_mark_never_comes_down) {
    given(5, 50, 100, 0, 100);

    ASSERT_TRUE(!player_record_max_experience());
    ASSERT_EQ_INT(100, player_max_experience());
}

TEST(restoring_puts_the_experience_back_to_the_mark) {
    given(5, 50, 100, 0, 100);

    ASSERT_TRUE(player_restore_experience());
    ASSERT_EQ_INT(100, player_experience());
}

/* 戻すものが無ければ false（呼び手はそれで「生気が戻る」と言わない）。 */
TEST(there_is_nothing_to_restore_at_the_mark) {
    given(5, 100, 100, 0, 100);

    ASSERT_TRUE(!player_restore_experience());
    ASSERT_EQ_INT(100, player_experience());
}

/* 階級はここでは動かない —— 呼び手が数えなおす。 */
TEST(restoring_does_not_move_the_level_by_itself) {
    given(5, 50, 100, 0, 100);

    (void)player_restore_experience();

    ASSERT_EQ_INT(5, player_level());
}

/* --- 素の置きかえ（セーブファイルと wizard の細工） ----------------------- */

/* **約束をわざと壊せること。** wizard の「死なない」細工は階級に
 * MAX_PLAYER_LEVEL を足し、経験値に 500 万を足す —— 数えなおすまで
 * 階級と経験値は食いちがったままで、それが正しい。 */
TEST(a_bare_setter_can_break_the_promise_on_purpose) {
    given(1, 0, 0, 0, 100);
    given_empty_price_list();
    given_price_to_leave_level(1, 10);
    /* 数えなおしを呼ぶので、止まる段を必ず置く。**空の表のままだと
     * 数えなおしは表の外を読んで落ちる** —— 本体が落ちないのは実の表が
     * 単調に増えていて最後が MAX_EXP より高いからで、それだけが支え
     * （src/player_level.c の player_level_deserved_by_experience() の
     * コメント）。 */
    given_price_to_leave_level(2, 10000000L);

    player_set_level((uint16_t)(player_level() + MAX_PLAYER_LEVEL));
    player_set_max_experience(player_max_experience() + 5000000L);
    player_set_experience(player_max_experience());

    ASSERT_EQ_INT(MAX_PLAYER_LEVEL + 1, player_level());
    ASSERT_EQ_INT(5000000L, player_experience());
    /* 経験値が値段 10 を軽く越えているのに、階級は数えなおしの答え（2）と
     * 一致していない —— wizard の細工はそれが正しい。 */
    ASSERT_EQ_INT(2, player_level_deserved_by_experience());
    ASSERT_TRUE(player_level() != player_level_deserved_by_experience());
}

int main(void) {
    RUN_TEST(the_five_numbers_start_at_nothing);
    RUN_TEST(the_level_holds_more_than_a_byte);
    RUN_TEST(each_number_stands_on_its_own);

    RUN_TEST(a_factor_of_a_hundred_is_the_table_entry_itself);
    RUN_TEST(the_argument_is_a_level_not_a_subscript);
    RUN_TEST(the_factor_scales_the_price);
    RUN_TEST(the_price_truncates_instead_of_rounding);
    RUN_TEST(the_dearest_price_still_fits_in_a_signed_number);
    RUN_TEST(the_price_to_advance_is_the_price_of_leaving_the_current_level);

    RUN_TEST(experience_arriving_whole_is_added);
    RUN_TEST(experience_arriving_whole_is_not_capped);
    RUN_TEST(experience_arriving_whole_leaves_the_fraction_alone);

    RUN_TEST(a_share_that_divides_evenly_adds_the_whole_quotient);
    RUN_TEST(the_remainder_of_the_share_is_kept_as_a_fraction);
    RUN_TEST(fractions_adding_up_to_a_point_carry_over);
    RUN_TEST(a_share_smaller_than_the_level_still_moves_the_fraction);
    RUN_TEST(small_shares_eventually_add_up_to_a_point);
    RUN_TEST(the_share_is_divided_by_how_far_the_character_has_come);

    RUN_TEST(losing_experience_subtracts);
    RUN_TEST(losing_more_than_there_is_leaves_nothing);
    RUN_TEST(losing_exactly_everything_leaves_nothing);
    RUN_TEST(losing_experience_leaves_the_fraction_alone);
    RUN_TEST(losing_part_of_the_experience_leaves_the_fraction_alone);
    RUN_TEST(losing_experience_does_not_touch_the_high_water_mark);

    RUN_TEST(the_ceiling_bites_above_the_maximum);
    RUN_TEST(exactly_the_maximum_is_left_alone);

    RUN_TEST(an_unpaid_price_does_not_deserve_the_next_level);
    RUN_TEST(exactly_paying_the_price_deserves_the_next_level);
    RUN_TEST(the_climb_looks_at_the_price_of_leaving_the_current_level);
    RUN_TEST(the_top_level_never_deserves_another);
    RUN_TEST(advancing_raises_the_level_by_one);
    RUN_TEST(half_of_any_surplus_is_thrown_away);
    RUN_TEST(without_a_surplus_nothing_is_thrown_away);
    RUN_TEST(exactly_the_price_is_left_alone_by_the_trim);

    RUN_TEST(an_experience_below_the_first_price_deserves_level_one);
    RUN_TEST(the_recount_walks_up_the_price_list);
    RUN_TEST(exactly_paying_a_price_counts_as_leaving_that_level);
    RUN_TEST(the_recount_stops_inside_the_price_list_at_the_ceiling);
    RUN_TEST(the_recount_can_raise_the_level);
    RUN_TEST(the_recount_can_lower_the_level);
    RUN_TEST(a_level_that_already_agrees_does_not_move);
    RUN_TEST(the_recount_goes_through_the_factor);

    RUN_TEST(a_new_high_water_mark_is_recorded);
    RUN_TEST(an_equal_experience_is_not_a_new_mark);
    RUN_TEST(the_high_water_mark_never_comes_down);
    RUN_TEST(restoring_puts_the_experience_back_to_the_mark);
    RUN_TEST(there_is_nothing_to_restore_at_the_mark);
    RUN_TEST(restoring_does_not_move_the_level_by_itself);

    RUN_TEST(a_bare_setter_can_break_the_promise_on_purpose);

    return TEST_SUMMARY();
}
