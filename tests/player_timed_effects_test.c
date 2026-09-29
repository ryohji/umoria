// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「一時的な状態の数えおとし」のテスト -- 現在のふるまいを保護する
 *
 * py から出す 9 つめの問い（1 つめは持っている金、2 つめは腹の具合、3 つめは
 * 画面に出す数字、4 つめは魔力、5 つめは体力、6 つめは階級と経験値、7 つめは
 * 状態の旗、8 つめは装備で決まる耐性・能力）。答えは 18 個 —— もとは
 * py.flags.blind から py.flags.tim_infra まで、18 ファイルから 281 か所が
 * 触っていた —— で、#18-12-9C からは src/player/player_timed_effects.c の中の配列
 * 1 つだけが持っている。
 *
 * **この単位で新しいのは「module が別の module を呼ぶ」こと。** 印
 * （player_status_flags.c の族 1）と数は 1 つの事実の両半分で、**どの印が
 * どの数と対か**を知っているのはこの module だけ。だからテストの重心は 3 つ:
 *
 *   1. **印と数の対応を 1 行ずつ釘打つ。** 12 個が対になっていて、表の 2 行を
 *      入れかえると「目が見えない」を「混乱した」と宣言する。1 個だけ始めて
 *      **旗の 1 語まるごと**を見る形を 12 回くりかえす（PY_* をここで名指しする
 *      のは意図的 —— 呼び手からは消すが、対応を守るテストはどこかで名前を
 *      言わなければならない）。
 *   2. **18 個は 18 個。** 1 つの配列に並んでいるので添字の取りちがえは隣を
 *      巻きこむ。1 個だけ動かして 18 個ぜんぶの「効いているか」を見る。
 *   3. **数は 0 を通りすぎて負になる。** dungeon.c は英雄状態で新しく怖がった
 *      人の恐れを 0 に置き、同じターンの数えおとしがそれを **-1** にする ——
 *      終わりの message も印の消しも起きず、-1 は残りつづけて次の恐れを
 *      1 ターン短くする。**「0 で止める」ように直すとふるまいが変わる**ので、
 *      その形に 4 本の釘を打つ。
 *
 * テストは 1 プロセスで状態を共有するので、各件が最初に 18 個の数と旗の 1 語を
 * 置きなおす。
 */
/* externs.h は要らない。窓口 2 本と、PY_* のための constant.h・types.h だけで
 * 足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_status_flags.h"
#include "player_timed_effects.h"

#include "minunit.h"

#define ONLY(effect) (1L << (effect))
#define ALL_EIGHTEEN ((1L << PLAYER_TIMED_COUNT) - 1)

/* 18 個の「効いているか」を 1 語に詰めなおしたもの。**これが「1 個だけ動かして
 * 隣を巻きこんでいないか」を見る道具。** */
static long clocks_in_force(void) {
    long in_force = 0;

    for (int effect = 0; effect < PLAYER_TIMED_COUNT; effect++) {
        if (player_timed_in_force((player_timed_effect)effect)) {
            in_force |= ONLY(effect);
        }
    }

    return in_force;
}

static void given_no_state_at_all(void) {
    for (int effect = 0; effect < PLAYER_TIMED_COUNT; effect++) {
        player_timed_set((player_timed_effect)effect, 0);
    }
    /* 印の側も消す —— 数と印は別の module なので、数を 0 にしても印は残る
     * （それ自身がこの単位のふるまいで、下の「消しても印は残る」で釘を打つ）。 */
    player_set_status_word(0);
}

/* 1 個だけ長さを置いて、18 個の「効いているか」を見る。 */
static long in_force_when_only(player_timed_effect effect) {
    given_no_state_at_all();
    player_timed_add(effect, 7);

    return clocks_in_force();
}

/* 1 個だけ始めて、旗の 1 語まるごとを見る。 */
static uint32_t word_when_beginning(player_timed_effect effect) {
    given_no_state_at_all();
    player_timed_add(effect, 5);
    (void)player_timed_beginning(effect);

    return player_status_word();
}

/* --- 18 個は 18 個 ------------------------------------------------------- */

TEST(the_blindness_clock_is_its_own) {
    ASSERT_EQ_INT(in_force_when_only(PLAYER_TIMED_BLINDNESS), ONLY(PLAYER_TIMED_BLINDNESS));
}

TEST(the_paralysis_clock_is_its_own) {
    ASSERT_EQ_INT(in_force_when_only(PLAYER_TIMED_PARALYSIS), ONLY(PLAYER_TIMED_PARALYSIS));
}

TEST(the_confusion_clock_is_its_own) {
    ASSERT_EQ_INT(in_force_when_only(PLAYER_TIMED_CONFUSION), ONLY(PLAYER_TIMED_CONFUSION));
}

TEST(the_haste_clock_is_its_own) {
    ASSERT_EQ_INT(in_force_when_only(PLAYER_TIMED_HASTE), ONLY(PLAYER_TIMED_HASTE));
}

TEST(the_slowness_clock_is_its_own) {
    ASSERT_EQ_INT(in_force_when_only(PLAYER_TIMED_SLOWNESS), ONLY(PLAYER_TIMED_SLOWNESS));
}

TEST(the_fear_clock_is_its_own) {
    ASSERT_EQ_INT(in_force_when_only(PLAYER_TIMED_FEAR), ONLY(PLAYER_TIMED_FEAR));
}

TEST(the_poison_clock_is_its_own) {
    ASSERT_EQ_INT(in_force_when_only(PLAYER_TIMED_POISON), ONLY(PLAYER_TIMED_POISON));
}

TEST(the_hallucination_clock_is_its_own) {
    ASSERT_EQ_INT(in_force_when_only(PLAYER_TIMED_HALLUCINATION), ONLY(PLAYER_TIMED_HALLUCINATION));
}

TEST(the_protection_from_evil_clock_is_its_own) {
    ASSERT_EQ_INT(in_force_when_only(PLAYER_TIMED_PROTECTION_FROM_EVIL), ONLY(PLAYER_TIMED_PROTECTION_FROM_EVIL));
}

TEST(the_invulnerability_clock_is_its_own) {
    ASSERT_EQ_INT(in_force_when_only(PLAYER_TIMED_INVULNERABILITY), ONLY(PLAYER_TIMED_INVULNERABILITY));
}

TEST(the_heroism_clock_is_its_own) {
    ASSERT_EQ_INT(in_force_when_only(PLAYER_TIMED_HEROISM), ONLY(PLAYER_TIMED_HEROISM));
}

TEST(the_super_heroism_clock_is_its_own) {
    ASSERT_EQ_INT(in_force_when_only(PLAYER_TIMED_SUPER_HEROISM), ONLY(PLAYER_TIMED_SUPER_HEROISM));
}

TEST(the_blessing_clock_is_its_own) {
    ASSERT_EQ_INT(in_force_when_only(PLAYER_TIMED_BLESSING), ONLY(PLAYER_TIMED_BLESSING));
}

TEST(the_heat_resistance_clock_is_its_own) {
    ASSERT_EQ_INT(in_force_when_only(PLAYER_TIMED_HEAT_RESISTANCE), ONLY(PLAYER_TIMED_HEAT_RESISTANCE));
}

TEST(the_cold_resistance_clock_is_its_own) {
    ASSERT_EQ_INT(in_force_when_only(PLAYER_TIMED_COLD_RESISTANCE), ONLY(PLAYER_TIMED_COLD_RESISTANCE));
}

TEST(the_seeing_invisible_clock_is_its_own) {
    ASSERT_EQ_INT(in_force_when_only(PLAYER_TIMED_SEEING_INVISIBLE), ONLY(PLAYER_TIMED_SEEING_INVISIBLE));
}

TEST(the_word_of_recall_clock_is_its_own) {
    ASSERT_EQ_INT(in_force_when_only(PLAYER_TIMED_WORD_OF_RECALL), ONLY(PLAYER_TIMED_WORD_OF_RECALL));
}

TEST(the_infra_vision_clock_is_its_own) {
    ASSERT_EQ_INT(in_force_when_only(PLAYER_TIMED_INFRA_VISION), ONLY(PLAYER_TIMED_INFRA_VISION));
}

TEST(there_are_eighteen_clocks) {
    ASSERT_EQ_INT(PLAYER_TIMED_COUNT, 18);
}

/* --- 置き場との対応 ------------------------------------------------------ */

/* A の段では py.flags のフィールドを名前で突きあわせていた（窓口と置き場の間に
 * 表が 1 枚あり、2 行を入れかえても窓口だけを見るテストでは気づけなかった）。
 * #18-12-9C で置き場が static な 18 個の並びになり、表は無くなったので、フィー
 * ルドの名前はもう出てこない。**それでも 18 個にそれぞれ違う長さを置いて一度に
 * 見る釘は残す**：番地の出しかたが 1 つずれれば、これだけが気づく。 */
static long clocks_holding_their_own_length(void) {
    long right = 0;

    for (int effect = 0; effect < PLAYER_TIMED_COUNT; effect++) {
        if (player_timed_turns((player_timed_effect)effect) == 1 + effect) {
            right |= ONLY(effect);
        }
    }

    return right;
}

TEST(each_clock_keeps_the_length_it_was_given) {
    given_no_state_at_all();

    for (int effect = 0; effect < PLAYER_TIMED_COUNT; effect++) {
        player_timed_set((player_timed_effect)effect, 1 + effect);
    }

    ASSERT_EQ_INT(clocks_holding_their_own_length(), ALL_EIGHTEEN);
}

/* --- 数を読む ----------------------------------------------------------- */

TEST(a_clock_at_zero_is_not_in_force) {
    given_no_state_at_all();

    ASSERT_FALSE(player_timed_in_force(PLAYER_TIMED_BLINDNESS));
}

TEST(a_clock_below_zero_is_not_in_force) {
    given_no_state_at_all();
    player_timed_set(PLAYER_TIMED_FEAR, -1);

    ASSERT_FALSE(player_timed_in_force(PLAYER_TIMED_FEAR));
}

TEST(one_turn_left_is_in_force) {
    given_no_state_at_all();
    player_timed_set(PLAYER_TIMED_BLINDNESS, 1);

    ASSERT_TRUE(player_timed_in_force(PLAYER_TIMED_BLINDNESS));
}

/* 「効いているか」だけでは足りない 4 か所のため（装置の成功率・「Paralysed」の
 * 1 との比べ・祈りの 3 との比べ・セーブの書き）。 */
TEST(turns_answers_the_number_that_was_put_in) {
    given_no_state_at_all();
    player_timed_set(PLAYER_TIMED_CONFUSION, 23);

    ASSERT_EQ_INT(player_timed_turns(PLAYER_TIMED_CONFUSION), 23);
}

TEST(turns_answers_a_negative_number_too) {
    given_no_state_at_all();
    player_timed_set(PLAYER_TIMED_FEAR, -1);

    ASSERT_EQ_INT(player_timed_turns(PLAYER_TIMED_FEAR), -1);
}

/* --- 長さを足す・置く・消す・縮める -------------------------------------- */

/* 原因はふつう足す —— 混乱させるモンスターに 2 度噛まれれば長く続く。 */
TEST(adding_turns_makes_the_state_last_longer) {
    given_no_state_at_all();
    player_timed_add(PLAYER_TIMED_CONFUSION, 5);
    player_timed_add(PLAYER_TIMED_CONFUSION, 3);

    ASSERT_EQ_INT(player_timed_turns(PLAYER_TIMED_CONFUSION), 8);
}

TEST(adding_to_a_clock_below_zero_starts_from_there) {
    given_no_state_at_all();
    player_timed_set(PLAYER_TIMED_FEAR, -1);
    player_timed_add(PLAYER_TIMED_FEAR, 3);

    ASSERT_EQ_INT(player_timed_turns(PLAYER_TIMED_FEAR), 2);
}

TEST(setting_turns_throws_away_what_was_left) {
    given_no_state_at_all();
    player_timed_add(PLAYER_TIMED_PARALYSIS, 30);
    player_timed_set(PLAYER_TIMED_PARALYSIS, 5);

    ASSERT_EQ_INT(player_timed_turns(PLAYER_TIMED_PARALYSIS), 5);
}

TEST(clearing_leaves_no_turns) {
    given_no_state_at_all();
    player_timed_add(PLAYER_TIMED_FEAR, 30);
    player_timed_clear(PLAYER_TIMED_FEAR);

    ASSERT_EQ_INT(player_timed_turns(PLAYER_TIMED_FEAR), 0);
}

TEST(clearing_one_clock_leaves_the_others_running) {
    given_no_state_at_all();
    player_timed_add(PLAYER_TIMED_FEAR, 30);
    player_timed_add(PLAYER_TIMED_POISON, 30);
    player_timed_clear(PLAYER_TIMED_FEAR);

    ASSERT_EQ_INT(clocks_in_force(), ONLY(PLAYER_TIMED_POISON));
}

/* 治療は 1 ターン残す —— 終わりの message と印の消しは数えおとしから出るので、
 * 0 を置いてしまうと「気分が良くなった」が出ない。 */
TEST(shortening_cuts_a_long_state_down) {
    given_no_state_at_all();
    player_timed_add(PLAYER_TIMED_POISON, 30);
    player_timed_shorten_to(PLAYER_TIMED_POISON, 1);

    ASSERT_EQ_INT(player_timed_turns(PLAYER_TIMED_POISON), 1);
}

TEST(shortening_never_lengthens_a_short_state) {
    given_no_state_at_all();
    player_timed_set(PLAYER_TIMED_POISON, 1);
    player_timed_shorten_to(PLAYER_TIMED_POISON, 10);

    ASSERT_EQ_INT(player_timed_turns(PLAYER_TIMED_POISON), 1);
}

/* 効いていない状態を治しても始まらない（もとの `if (poisoned > 1)` と同じ）。 */
TEST(shortening_a_state_that_is_not_in_force_starts_nothing) {
    given_no_state_at_all();
    player_timed_shorten_to(PLAYER_TIMED_POISON, 1);

    ASSERT_EQ_INT(player_timed_turns(PLAYER_TIMED_POISON), 0);
}

TEST(shortening_to_its_own_length_changes_nothing) {
    given_no_state_at_all();
    player_timed_set(PLAYER_TIMED_POISON, 1);
    player_timed_shorten_to(PLAYER_TIMED_POISON, 1);

    ASSERT_EQ_INT(player_timed_turns(PLAYER_TIMED_POISON), 1);
}

/* --- どの印がどの数と対か（この module だけが知っている 12 行） ---------- */

/* 12 件とも「1 個だけ始めて旗の 1 語まるごとを見る」形。**期待値は 1 つの
 * PY_* ちょうど**で、表の行を入れかえれば必ずどれかが落ちる。 */

TEST(beginning_blindness_marks_blindness_and_nothing_else) {
    ASSERT_EQ_INT(word_when_beginning(PLAYER_TIMED_BLINDNESS), PY_BLIND);
}

TEST(beginning_confusion_marks_confusion_and_nothing_else) {
    ASSERT_EQ_INT(word_when_beginning(PLAYER_TIMED_CONFUSION), PY_CONFUSED);
}

TEST(beginning_haste_marks_haste_and_nothing_else) {
    ASSERT_EQ_INT(word_when_beginning(PLAYER_TIMED_HASTE), PY_FAST);
}

TEST(beginning_slowness_marks_slowness_and_nothing_else) {
    ASSERT_EQ_INT(word_when_beginning(PLAYER_TIMED_SLOWNESS), PY_SLOW);
}

TEST(beginning_fear_marks_fear_and_nothing_else) {
    ASSERT_EQ_INT(word_when_beginning(PLAYER_TIMED_FEAR), PY_FEAR);
}

TEST(beginning_poison_marks_poison_and_nothing_else) {
    ASSERT_EQ_INT(word_when_beginning(PLAYER_TIMED_POISON), PY_POISONED);
}

TEST(beginning_invulnerability_marks_invulnerability_and_nothing_else) {
    ASSERT_EQ_INT(word_when_beginning(PLAYER_TIMED_INVULNERABILITY), PY_INVULN);
}

TEST(beginning_heroism_marks_heroism_and_nothing_else) {
    ASSERT_EQ_INT(word_when_beginning(PLAYER_TIMED_HEROISM), PY_HERO);
}

TEST(beginning_super_heroism_marks_super_heroism_and_nothing_else) {
    ASSERT_EQ_INT(word_when_beginning(PLAYER_TIMED_SUPER_HEROISM), PY_SHERO);
}

TEST(beginning_blessing_marks_blessing_and_nothing_else) {
    ASSERT_EQ_INT(word_when_beginning(PLAYER_TIMED_BLESSING), PY_BLESSED);
}

TEST(beginning_seeing_invisible_marks_seeing_invisible_and_nothing_else) {
    ASSERT_EQ_INT(word_when_beginning(PLAYER_TIMED_SEEING_INVISIBLE), PY_DET_INV);
}

TEST(beginning_infra_vision_marks_infra_vision_and_nothing_else) {
    ASSERT_EQ_INT(word_when_beginning(PLAYER_TIMED_INFRA_VISION), PY_TIM_INFRA);
}

/* --- 印を持たない 6 個 --------------------------------------------------- */

/* 麻痺・幻覚・邪悪からの守り・熱と冷気への耐性・帰還の言葉。**印が無いのに
 * 旗の 1 語のどこかを立てるのが一番怖い間違い**（PY_PARALYSED は族 4 ——
 * 「状態表示行がいま何を出しているか」の記録で、状態そのものではない）ので、
 * 6 個それぞれで「始まりは無い」と言わせる。 */

static bool no_mark_clock_claims_a_beginning(player_timed_effect effect) {
    given_no_state_at_all();
    player_timed_add(effect, 5);

    return player_timed_beginning(effect) || player_status_word() != 0;
}

TEST(paralysis_has_no_beginning_to_announce) {
    ASSERT_FALSE(no_mark_clock_claims_a_beginning(PLAYER_TIMED_PARALYSIS));
}

TEST(hallucination_has_no_beginning_to_announce) {
    ASSERT_FALSE(no_mark_clock_claims_a_beginning(PLAYER_TIMED_HALLUCINATION));
}

TEST(protection_from_evil_has_no_beginning_to_announce) {
    ASSERT_FALSE(no_mark_clock_claims_a_beginning(PLAYER_TIMED_PROTECTION_FROM_EVIL));
}

TEST(heat_resistance_has_no_beginning_to_announce) {
    ASSERT_FALSE(no_mark_clock_claims_a_beginning(PLAYER_TIMED_HEAT_RESISTANCE));
}

TEST(cold_resistance_has_no_beginning_to_announce) {
    ASSERT_FALSE(no_mark_clock_claims_a_beginning(PLAYER_TIMED_COLD_RESISTANCE));
}

TEST(word_of_recall_has_no_beginning_to_announce) {
    ASSERT_FALSE(no_mark_clock_claims_a_beginning(PLAYER_TIMED_WORD_OF_RECALL));
}

/* 印を持たない 6 個も数は数える —— 帰還の言葉は 1 で発動し、麻痺は 0 まで
 * 降りて解ける。 */
TEST(a_clock_without_a_mark_still_runs_out) {
    given_no_state_at_all();
    player_timed_set(PLAYER_TIMED_PARALYSIS, 1);

    ASSERT_TRUE(player_timed_count_down(PLAYER_TIMED_PARALYSIS));
}

/* --- 始まり ------------------------------------------------------------- */

TEST(a_beginning_is_due_the_first_time) {
    given_no_state_at_all();
    player_timed_add(PLAYER_TIMED_BLINDNESS, 10);

    ASSERT_TRUE(player_timed_beginning(PLAYER_TIMED_BLINDNESS));
}

/* dungeon.c が毎ターン聞くので、2 度めからは「もう言った」でなければ message が
 * 毎ターン出る。 */
TEST(a_beginning_is_not_due_again_on_the_next_turn) {
    given_no_state_at_all();
    player_timed_add(PLAYER_TIMED_BLINDNESS, 10);
    (void)player_timed_beginning(PLAYER_TIMED_BLINDNESS);

    ASSERT_FALSE(player_timed_beginning(PLAYER_TIMED_BLINDNESS));
}

/* 長くしても始まりは 1 度だけ —— 続いているあいだに噛まれても「目が見えない」は
 * 出ない。 */
TEST(adding_turns_to_a_state_already_begun_brings_no_new_beginning) {
    given_no_state_at_all();
    player_timed_add(PLAYER_TIMED_CONFUSION, 10);
    (void)player_timed_beginning(PLAYER_TIMED_CONFUSION);
    player_timed_add(PLAYER_TIMED_CONFUSION, 10);

    ASSERT_FALSE(player_timed_beginning(PLAYER_TIMED_CONFUSION));
}

TEST(a_beginning_does_not_move_the_clock) {
    given_no_state_at_all();
    player_timed_add(PLAYER_TIMED_BLINDNESS, 10);
    (void)player_timed_beginning(PLAYER_TIMED_BLINDNESS);

    ASSERT_EQ_INT(player_timed_turns(PLAYER_TIMED_BLINDNESS), 10);
}

/* --- 1 ターン過ぎる ----------------------------------------------------- */

TEST(one_turn_takes_one_turn_off) {
    given_no_state_at_all();
    player_timed_set(PLAYER_TIMED_BLINDNESS, 10);
    (void)player_timed_count_down(PLAYER_TIMED_BLINDNESS);

    ASSERT_EQ_INT(player_timed_turns(PLAYER_TIMED_BLINDNESS), 9);
}

TEST(no_ending_is_due_while_turns_remain) {
    given_no_state_at_all();
    player_timed_set(PLAYER_TIMED_BLINDNESS, 10);

    ASSERT_FALSE(player_timed_count_down(PLAYER_TIMED_BLINDNESS));
}

TEST(the_ending_is_due_on_the_turn_the_last_one_is_used) {
    given_no_state_at_all();
    player_timed_set(PLAYER_TIMED_BLINDNESS, 1);

    ASSERT_TRUE(player_timed_count_down(PLAYER_TIMED_BLINDNESS));
}

TEST(running_out_puts_the_mark_away) {
    given_no_state_at_all();
    player_timed_set(PLAYER_TIMED_BLINDNESS, 1);
    (void)player_timed_beginning(PLAYER_TIMED_BLINDNESS);
    (void)player_timed_count_down(PLAYER_TIMED_BLINDNESS);

    ASSERT_EQ_INT(player_status_word(), 0);
}

/* 消えた印しか消さない —— 同時に効いている別の状態は続く。 */
TEST(running_out_leaves_the_other_marks_where_they_are) {
    given_no_state_at_all();
    player_timed_set(PLAYER_TIMED_BLINDNESS, 1);
    player_timed_set(PLAYER_TIMED_CONFUSION, 10);
    (void)player_timed_beginning(PLAYER_TIMED_BLINDNESS);
    (void)player_timed_beginning(PLAYER_TIMED_CONFUSION);
    (void)player_timed_count_down(PLAYER_TIMED_BLINDNESS);

    ASSERT_EQ_INT(player_status_word(), PY_CONFUSED);
}

TEST(one_turn_leaves_the_other_seventeen_clocks_alone) {
    given_no_state_at_all();
    player_timed_set(PLAYER_TIMED_BLINDNESS, 1);
    player_timed_set(PLAYER_TIMED_CONFUSION, 10);
    (void)player_timed_count_down(PLAYER_TIMED_BLINDNESS);

    ASSERT_EQ_INT(clocks_in_force(), ONLY(PLAYER_TIMED_CONFUSION));
}

/* 1 ターンしか残っていない状態で始まった場合、**始まりと終わりが同じターンに
 * 落ちる**。両方が起きることを 2 ビットに詰める（3 = 始まりも終わりも）。 */
static int began_and_ended(player_timed_effect effect, int turns) {
    int what_happened = 0;

    given_no_state_at_all();
    player_timed_add(effect, turns);

    if (player_timed_beginning(effect)) {
        what_happened |= 1;
    }
    if (player_timed_count_down(effect)) {
        what_happened |= 2;
    }

    return what_happened;
}

TEST(a_state_with_one_turn_begins_and_ends_in_the_same_turn) {
    ASSERT_EQ_INT(began_and_ended(PLAYER_TIMED_BLINDNESS, 1), 1 | 2);
}

TEST(a_state_with_two_turns_only_begins) {
    ASSERT_EQ_INT(began_and_ended(PLAYER_TIMED_BLINDNESS, 2), 1);
}

/* 18 個ぜんぶが数え切れる —— 添字がずれていれば隣を数えて false が返る。 */
static long clocks_that_ran_out(void) {
    long ran_out = 0;

    given_no_state_at_all();
    for (int effect = 0; effect < PLAYER_TIMED_COUNT; effect++) {
        player_timed_set((player_timed_effect)effect, 1);
    }
    for (int effect = 0; effect < PLAYER_TIMED_COUNT; effect++) {
        if (player_timed_count_down((player_timed_effect)effect)) {
            ran_out |= ONLY(effect);
        }
    }

    return ran_out;
}

TEST(all_eighteen_clocks_run_out_on_their_own_last_turn) {
    ASSERT_EQ_INT(clocks_that_ran_out(), ALL_EIGHTEEN);
}

/* --- 0 を通りすぎる（恐れの -1） ---------------------------------------- */

/* dungeon.c:301-320 の形そのまま: 英雄状態の人が新しく怖がると、その恐れは
 * 0 に置かれ、**同じターンの数えおとしがそれを -1 にする**。 */

TEST(counting_down_from_zero_reaches_minus_one) {
    given_no_state_at_all();
    player_timed_clear(PLAYER_TIMED_FEAR);
    (void)player_timed_count_down(PLAYER_TIMED_FEAR);

    ASSERT_EQ_INT(player_timed_turns(PLAYER_TIMED_FEAR), -1);
}

TEST(counting_down_from_zero_brings_no_ending) {
    given_no_state_at_all();
    player_timed_clear(PLAYER_TIMED_FEAR);

    ASSERT_FALSE(player_timed_count_down(PLAYER_TIMED_FEAR));
}

/* 数を 0 にしても印はそのまま —— 消すのは数えおとしの仕事で、
 * player_timed_clear() は数だけを触る（moria4.c が罠の上で混乱を預けて戻せるのも
 * これがあってこそ）。 */
TEST(clearing_the_clock_leaves_the_mark_set) {
    given_no_state_at_all();
    player_timed_add(PLAYER_TIMED_FEAR, 10);
    (void)player_timed_beginning(PLAYER_TIMED_FEAR);
    player_timed_clear(PLAYER_TIMED_FEAR);

    ASSERT_EQ_INT(player_status_word(), PY_FEAR);
}

/* -1 まで落ちても印は消えない（「== 0」を外れたので終わりを通らない）。
 * **これが「0 で止める」直しをしたときに落ちる釘。** */
TEST(passing_through_zero_never_puts_the_mark_away) {
    given_no_state_at_all();
    player_timed_add(PLAYER_TIMED_FEAR, 10);
    (void)player_timed_beginning(PLAYER_TIMED_FEAR);
    player_timed_clear(PLAYER_TIMED_FEAR);
    (void)player_timed_count_down(PLAYER_TIMED_FEAR);

    ASSERT_EQ_INT(player_status_word(), PY_FEAR);
}

/* -1 は残りつづける（dungeon.c の塊は「0 より大きいあいだ」しか走らないので
 * 誰も直さない）、そして次の恐れを 1 ターン短くする。 */
TEST(the_minus_one_makes_the_next_fright_one_turn_shorter) {
    given_no_state_at_all();
    player_timed_clear(PLAYER_TIMED_FEAR);
    (void)player_timed_count_down(PLAYER_TIMED_FEAR);
    player_timed_add(PLAYER_TIMED_FEAR, 10);

    ASSERT_EQ_INT(player_timed_turns(PLAYER_TIMED_FEAR), 9);
}

int main(void) {
    RUN_TEST(the_blindness_clock_is_its_own);
    RUN_TEST(the_paralysis_clock_is_its_own);
    RUN_TEST(the_confusion_clock_is_its_own);
    RUN_TEST(the_haste_clock_is_its_own);
    RUN_TEST(the_slowness_clock_is_its_own);
    RUN_TEST(the_fear_clock_is_its_own);
    RUN_TEST(the_poison_clock_is_its_own);
    RUN_TEST(the_hallucination_clock_is_its_own);
    RUN_TEST(the_protection_from_evil_clock_is_its_own);
    RUN_TEST(the_invulnerability_clock_is_its_own);
    RUN_TEST(the_heroism_clock_is_its_own);
    RUN_TEST(the_super_heroism_clock_is_its_own);
    RUN_TEST(the_blessing_clock_is_its_own);
    RUN_TEST(the_heat_resistance_clock_is_its_own);
    RUN_TEST(the_cold_resistance_clock_is_its_own);
    RUN_TEST(the_seeing_invisible_clock_is_its_own);
    RUN_TEST(the_word_of_recall_clock_is_its_own);
    RUN_TEST(the_infra_vision_clock_is_its_own);
    RUN_TEST(there_are_eighteen_clocks);

    RUN_TEST(each_clock_keeps_the_length_it_was_given);

    RUN_TEST(a_clock_at_zero_is_not_in_force);
    RUN_TEST(a_clock_below_zero_is_not_in_force);
    RUN_TEST(one_turn_left_is_in_force);
    RUN_TEST(turns_answers_the_number_that_was_put_in);
    RUN_TEST(turns_answers_a_negative_number_too);

    RUN_TEST(adding_turns_makes_the_state_last_longer);
    RUN_TEST(adding_to_a_clock_below_zero_starts_from_there);
    RUN_TEST(setting_turns_throws_away_what_was_left);
    RUN_TEST(clearing_leaves_no_turns);
    RUN_TEST(clearing_one_clock_leaves_the_others_running);
    RUN_TEST(shortening_cuts_a_long_state_down);
    RUN_TEST(shortening_never_lengthens_a_short_state);
    RUN_TEST(shortening_a_state_that_is_not_in_force_starts_nothing);
    RUN_TEST(shortening_to_its_own_length_changes_nothing);

    RUN_TEST(beginning_blindness_marks_blindness_and_nothing_else);
    RUN_TEST(beginning_confusion_marks_confusion_and_nothing_else);
    RUN_TEST(beginning_haste_marks_haste_and_nothing_else);
    RUN_TEST(beginning_slowness_marks_slowness_and_nothing_else);
    RUN_TEST(beginning_fear_marks_fear_and_nothing_else);
    RUN_TEST(beginning_poison_marks_poison_and_nothing_else);
    RUN_TEST(beginning_invulnerability_marks_invulnerability_and_nothing_else);
    RUN_TEST(beginning_heroism_marks_heroism_and_nothing_else);
    RUN_TEST(beginning_super_heroism_marks_super_heroism_and_nothing_else);
    RUN_TEST(beginning_blessing_marks_blessing_and_nothing_else);
    RUN_TEST(beginning_seeing_invisible_marks_seeing_invisible_and_nothing_else);
    RUN_TEST(beginning_infra_vision_marks_infra_vision_and_nothing_else);

    RUN_TEST(paralysis_has_no_beginning_to_announce);
    RUN_TEST(hallucination_has_no_beginning_to_announce);
    RUN_TEST(protection_from_evil_has_no_beginning_to_announce);
    RUN_TEST(heat_resistance_has_no_beginning_to_announce);
    RUN_TEST(cold_resistance_has_no_beginning_to_announce);
    RUN_TEST(word_of_recall_has_no_beginning_to_announce);
    RUN_TEST(a_clock_without_a_mark_still_runs_out);

    RUN_TEST(a_beginning_is_due_the_first_time);
    RUN_TEST(a_beginning_is_not_due_again_on_the_next_turn);
    RUN_TEST(adding_turns_to_a_state_already_begun_brings_no_new_beginning);
    RUN_TEST(a_beginning_does_not_move_the_clock);

    RUN_TEST(one_turn_takes_one_turn_off);
    RUN_TEST(no_ending_is_due_while_turns_remain);
    RUN_TEST(the_ending_is_due_on_the_turn_the_last_one_is_used);
    RUN_TEST(running_out_puts_the_mark_away);
    RUN_TEST(running_out_leaves_the_other_marks_where_they_are);
    RUN_TEST(one_turn_leaves_the_other_seventeen_clocks_alone);
    RUN_TEST(a_state_with_one_turn_begins_and_ends_in_the_same_turn);
    RUN_TEST(a_state_with_two_turns_only_begins);
    RUN_TEST(all_eighteen_clocks_run_out_on_their_own_last_turn);

    RUN_TEST(counting_down_from_zero_reaches_minus_one);
    RUN_TEST(counting_down_from_zero_brings_no_ending);
    RUN_TEST(clearing_the_clock_leaves_the_mark_set);
    RUN_TEST(passing_through_zero_never_puts_the_mark_away);
    RUN_TEST(the_minus_one_makes_the_next_fright_one_turn_shorter);

    return TEST_SUMMARY();
}
