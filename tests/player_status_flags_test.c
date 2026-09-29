// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「いまの状態の旗」のテスト -- 現在のふるまいを保護する
 *
 * py から出す 7 つめの問い（1 つめは持っている金、2 つめは腹の具合、3 つめは
 * 画面に出す数字、4 つめは魔力、5 つめは体力、6 つめは階級と経験値）。
 * フィールドは 1 つ —— もとは py.flags.status —— だが、そこに **31 の名前**が
 * 載っていて、10 ファイルから 105 か所が触っていた。
 *
 * **この単位で新しいのは「問いの答えが数ではなく集合」ということ。**
 * ここまでの 6 つは「いくら」「どこまで」だったが、これは「30 のうちどれが
 * 真か」。だからテストの主眼も変わる:
 *
 *   1. **ビットの番号はセーブファイルの書式**（save.c が 1 語まるごと
 *      wr_long / rd_long する）。だから **14 の印がそれぞれ constant.h の
 *      どのビットかを名指しで釘打つ** —— 表（src/player/player_status_flags.c の
 *      effect_marks[]）の 2 行を入れかえても、独立性だけ見るテストでは
 *      気づけない。入れかえは「セーブファイルの読みちがえ」そのもの。
 *   2. **どの窓口も自分のビット以外を触らない。** 1 語を共有しているので、
 *      マスクの取りちがえ（`&` と `&~`、`|=` と `=`）は他の族の旗を巻きこむ。
 *      **全ビットを立ててから窓口を 1 つ呼び、残りが動いていないことを見る**
 *      形をくりかえす。
 *
 * 30 のビットは 4 つの族に分かれる（src/player/player_status_flags.h の頭に長く
 * 書いた）。保護したい性質を族ごとに並べると:
 *
 *   族 1・印（14）: 立っているかを訊ける。**立てる窓口は「新しかったか」を
 *      返す** —— 呼び手の形が `if ((PY_X & status) == 0) { ... }` で、
 *      初回だけの言葉（「英雄になった！」）がそこに吊られているから。
 *      **腹の 2 つだけは一息で消える**（eat.c の満腹）。
 *
 *   族 2・要求（13）: 立てて歩きさる。**一様ではない** —— 速さ・鎧・体力・
 *      魔力・能力値は「読んで消す」1 手だが、腕力の点検と「Study」の欄は
 *      dungeon.c が読んで、仕事をした関数（check_strength・prt_study）が
 *      消す。だから **読んでも消えないこと**を釘で留める。
 *      能力値の 6 つは **PY_STR から上へ 1 ビットずつ隣接**していて
 *      （constant.h の「these 6 stat flags must be adjacent」）、
 *      **PY_STATS がちょうどその 6 つを覆う**。この 2 つの前提はもと 3 か所が
 *      頼っていて、いまはこのモジュールだけが頼っている。
 *
 *   族 3・いま何をしているか（2）: 探索と休息。時計も再計算もない。
 *
 *   族 4・状態行が何を出しているか（2）: 麻痺と Repeat。状態でも要求でもなく
 *      **画面の控え**なので、真にも偽にも置ける（set か clear か）。
 *
 * テストは 1 プロセスで状態を共有するので、走りだしを見る 1 件は main() の
 * 先頭に置き、以降は各件が最初に 1 語を置きなおす。
 */
/* externs.h は要らない。窓口と、PY_* と uint32_t のための constant.h・
 * types.h だけで足りる。**PY_* をここで名指しするのは意図的** ——
 * 呼び手からは消したが、ビット番号がセーブファイルの書式である以上、
 * それを守るテストはどこかで名前を言わなければならない。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_status_flags.h"

#include "minunit.h"

/* 1 語ぜんぶ。名前の無いビット（0x00000800 と 0x00800000）も含めて立てる ——
 * 「自分のビット以外を触らない」はいちばん強い形で見たい。 */
#define EVERY_BIT 0xFFFFFFFFu

/* 旗が 1 つも立っていない状態（人物の走りだし）。 */
static void given_nothing_set(void) {
    player_set_status_word(0);
}

static void given_every_bit_set(void) {
    player_set_status_word(EVERY_BIT);
}

/* 期待値の書き方。「ぜんぶ立っていて、この 1 つだけ消えている」。 */
static long every_bit_but(uint32_t mask) {
    return (long)(uint32_t)(EVERY_BIT & ~mask);
}

/* 何も立っていない所から印を 1 つ立てて、1 語を見る。
 * **これが「印 ↔ ビット」の釘打ちの道具。** */
static long mark_of(player_effect effect) {
    given_nothing_set();
    player_note_effect_started(effect);

    return (long)player_status_word();
}

/* --- 走りだし ------------------------------------------------------------- */
/* この 1 件は main() の先頭で走らせる。ほかのテストが 1 語を動かすので。 */

TEST(no_flag_stands_at_the_start) {
    ASSERT_EQ_INT(0, player_status_word());
}

/* --- 族 1: 印がどのビットか（セーブファイルの書式） ----------------------- */
/* 14 件を名指しで並べる。effect_marks[] の 2 行の入れかえは、ここだけで
 * 捕まる —— 独立性の件はどちらも通してしまう。 */

TEST(the_hero_mark_is_the_hero_bit) {
    ASSERT_EQ_INT(PY_HERO, mark_of(PLAYER_EFFECT_HERO));
}

TEST(the_super_hero_mark_is_the_super_hero_bit) {
    ASSERT_EQ_INT(PY_SHERO, mark_of(PLAYER_EFFECT_SUPER_HERO));
}

TEST(the_blind_mark_is_the_blind_bit) {
    ASSERT_EQ_INT(PY_BLIND, mark_of(PLAYER_EFFECT_BLIND));
}

TEST(the_confused_mark_is_the_confused_bit) {
    ASSERT_EQ_INT(PY_CONFUSED, mark_of(PLAYER_EFFECT_CONFUSED));
}

/* 名前が 2 通りある 1 つ。旗は PY_FEAR、窓口は afraid。 */
TEST(the_afraid_mark_is_the_fear_bit) {
    ASSERT_EQ_INT(PY_FEAR, mark_of(PLAYER_EFFECT_AFRAID));
}

TEST(the_poisoned_mark_is_the_poisoned_bit) {
    ASSERT_EQ_INT(PY_POISONED, mark_of(PLAYER_EFFECT_POISONED));
}

TEST(the_hasted_mark_is_the_fast_bit) {
    ASSERT_EQ_INT(PY_FAST, mark_of(PLAYER_EFFECT_HASTED));
}

TEST(the_slowed_mark_is_the_slow_bit) {
    ASSERT_EQ_INT(PY_SLOW, mark_of(PLAYER_EFFECT_SLOWED));
}

TEST(the_invulnerable_mark_is_the_invulnerability_bit) {
    ASSERT_EQ_INT(PY_INVULN, mark_of(PLAYER_EFFECT_INVULNERABLE));
}

TEST(the_blessed_mark_is_the_blessed_bit) {
    ASSERT_EQ_INT(PY_BLESSED, mark_of(PLAYER_EFFECT_BLESSED));
}

TEST(the_see_invisible_mark_is_the_detect_invisible_bit) {
    ASSERT_EQ_INT(PY_DET_INV, mark_of(PLAYER_EFFECT_SEE_INVISIBLE));
}

TEST(the_infra_vision_mark_is_the_timed_infra_vision_bit) {
    ASSERT_EQ_INT(PY_TIM_INFRA, mark_of(PLAYER_EFFECT_INFRA_VISION));
}

/* 腹の 2 つ。counter が py.flags ではなく胃の中身（player_food.c）なのが
 * ほかの 12 と違うところだが、旗としては同じ族。 */
TEST(the_hungry_mark_is_the_hungry_bit) {
    ASSERT_EQ_INT(PY_HUNGRY, mark_of(PLAYER_EFFECT_HUNGRY));
}

TEST(the_weak_mark_is_the_weak_bit) {
    ASSERT_EQ_INT(PY_WEAK, mark_of(PLAYER_EFFECT_WEAK));
}

/* --- 族 1: 印のふるまい --------------------------------------------------- */

TEST(a_mark_that_was_set_is_in_force) {
    given_nothing_set();
    player_note_effect_started(PLAYER_EFFECT_BLIND);

    ASSERT_TRUE(player_effect_in_force(PLAYER_EFFECT_BLIND));
}

TEST(a_mark_that_was_not_set_is_not_in_force) {
    given_nothing_set();
    player_note_effect_started(PLAYER_EFFECT_BLIND);

    ASSERT_FALSE(player_effect_in_force(PLAYER_EFFECT_CONFUSED));
}

/* 呼び手の初回だけの言葉が吊られている 1 本。 */
TEST(the_first_note_of_a_start_says_the_mark_was_new) {
    given_nothing_set();

    ASSERT_TRUE(player_note_effect_started(PLAYER_EFFECT_POISONED));
}

TEST(a_second_note_of_the_same_start_says_it_was_not_new) {
    given_nothing_set();
    player_note_effect_started(PLAYER_EFFECT_POISONED);

    ASSERT_FALSE(player_note_effect_started(PLAYER_EFFECT_POISONED));
}

/* 立てるほうも消すほうも、1 語の残りを動かさない。 */
TEST(noting_a_start_leaves_every_other_bit_alone) {
    player_set_status_word(EVERY_BIT & ~PY_HERO);
    player_note_effect_started(PLAYER_EFFECT_HERO);

    ASSERT_EQ_INT((long)EVERY_BIT, player_status_word());
}

TEST(noting_an_end_clears_only_that_mark) {
    given_every_bit_set();
    player_note_effect_ended(PLAYER_EFFECT_HERO);

    ASSERT_EQ_INT(every_bit_but(PY_HERO), player_status_word());
}

TEST(a_mark_that_ended_is_no_longer_in_force) {
    given_every_bit_set();
    player_note_effect_ended(PLAYER_EFFECT_SLOWED);

    ASSERT_FALSE(player_effect_in_force(PLAYER_EFFECT_SLOWED));
}

/* 消えている印を消しても何も起きない（呼び手は立っているか確かめずに
 * 呼ぶことがある）。 */
TEST(ending_a_mark_that_was_not_set_changes_nothing) {
    given_nothing_set();
    player_note_effect_ended(PLAYER_EFFECT_SLOWED);

    ASSERT_EQ_INT(0, player_status_word());
}

/* 満腹は 2 つを一息で消す（eat.c）。片方だけ消す変異はここで止まる。 */
TEST(a_full_stomach_clears_both_hunger_marks) {
    player_set_status_word(PY_HUNGRY | PY_WEAK);
    player_note_hunger_satisfied();

    ASSERT_EQ_INT(0, player_status_word());
}

TEST(a_full_stomach_leaves_every_other_bit_alone) {
    given_every_bit_set();
    player_note_hunger_satisfied();

    ASSERT_EQ_INT(every_bit_but(PY_HUNGRY | PY_WEAK), player_status_word());
}

/* --- 族 2: 読んでも消えない 2 つ（腕力の点検と Study） -------------------- */
/* dungeon.c が読み、仕事をした関数が消す。**読みが消してしまう変異**は
 * 本体では「点検が 1 度で終わらない」「Study の欄が消えない」になる。 */

TEST(a_strength_check_request_is_its_own_bit) {
    given_nothing_set();
    player_request_strength_check();

    ASSERT_EQ_INT(PY_STR_WGT, player_status_word());
}

TEST(a_strength_check_request_is_remembered) {
    given_nothing_set();
    player_request_strength_check();

    ASSERT_TRUE(player_strength_check_requested());
}

/* 読んでも消えない —— 消すのは check_strength() の仕事。 */
TEST(reading_a_strength_check_request_does_not_take_it) {
    given_nothing_set();
    player_request_strength_check();
    player_strength_check_requested();

    ASSERT_TRUE(player_strength_check_requested());
}

TEST(clearing_a_strength_check_request_takes_only_that_bit) {
    given_every_bit_set();
    player_clear_strength_check_request();

    ASSERT_EQ_INT(every_bit_but(PY_STR_WGT), player_status_word());
}

TEST(no_strength_check_is_requested_when_none_was_asked_for) {
    player_set_status_word(EVERY_BIT & ~PY_STR_WGT);

    ASSERT_FALSE(player_strength_check_requested());
}

TEST(a_study_redraw_request_is_its_own_bit) {
    given_nothing_set();
    player_request_study_redraw();

    ASSERT_EQ_INT(PY_STUDY, player_status_word());
}

TEST(a_study_redraw_request_is_remembered) {
    given_nothing_set();
    player_request_study_redraw();

    ASSERT_TRUE(player_study_redraw_requested());
}

/* 読んでも消えない —— 消すのは prt_study() の仕事。 */
TEST(reading_a_study_redraw_request_does_not_take_it) {
    given_nothing_set();
    player_request_study_redraw();
    player_study_redraw_requested();

    ASSERT_TRUE(player_study_redraw_requested());
}

TEST(clearing_a_study_redraw_request_takes_only_that_bit) {
    given_every_bit_set();
    player_clear_study_redraw_request();

    ASSERT_EQ_INT(every_bit_but(PY_STUDY), player_status_word());
}

TEST(no_study_redraw_is_requested_when_none_was_asked_for) {
    player_set_status_word(EVERY_BIT & ~PY_STUDY);

    ASSERT_FALSE(player_study_redraw_requested());
}

/* --- 族 2: 読んで消す 4 つ（速さ・鎧・体力・魔力） ------------------------- */
/* どれも dungeon.c の 1 つの塊が受けとる。**「待っていたか」を返して何も
 * 残さない**のが約束で、残す変異は本体では「毎ターン描きなおす」になる。 */

TEST(a_speed_redraw_request_is_its_own_bit) {
    given_nothing_set();
    player_request_speed_redraw();

    ASSERT_EQ_INT(PY_SPEED, player_status_word());
}

TEST(taking_a_speed_redraw_request_says_one_was_waiting) {
    given_nothing_set();
    player_request_speed_redraw();

    ASSERT_TRUE(player_take_speed_redraw_request());
}

TEST(taking_a_speed_redraw_request_leaves_nothing_behind) {
    given_nothing_set();
    player_request_speed_redraw();
    player_take_speed_redraw_request();

    ASSERT_FALSE(player_take_speed_redraw_request());
}

TEST(taking_a_speed_redraw_request_that_was_not_made_says_so) {
    player_set_status_word(EVERY_BIT & ~PY_SPEED);

    ASSERT_FALSE(player_take_speed_redraw_request());
}

TEST(taking_a_speed_redraw_request_takes_only_that_bit) {
    given_every_bit_set();
    player_take_speed_redraw_request();

    ASSERT_EQ_INT(every_bit_but(PY_SPEED), player_status_word());
}

TEST(an_armor_redraw_request_is_its_own_bit) {
    given_nothing_set();
    player_request_armor_redraw();

    ASSERT_EQ_INT(PY_ARMOR, player_status_word());
}

TEST(taking_an_armor_redraw_request_says_one_was_waiting) {
    given_nothing_set();
    player_request_armor_redraw();

    ASSERT_TRUE(player_take_armor_redraw_request());
}

TEST(taking_an_armor_redraw_request_takes_only_that_bit) {
    given_every_bit_set();
    player_take_armor_redraw_request();

    ASSERT_EQ_INT(every_bit_but(PY_ARMOR), player_status_word());
}

TEST(a_hp_redraw_request_is_its_own_bit) {
    given_nothing_set();
    player_request_hp_redraw();

    ASSERT_EQ_INT(PY_HP, player_status_word());
}

TEST(taking_a_hp_redraw_request_says_one_was_waiting) {
    given_nothing_set();
    player_request_hp_redraw();

    ASSERT_TRUE(player_take_hp_redraw_request());
}

TEST(taking_a_hp_redraw_request_takes_only_that_bit) {
    given_every_bit_set();
    player_take_hp_redraw_request();

    ASSERT_EQ_INT(every_bit_but(PY_HP), player_status_word());
}

/* 魔力の旗はいちばん上のビット。**1 語は符号なしでなければならない** ——
 * 符号つきの器に入れると `1L << 31` が処理系まかせになる。 */
TEST(a_mana_redraw_request_is_the_top_bit) {
    given_nothing_set();
    player_request_mana_redraw();

    ASSERT_EQ_INT(PY_MANA, player_status_word());
}

TEST(taking_a_mana_redraw_request_says_one_was_waiting) {
    given_nothing_set();
    player_request_mana_redraw();

    ASSERT_TRUE(player_take_mana_redraw_request());
}

TEST(taking_a_mana_redraw_request_takes_only_that_bit) {
    given_every_bit_set();
    player_take_mana_redraw_request();

    ASSERT_EQ_INT(every_bit_but(PY_MANA), player_status_word());
}

/* --- 族 2: 能力値の 6 つ（隣接という前提） --------------------------------- */
/* 呼び手は添字（0..5、py.stats の順）で言い、ずらしはこのモジュールの中。
 * **6 件を名指しで並べるのは、ずらしの向きと起点を釘で留めるため** ——
 * `PY_STR << (stat + 1)` や `PY_CHR >> stat` はここでしか捕まらない。 */

static long stat_mark_of(int stat) {
    given_nothing_set();
    player_request_stat_redraw(stat);

    return (long)player_status_word();
}

TEST(the_first_stat_is_the_strength_bit) {
    ASSERT_EQ_INT(PY_STR, stat_mark_of(0));
}

TEST(the_second_stat_is_the_intelligence_bit) {
    ASSERT_EQ_INT(PY_INT, stat_mark_of(1));
}

TEST(the_third_stat_is_the_wisdom_bit) {
    ASSERT_EQ_INT(PY_WIS, stat_mark_of(2));
}

TEST(the_fourth_stat_is_the_dexterity_bit) {
    ASSERT_EQ_INT(PY_DEX, stat_mark_of(3));
}

TEST(the_fifth_stat_is_the_constitution_bit) {
    ASSERT_EQ_INT(PY_CON, stat_mark_of(4));
}

TEST(the_sixth_stat_is_the_charisma_bit) {
    ASSERT_EQ_INT(PY_CHR, stat_mark_of(5));
}

/* constant.h の「these 6 stat flags must be adjacent」の後半 ——
 * **PY_STATS がちょうどその 6 つを覆う**。 */
TEST(the_six_stat_requests_are_what_the_stats_mask_covers) {
    given_nothing_set();
    for (int stat = 0; stat < 6; stat++) {
        player_request_stat_redraw(stat);
    }

    ASSERT_EQ_INT(PY_STATS, player_status_word());
}

TEST(a_stat_request_is_remembered) {
    given_nothing_set();
    player_request_stat_redraw(2);

    ASSERT_TRUE(player_stat_redraw_requested(2));
}

TEST(a_stat_request_does_not_answer_for_another_stat) {
    given_nothing_set();
    player_request_stat_redraw(2);

    ASSERT_FALSE(player_stat_redraw_requested(3));
}

/* 読んでも消えない（消すのは 6 つまとめて 1 回）。 */
TEST(reading_a_stat_request_does_not_take_it) {
    given_nothing_set();
    player_request_stat_redraw(4);
    player_stat_redraw_requested(4);

    ASSERT_TRUE(player_stat_redraw_requested(4));
}

/* dungeon.c は 6 つのうち 1 つでも立っていれば計算しなおす。 */
TEST(any_stat_request_is_seen) {
    given_nothing_set();
    player_request_stat_redraw(5);

    ASSERT_TRUE(player_any_stat_redraw_requested());
}

/* 6 つ以外の旗を PY_STATS が拾っていないこと。 */
TEST(no_stat_request_is_seen_when_only_other_flags_stand) {
    player_set_status_word(EVERY_BIT & ~(uint32_t)PY_STATS);

    ASSERT_FALSE(player_any_stat_redraw_requested());
}

TEST(clearing_the_stat_requests_takes_all_six) {
    given_every_bit_set();
    player_clear_stat_redraw_requests();

    ASSERT_EQ_INT(every_bit_but(PY_STATS), player_status_word());
}

TEST(no_stat_is_requested_after_clearing) {
    given_every_bit_set();
    player_clear_stat_redraw_requests();

    ASSERT_FALSE(player_stat_redraw_requested(3));
}

/* --- 族 3: いま何をしているか（探索・休息） -------------------------------- */
/* 30 のうちいちばん読まれる旗（探索は 9 か所）。時計も再計算もない。 */

TEST(a_character_who_started_searching_is_searching) {
    given_nothing_set();
    player_start_searching();

    ASSERT_TRUE(player_is_searching());
}

TEST(starting_to_search_sets_only_the_search_bit) {
    given_nothing_set();
    player_start_searching();

    ASSERT_EQ_INT(PY_SEARCH, player_status_word());
}

TEST(a_character_who_stopped_searching_is_not_searching) {
    given_every_bit_set();
    player_stop_searching();

    ASSERT_FALSE(player_is_searching());
}

TEST(stopping_searching_clears_only_the_search_bit) {
    given_every_bit_set();
    player_stop_searching();

    ASSERT_EQ_INT(every_bit_but(PY_SEARCH), player_status_word());
}

TEST(a_character_who_started_resting_is_resting) {
    given_nothing_set();
    player_start_resting();

    ASSERT_TRUE(player_is_resting());
}

TEST(starting_to_rest_sets_only_the_rest_bit) {
    given_nothing_set();
    player_start_resting();

    ASSERT_EQ_INT(PY_REST, player_status_word());
}

TEST(a_character_who_stopped_resting_is_not_resting) {
    given_every_bit_set();
    player_stop_resting();

    ASSERT_FALSE(player_is_resting());
}

TEST(stopping_resting_clears_only_the_rest_bit) {
    given_every_bit_set();
    player_stop_resting();

    ASSERT_EQ_INT(every_bit_but(PY_REST), player_status_word());
}

/* 隣りあった 2 ビットなので取りちがえやすい。 */
TEST(searching_is_not_resting) {
    given_nothing_set();
    player_start_searching();

    ASSERT_FALSE(player_is_resting());
}

/* --- 族 4: 状態行が何を出しているか（麻痺・Repeat） ------------------------ */
/* 状態でも要求でもなく**画面の控え**。だから真にも偽にも置ける。 */

TEST(the_status_line_can_be_recorded_as_showing_paralysis) {
    given_nothing_set();
    player_set_status_line_shows_paralysis(true);

    ASSERT_TRUE(player_status_line_shows_paralysis());
}

TEST(recording_paralysis_sets_only_the_paralysis_bit) {
    given_nothing_set();
    player_set_status_line_shows_paralysis(true);

    ASSERT_EQ_INT(PY_PARALYSED, player_status_word());
}

/* 偽を渡したら消える —— 引数を見ない変異（いつも立てる）はここで止まる。 */
TEST(recording_no_paralysis_clears_only_the_paralysis_bit) {
    given_every_bit_set();
    player_set_status_line_shows_paralysis(false);

    ASSERT_EQ_INT(every_bit_but(PY_PARALYSED), player_status_word());
}

TEST(the_status_line_is_not_showing_paralysis_once_it_is_cleared) {
    given_every_bit_set();
    player_set_status_line_shows_paralysis(false);

    ASSERT_FALSE(player_status_line_shows_paralysis());
}

TEST(the_status_line_can_be_recorded_as_showing_repeat) {
    given_nothing_set();
    player_set_status_line_shows_repeat(true);

    ASSERT_TRUE(player_status_line_shows_repeat());
}

TEST(recording_repeat_sets_only_the_repeat_bit) {
    given_nothing_set();
    player_set_status_line_shows_repeat(true);

    ASSERT_EQ_INT(PY_REPEAT, player_status_word());
}

TEST(recording_no_repeat_clears_only_the_repeat_bit) {
    given_every_bit_set();
    player_set_status_line_shows_repeat(false);

    ASSERT_EQ_INT(every_bit_but(PY_REPEAT), player_status_word());
}

/* prt_state() は毎回まず消して、描いたら立てなおす。 */
TEST(the_status_line_record_can_be_cleared_and_set_again) {
    given_nothing_set();
    player_set_status_line_shows_repeat(false);
    player_set_status_line_shows_repeat(true);

    ASSERT_TRUE(player_status_line_shows_repeat());
}

/* --- 1 語まるごと（セーブファイルの書式） --------------------------------- */

/* save.c は 1 語を wr_long / rd_long で往復させる。**いちばん上のビットまで
 * 落とさずに運べること**が要件。 */
TEST(the_whole_word_makes_the_round_trip) {
    player_set_status_word(0xDEADBEEFu);

    ASSERT_EQ_INT((long)0xDEADBEEFu, player_status_word());
}

/* 置きなおしは**上書き**で、足しこみではない（読みこみが前の人物の旗を
 * 引きずらないこと）。 */
TEST(setting_the_word_replaces_what_was_there) {
    given_every_bit_set();
    player_set_status_word(PY_SEARCH);

    ASSERT_EQ_INT(PY_SEARCH, player_status_word());
}

/* 窓口で立てた旗が 1 語に見えること —— 窓口と 1 語が同じ置き場を指している
 * ことの確認（別の器に分かれていたら気づけるように）。 */
TEST(the_word_shows_what_the_windows_set) {
    given_nothing_set();
    player_start_searching();
    player_request_hp_redraw();

    ASSERT_EQ_INT(PY_SEARCH | PY_HP, player_status_word());
}

int main(void) {
    RUN_TEST(no_flag_stands_at_the_start);

    RUN_TEST(the_hero_mark_is_the_hero_bit);
    RUN_TEST(the_super_hero_mark_is_the_super_hero_bit);
    RUN_TEST(the_blind_mark_is_the_blind_bit);
    RUN_TEST(the_confused_mark_is_the_confused_bit);
    RUN_TEST(the_afraid_mark_is_the_fear_bit);
    RUN_TEST(the_poisoned_mark_is_the_poisoned_bit);
    RUN_TEST(the_hasted_mark_is_the_fast_bit);
    RUN_TEST(the_slowed_mark_is_the_slow_bit);
    RUN_TEST(the_invulnerable_mark_is_the_invulnerability_bit);
    RUN_TEST(the_blessed_mark_is_the_blessed_bit);
    RUN_TEST(the_see_invisible_mark_is_the_detect_invisible_bit);
    RUN_TEST(the_infra_vision_mark_is_the_timed_infra_vision_bit);
    RUN_TEST(the_hungry_mark_is_the_hungry_bit);
    RUN_TEST(the_weak_mark_is_the_weak_bit);

    RUN_TEST(a_mark_that_was_set_is_in_force);
    RUN_TEST(a_mark_that_was_not_set_is_not_in_force);
    RUN_TEST(the_first_note_of_a_start_says_the_mark_was_new);
    RUN_TEST(a_second_note_of_the_same_start_says_it_was_not_new);
    RUN_TEST(noting_a_start_leaves_every_other_bit_alone);
    RUN_TEST(noting_an_end_clears_only_that_mark);
    RUN_TEST(a_mark_that_ended_is_no_longer_in_force);
    RUN_TEST(ending_a_mark_that_was_not_set_changes_nothing);
    RUN_TEST(a_full_stomach_clears_both_hunger_marks);
    RUN_TEST(a_full_stomach_leaves_every_other_bit_alone);

    RUN_TEST(a_strength_check_request_is_its_own_bit);
    RUN_TEST(a_strength_check_request_is_remembered);
    RUN_TEST(reading_a_strength_check_request_does_not_take_it);
    RUN_TEST(clearing_a_strength_check_request_takes_only_that_bit);
    RUN_TEST(no_strength_check_is_requested_when_none_was_asked_for);

    RUN_TEST(a_study_redraw_request_is_its_own_bit);
    RUN_TEST(a_study_redraw_request_is_remembered);
    RUN_TEST(reading_a_study_redraw_request_does_not_take_it);
    RUN_TEST(clearing_a_study_redraw_request_takes_only_that_bit);
    RUN_TEST(no_study_redraw_is_requested_when_none_was_asked_for);

    RUN_TEST(a_speed_redraw_request_is_its_own_bit);
    RUN_TEST(taking_a_speed_redraw_request_says_one_was_waiting);
    RUN_TEST(taking_a_speed_redraw_request_leaves_nothing_behind);
    RUN_TEST(taking_a_speed_redraw_request_that_was_not_made_says_so);
    RUN_TEST(taking_a_speed_redraw_request_takes_only_that_bit);

    RUN_TEST(an_armor_redraw_request_is_its_own_bit);
    RUN_TEST(taking_an_armor_redraw_request_says_one_was_waiting);
    RUN_TEST(taking_an_armor_redraw_request_takes_only_that_bit);

    RUN_TEST(a_hp_redraw_request_is_its_own_bit);
    RUN_TEST(taking_a_hp_redraw_request_says_one_was_waiting);
    RUN_TEST(taking_a_hp_redraw_request_takes_only_that_bit);

    RUN_TEST(a_mana_redraw_request_is_the_top_bit);
    RUN_TEST(taking_a_mana_redraw_request_says_one_was_waiting);
    RUN_TEST(taking_a_mana_redraw_request_takes_only_that_bit);

    RUN_TEST(the_first_stat_is_the_strength_bit);
    RUN_TEST(the_second_stat_is_the_intelligence_bit);
    RUN_TEST(the_third_stat_is_the_wisdom_bit);
    RUN_TEST(the_fourth_stat_is_the_dexterity_bit);
    RUN_TEST(the_fifth_stat_is_the_constitution_bit);
    RUN_TEST(the_sixth_stat_is_the_charisma_bit);
    RUN_TEST(the_six_stat_requests_are_what_the_stats_mask_covers);
    RUN_TEST(a_stat_request_is_remembered);
    RUN_TEST(a_stat_request_does_not_answer_for_another_stat);
    RUN_TEST(reading_a_stat_request_does_not_take_it);
    RUN_TEST(any_stat_request_is_seen);
    RUN_TEST(no_stat_request_is_seen_when_only_other_flags_stand);
    RUN_TEST(clearing_the_stat_requests_takes_all_six);
    RUN_TEST(no_stat_is_requested_after_clearing);

    RUN_TEST(a_character_who_started_searching_is_searching);
    RUN_TEST(starting_to_search_sets_only_the_search_bit);
    RUN_TEST(a_character_who_stopped_searching_is_not_searching);
    RUN_TEST(stopping_searching_clears_only_the_search_bit);
    RUN_TEST(a_character_who_started_resting_is_resting);
    RUN_TEST(starting_to_rest_sets_only_the_rest_bit);
    RUN_TEST(a_character_who_stopped_resting_is_not_resting);
    RUN_TEST(stopping_resting_clears_only_the_rest_bit);
    RUN_TEST(searching_is_not_resting);

    RUN_TEST(the_status_line_can_be_recorded_as_showing_paralysis);
    RUN_TEST(recording_paralysis_sets_only_the_paralysis_bit);
    RUN_TEST(recording_no_paralysis_clears_only_the_paralysis_bit);
    RUN_TEST(the_status_line_is_not_showing_paralysis_once_it_is_cleared);
    RUN_TEST(the_status_line_can_be_recorded_as_showing_repeat);
    RUN_TEST(recording_repeat_sets_only_the_repeat_bit);
    RUN_TEST(recording_no_repeat_clears_only_the_repeat_bit);
    RUN_TEST(the_status_line_record_can_be_cleared_and_set_again);

    RUN_TEST(the_whole_word_makes_the_round_trip);
    RUN_TEST(setting_the_word_replaces_what_was_there);
    RUN_TEST(the_word_shows_what_the_windows_set);

    return TEST_SUMMARY();
}
