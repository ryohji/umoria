// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「どれくらい探すか」のテスト -- 現在のふるまいを保護する
 *
 * py から出す 24 つめの問いで、**`struct misc` から出る 10 つめ**
 * （どこまで潜ったか・体力の骰子・守りの点数・素の命中力・罠と鍵をはずす腕・
 * 抵抗・どの種族か・体の重さ・命中と打撃の下駄につづく）。答えは
 * **2 つの short**。py.misc.srh と py.misc.fos を 8 ファイルから
 * 19 か所（18 行）が名ざしていたが、#18-12-25B で 16 呼びの窓口ごしになり、
 * #18-12-25C で **src/player/player_search_skill.c の static 2 つ**になった
 * —— `struct misc` は 10 → **8 フィールド**。人物の器（足場）も消えた
 * （#18-12-1C から 23 回め）。
 *
 * **2 つのフィールドで答えも 2 つ**（18 つめの `pac` ＋ `ptoac` は 26 か所
 * ぜんぶが和だったので答えは 1 つ）。**和を読む者が 1 人もいない** ——
 * `fos` は「今回見るか」を、`srh` は「見たら見つかるか」を決める。
 * それでも **module は 1 つ** —— 階級の表と探索つきの装備がいつも両方を
 * 動かすから（→ 台帳の所見 40）。
 *
 * **窓口は 5 本で、23 つめの裏返し**（あちらは置くが 1 本・足すが 2 本）——
 * 読み 2・**置く 2**・**対で足す 1**。
 * **置くのが 2 本なのは `wizard.c:173` が `srh` だけを置くから**
 * （デバッグの入り口は Gold → Searching → Stealth で `fos` を訊かない）。
 * **足すのが 1 本なのは、足す 2 か所がどちらも必ず両方を動かすから。**
 *
 * **`fos` は小さいほど良い**（1/n の n）。負にもなる —— Halfling は −5 から
 * 始まり、探索の指輪でさらに下がる。
 *
 * 外に残すものは 4 つ（player_search_skill.h に書いてある）——
 * 数のもと（種族と階級の表・装備）・人物画面の逆さ（`40 - fos` と 0 の留め）・
 * 今回見るかの判断（`randint()` と探索モードの旗）・見て何が見つかるか
 * （`search()`）。
 *
 * テストは 1 プロセスで状態を共有するので、各件が最初に 2 つを置きなおす。
 */
/* externs.h は要らない。窓口 5 本と、型のための 3 つだけ。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_search_skill.h"

#include "minunit.h"

/* 創成で 2 つが決まったところから始める（create.c:109・:112 と同じ形。
 * 引数は「見つかる腕・見る頻度」の順で、セーブの並びも、フィールドの並びも
 * この順）。 */
static void given_search_skill_of(int chance, int frequency) {
    player_search_chance_set(chance);
    player_search_frequency_set(frequency);
}

/* ------------------------------------------------------------------
 * 2 つの答え -- 別々に読め、互いに移らない
 * ------------------------------------------------------------------ */

TEST(the_chance_is_whatever_creation_worked_out) {
    given_search_skill_of(12, 33); /* Halfling の 12 に階級ぶんが乗る前 */

    ASSERT_EQ_INT(12, player_search_chance());
}

TEST(the_frequency_is_a_different_number_from_the_chance) {
    given_search_skill_of(12, 33);

    ASSERT_EQ_INT(33, player_search_frequency());
}

TEST(reading_either_number_twice_gives_the_same_answer) {
    given_search_skill_of(32, 20);

    ASSERT_EQ_INT(32, player_search_chance());
    ASSERT_EQ_INT(32, player_search_chance());
    ASSERT_EQ_INT(20, player_search_frequency());
    ASSERT_EQ_INT(20, player_search_frequency());
}

/* **`fos` は小さいほど良い**（`randint(fos) == 1` の n）。Halfling の
 * 種族ぶんは −5（src/data/player.c:126 の表）で、探索の指輪はさらに下げる。
 * player_move.c の "fos may be negative if have good rings of searching"
 * がそれを言っている。ここを留めると遊びが変わる（→ 所見 24）。 */
TEST(the_frequency_can_be_negative_because_smaller_is_better) {
    given_search_skill_of(12, -5);

    ASSERT_EQ_INT(-5, player_search_frequency());
}

/* 0 は「まだ決まっていない」でもあるが、`srh` の 0 は「まったく見つけられない」
 * という本当の答えでもある（`search()` に 0 が渡る）。 */
TEST(zero_is_both_the_starting_state_and_an_ordinary_chance) {
    given_search_skill_of(0, 0);

    ASSERT_EQ_INT(0, player_search_chance());
    ASSERT_EQ_INT(0, player_search_frequency());
}

/* ------------------------------------------------------------------
 * 置く 2 本 -- 片方だけ置く書き手が 1 人いる
 * ------------------------------------------------------------------ */

/* **これが窓口を 2 本に分けている理由。** wizard.c:173 は Searching だけを
 * 置きかえる（`m_ptr->srh = tmp_val;`）—— デバッグの入り口は
 * Gold → Searching → Stealth と並んでいて、**`fos` を訊く行が無い**。
 * 23 つめ（命中と打撃の下駄）は片方だけ置く呼び手が 1 つも無かったので
 * 対で置く 1 本になった。ここはその裏返し（→ 所見 40 の手順 5a）。 */
TEST(the_debugging_editor_sets_the_chance_without_touching_the_frequency) {
    given_search_skill_of(12, 33);

    player_search_chance_set(200); /* wizard.c の上限いっぱい */

    ASSERT_EQ_INT(200, player_search_chance());
    ASSERT_EQ_INT(33, player_search_frequency()); /* 動かない */
}

TEST(setting_the_frequency_leaves_the_chance_alone_as_well) {
    given_search_skill_of(12, 33);

    player_search_frequency_set(40);

    ASSERT_EQ_INT(12, player_search_chance());
    ASSERT_EQ_INT(40, player_search_frequency());
}

/* 置きなおすと**あとに置いたほうが残る**。足すのではない。 */
TEST(setting_again_replaces_rather_than_adds) {
    given_search_skill_of(12, 33);
    given_search_skill_of(32, 20);

    ASSERT_EQ_INT(32, player_search_chance());
    ASSERT_EQ_INT(20, player_search_frequency());
}

/* 創成の 2 行は**隣りあっていない**（create.c:109 と :112 のあいだに
 * player_base_to_hit_set() が入っている）。**それでも順は
 * 「見つかる腕・見る頻度」で、セーブファイルの 2 つの short と同じ**
 * （save.c:167 が srh、:168 が fos）。入れちがえたらここで落ちる。 */
TEST(the_chance_comes_first_the_way_the_saved_file_holds_it) {
    given_search_skill_of(1, 7);

    ASSERT_EQ_INT(1, player_search_chance());
    ASSERT_EQ_INT(7, player_search_frequency());
}

/* 読みもどしは器の番地に読んでいた（`rd_short((uint16_t *)&m_ptr->…)`）。
 * B で局所の `uint16_t` 2 つに受けてから置く形になる（save.c:649〜:650）。
 * **置きなおす窓口は創成と同じ 2 本** —— 15 つめが立てた問いへの 9 度めの
 * 答えで、守りの点数を除く 7 つと同じ側。 */
TEST(loading_a_saved_game_uses_the_very_same_windows) {
    given_search_skill_of(0, 0);

    int16_t chance_from_the_file = 32;
    int16_t frequency_from_the_file = -5;
    player_search_chance_set(chance_from_the_file);
    player_search_frequency_set(frequency_from_the_file);

    ASSERT_EQ_INT(32, player_search_chance());
    ASSERT_EQ_INT(-5, player_search_frequency());
}

/* ------------------------------------------------------------------
 * 対で足す 1 本 -- 足す 2 か所はどちらも必ず両方を動かす
 * ------------------------------------------------------------------ */

/* create.c:429・:431 —— 階級ぶんは `msrh` と `mfos` で、**どちらも正**
 * （`uint8_t`。Warrior は msrh 12・mfos 38 あたり）。 */
TEST(the_class_adds_to_both_numbers_with_two_positive_amounts) {
    given_search_skill_of(12, -5); /* 種族ぶんだけの Halfling */

    player_search_skill_adjust(12, 38); /* 階級ぶん */

    ASSERT_EQ_INT(24, player_search_chance());
    ASSERT_EQ_INT(33, player_search_frequency());
}

/* **moria1.c:71〜:72 —— 探索つきの装備は同じ `amount` を
 * `srh` には足し `fos` からは引く。** 符号は呼び手が書く
 * （`player_search_skill_adjust(amount, -amount)`）—— この窓口は
 * どちらの引数も足すだけで、**逆向きだということを知らない**。 */
TEST(searching_gear_moves_the_two_numbers_in_opposite_directions) {
    given_search_skill_of(24, 33);

    player_search_skill_adjust(5, -5); /* +5 の探索の指輪 */

    ASSERT_EQ_INT(29, player_search_chance());  /* よく見つかる */
    ASSERT_EQ_INT(28, player_search_frequency()); /* より頻繁に見る */
}

/* 外すときは呼び手の `factor` が −1 なので、同じ 1 行が戻す
 * （`amount = t_ptr->p1 * factor`）。 */
TEST(taking_the_gear_off_puts_both_numbers_back) {
    given_search_skill_of(24, 33);

    player_search_skill_adjust(5, -5);
    player_search_skill_adjust(-5, 5);

    ASSERT_EQ_INT(24, player_search_chance());
    ASSERT_EQ_INT(33, player_search_frequency());
}

/* **窓口が引く向きを持っていないことを固定する。** もし窓口が 2 つめの
 * 引数を自分で符号反転していたら、階級ぶんの `(msrh, mfos)` が嘘になる。 */
TEST(the_window_adds_both_amounts_and_negates_neither) {
    given_search_skill_of(0, 0);

    player_search_skill_adjust(3, 7); /* どちらも正 */

    ASSERT_EQ_INT(3, player_search_chance());
    ASSERT_EQ_INT(7, player_search_frequency());
}

TEST(adjusting_by_zero_moves_nothing) {
    given_search_skill_of(24, 33);

    player_search_skill_adjust(0, 0);

    ASSERT_EQ_INT(24, player_search_chance());
    ASSERT_EQ_INT(33, player_search_frequency());
}

/* 装備を重ねると積む（`py_bonuses()` は品物 1 つずつに呼ばれる）。 */
TEST(two_pieces_of_gear_keep_adding_because_each_item_calls_once) {
    given_search_skill_of(0, 40);

    player_search_skill_adjust(3, -3);
    player_search_skill_adjust(5, -5);

    ASSERT_EQ_INT(8, player_search_chance());
    ASSERT_EQ_INT(32, player_search_frequency());
}

/* 足したあとで置くと、積んだものは消える —— 創成が種族の表から置きなおす
 * のがこの形（create.c:109 のあと :429 で足す）。 */
TEST(setting_wipes_whatever_the_gear_had_added) {
    given_search_skill_of(0, 40);
    player_search_skill_adjust(9, -9);

    player_search_chance_set(12);

    ASSERT_EQ_INT(12, player_search_chance());
    ASSERT_EQ_INT(31, player_search_frequency()); /* こちらは残る */
}

/* ------------------------------------------------------------------
 * 呼び手の計算 -- 窓口は逆さにも留めもしない
 * ------------------------------------------------------------------ */

/* **人物画面だけが `40 - fos` と逆さにし、0 で下げどめる**
 * （abilities.c:51〜:54）。**逆さにするのは呼び手の仕事** ——
 * 自動探索は `fos` をそのまま `randint()` に渡す（player_move.c）。 */
TEST(the_character_sheet_turns_the_frequency_upside_down_but_the_window_does_not) {
    given_search_skill_of(0, 22);

    ASSERT_EQ_INT(22, player_search_frequency());
    ASSERT_EQ_INT(18, 40 - player_search_frequency()); /* 画面の側 */
}

/* 40 を超えた `fos`（＝ほとんど見ない人物）は画面では 0 に留められる。
 * **留めるのは呼び手** —— 窓口は 52 をそのまま返す。 */
TEST(the_sheets_floor_at_zero_is_the_callers_rule) {
    given_search_skill_of(0, 52);

    ASSERT_EQ_INT(52, player_search_frequency()); /* 窓口は何も直さない */

    int perception = 40 - player_search_frequency();
    ASSERT_EQ_INT(-12, perception);
    if (perception < 0) {
        perception = 0; /* abilities.c:52〜:54 の側 */
    }
    ASSERT_EQ_INT(0, perception);
}

/* `srh` は画面にもそのまま出る（補正が 1 つも乗らない。
 * abilities.c:56。23 つめの命中の下駄が 3 倍されたのとは違う）。 */
TEST(the_sheet_shows_the_chance_unchanged) {
    given_search_skill_of(32, 0);

    ASSERT_EQ_INT(32, player_search_chance());
}

/* 「今回見るか」の判断は呼び手にある（player_move.c）——
 * `fos <= 1` なら毎回見る。窓口はその 1 も知らない。 */
TEST(the_one_or_less_shortcut_is_the_callers_rule) {
    given_search_skill_of(0, 1);

    ASSERT_EQ_INT(1, player_search_frequency());
    ASSERT_EQ_INT(1, player_search_frequency() <= 1); /* 呼び手の条件 */
}

/* ------------------------------------------------------------------
 * 幅と留め -- 窓口は何も断らない（所見 24）
 * ------------------------------------------------------------------ */

/* wizard.c は 0..200 に囲ってから呼ぶが、**それは入り口の規則**。
 * もとのフィールドは何も留めていなかったので、窓口も留めない。 */
TEST(the_windows_refuse_nothing_the_fields_refused_nothing) {
    given_search_skill_of(1000, -1000);

    ASSERT_EQ_INT(1000, player_search_chance());
    ASSERT_EQ_INT(-1000, player_search_frequency());
}

/* 置き場は 2 バイトの**符号つき**のまま。`p_ptr->misc.srh = r_ptr->srh`
 * が切り落としていたのと同じところで折りかえす。 */
TEST(the_store_is_a_signed_short_so_thirty_two_thousand_seven_hundred_sixty_eight_wraps) {
    given_search_skill_of(32768, -32769);

    ASSERT_EQ_INT(-32768, player_search_chance());
    ASSERT_EQ_INT(32767, player_search_frequency());
}

TEST(the_widest_numbers_a_signed_short_holds_are_kept) {
    given_search_skill_of(32767, -32768);

    ASSERT_EQ_INT(32767, player_search_chance());
    ASSERT_EQ_INT(-32768, player_search_frequency());
}

/* 足す窓口も同じ幅で折りかえす（`+=` が int16_t に書きもどしていたのと同じ）。 */
TEST(adjusting_past_the_top_wraps_the_way_the_fields_always_did) {
    given_search_skill_of(32767, 0);

    player_search_skill_adjust(1, 0);

    ASSERT_EQ_INT(-32768, player_search_chance());
}

int main(void) {
    RUN_TEST(the_chance_is_whatever_creation_worked_out);
    RUN_TEST(the_frequency_is_a_different_number_from_the_chance);
    RUN_TEST(reading_either_number_twice_gives_the_same_answer);
    RUN_TEST(the_frequency_can_be_negative_because_smaller_is_better);
    RUN_TEST(zero_is_both_the_starting_state_and_an_ordinary_chance);

    RUN_TEST(the_debugging_editor_sets_the_chance_without_touching_the_frequency);
    RUN_TEST(setting_the_frequency_leaves_the_chance_alone_as_well);
    RUN_TEST(setting_again_replaces_rather_than_adds);
    RUN_TEST(the_chance_comes_first_the_way_the_saved_file_holds_it);
    RUN_TEST(loading_a_saved_game_uses_the_very_same_windows);

    RUN_TEST(the_class_adds_to_both_numbers_with_two_positive_amounts);
    RUN_TEST(searching_gear_moves_the_two_numbers_in_opposite_directions);
    RUN_TEST(taking_the_gear_off_puts_both_numbers_back);
    RUN_TEST(the_window_adds_both_amounts_and_negates_neither);
    RUN_TEST(adjusting_by_zero_moves_nothing);
    RUN_TEST(two_pieces_of_gear_keep_adding_because_each_item_calls_once);
    RUN_TEST(setting_wipes_whatever_the_gear_had_added);

    RUN_TEST(the_character_sheet_turns_the_frequency_upside_down_but_the_window_does_not);
    RUN_TEST(the_sheets_floor_at_zero_is_the_callers_rule);
    RUN_TEST(the_sheet_shows_the_chance_unchanged);
    RUN_TEST(the_one_or_less_shortcut_is_the_callers_rule);

    RUN_TEST(the_windows_refuse_nothing_the_fields_refused_nothing);
    RUN_TEST(the_store_is_a_signed_short_so_thirty_two_thousand_seven_hundred_sixty_eight_wraps);
    RUN_TEST(the_widest_numbers_a_signed_short_holds_are_kept);
    RUN_TEST(adjusting_past_the_top_wraps_the_way_the_fields_always_did);

    return TEST_SUMMARY();
}
