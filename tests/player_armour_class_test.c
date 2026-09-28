// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「守りの点数はいくつか」のテスト -- 現在のふるまいを保護する
 *
 * py から出す 17 つめの問いで、**`struct misc` から出る 3 つめ**
 * （1 つめはどこまで潜ったか、2 つめは体力の骰子）。答えは short 2 本 ——
 * もとは py.misc.pac（Total AC）と py.misc.ptoac（Magical AC）で、
 * 6 ファイルから 73 か所が名ざしていた（47 行）。
 *
 * **2 つのフィールドで 1 つの問い。** 分けめは「点の出どころ」であって
 * 意味ではない —— 着ているものの `ac` の合計と呪文のぶんが片方、
 * 敏捷さの下駄 toac_adj() と着ているものの `toac` の合計がもう片方。
 *
 * **読み手 26 か所は 1 つも片方だけを読まない。** ぜんぶ `pac + ptoac` で、
 * だから create.c が種族のときと階級のときで**半分を入れかえている**のに
 * 誰も気づけない（どちらも和は同じ）。証拠はこのファイルの最後の組。
 *
 * **大きいほど当たりにくい。** 読み手 25 か所は test_hit() に
 * 「攻める側が超えねばならない数」として渡し、残る 1 つは
 * 当たった打撃を `* damage / 200` だけ削る。
 *
 * 当たり判定そのもの・打撃の減りかた・画面に出る数・呪文の時計は
 * **この module の外**（player_armour_class.h）。とくに**画面の数は別の問い**
 * で、prt_pac() が読むのは player_display_ac() のほう。
 *
 * テストは 1 プロセスで状態を共有するので、各件が最初に 2 本を置きなおす。
 */
/* externs.h は要らない。窓口 7 本と、型のための types.h だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_armour_class.h"

#include "minunit.h"

/* 何も着ていない人物から始める（calc_bonuses() の入り口と同じ形）。
 * 引数は敏捷さの下駄 toac_adj() のぶんで、-4 から 5 までを取る。 */
static void given_a_character_wearing_nothing(int dexterity_bonus) { player_armour_class_reset(dexterity_bonus); }

/* ------------------------------------------------------------------
 * 点数そのもの -- 読み手 26 か所が読む和
 * ------------------------------------------------------------------ */

TEST(a_character_wearing_nothing_has_only_the_dexterity_bonus) {
    given_a_character_wearing_nothing(3);

    ASSERT_EQ_INT(3, player_armour_class());
}

TEST(reading_the_points_twice_gives_the_same_answer) {
    given_a_character_wearing_nothing(2);

    ASSERT_EQ_INT(2, player_armour_class());
    ASSERT_EQ_INT(2, player_armour_class());
}

/* 敏捷さの下駄は負にもなる（toac_adj の表は -4 から 5）。
 * **留めは無い** —— もとのコードが検査していないので、窓口もしない。 */
TEST(a_clumsy_character_is_easier_to_hit_than_a_bare_one) {
    given_a_character_wearing_nothing(-4);

    ASSERT_EQ_INT(-4, player_armour_class());
}

/* 敏捷さがふつうなら 0。**0 は「まだ決まっていない」ではない** ——
 * 深さの 0 階や骰子の 0 面とちがって、この問いの 0 は遊びのなかの答え。 */
TEST(zero_is_a_real_answer_and_not_an_unset_one) {
    given_a_character_wearing_nothing(0);

    ASSERT_EQ_INT(0, player_armour_class());
}

/* ------------------------------------------------------------------
 * 作りなおし -- calc_bonuses() が装備が変わるたびにやること
 * ------------------------------------------------------------------ */

/* **着ているものぶんは必ず 0 に戻る。** これが `_reset()` の持っている規則で、
 * だからセーブの読みもどしはこの窓口を通れない。 */
TEST(rebuilding_throws_away_what_was_worn) {
    given_a_character_wearing_nothing(2);
    player_armour_class_add_item(14, 0);

    player_armour_class_reset(2);

    ASSERT_EQ_INT(2, player_armour_class());
}

TEST(rebuilding_replaces_the_dexterity_bonus_too) {
    given_a_character_wearing_nothing(5);

    player_armour_class_reset(-1);

    ASSERT_EQ_INT(-1, player_armour_class());
}

/* 作りなおしは足しではない —— 2 度呼んでも下駄が 2 倍にならない
 * （calc_bonuses() は 1 ターンに何度も呼ばれる）。 */
TEST(rebuilding_twice_does_not_double_the_bonus) {
    given_a_character_wearing_nothing(3);

    player_armour_class_reset(3);

    ASSERT_EQ_INT(3, player_armour_class());
}

/* ------------------------------------------------------------------
 * 着ているもの -- calc_bonuses() が装備の枠を 1 つずつ歩く
 * ------------------------------------------------------------------ */

TEST(one_worn_item_adds_its_own_armour) {
    given_a_character_wearing_nothing(0);

    player_armour_class_add_item(4, 0); /* Soft Leather Armor */

    ASSERT_EQ_INT(4, player_armour_class());
}

/* 装備は積む —— 6 枠ぜんぶを歩くのが calc_bonuses() の仕事。 */
TEST(every_worn_slot_piles_on) {
    given_a_character_wearing_nothing(1);

    player_armour_class_add_item(14, 0); /* Chain Mail */
    player_armour_class_add_item(3, 0);  /* Hard Leather Cap 相当 */
    player_armour_class_add_item(2, 0);  /* Leather Gloves 相当 */

    ASSERT_EQ_INT(20, player_armour_class());
}

/* 1 つのものが両方の半分に足す。**和で見れば 1 つの足し**だが、
 * ファイルに出るときだけ分かれている。 */
TEST(a_magic_item_adds_to_both_halves_at_once) {
    given_a_character_wearing_nothing(0);

    player_armour_class_add_item(14, 3);

    ASSERT_EQ_INT(17, player_armour_class());
    ASSERT_EQ_INT(14, player_armour_class_armour());
    ASSERT_EQ_INT(3, player_armour_class_magical());
}

/* 錆びた鎖帷子は `ac` 14 の `toac` -8 —— **着るほど弱くなるものがある**。
 * 窓口は断らない（もとのコードも断っていない）。 */
TEST(a_cursed_suit_can_take_more_away_than_it_gives) {
    given_a_character_wearing_nothing(0);

    player_armour_class_add_item(14, -8);

    ASSERT_EQ_INT(6, player_armour_class());
}

/* ------------------------------------------------------------------
 * 呪文のぶん -- 二重帳簿の片方（moria1.c）と、もう片方（dungeon.c）
 * ------------------------------------------------------------------ */

TEST(invulnerability_is_worth_a_hundred_points) {
    given_a_character_wearing_nothing(0);

    player_armour_class_adjust(100);

    ASSERT_EQ_INT(100, player_armour_class());
}

TEST(a_blessing_is_worth_two_points) {
    given_a_character_wearing_nothing(0);

    player_armour_class_adjust(2);

    ASSERT_EQ_INT(2, player_armour_class());
}

/* 同じ窓口が足しも引きもする。dungeon.c は呪文が始まった瞬間に足して
 * 切れた瞬間に引くので、**行きと帰りで元に戻らなければならない**。 */
TEST(a_spell_that_runs_out_leaves_the_points_where_they_were) {
    given_a_character_wearing_nothing(2);
    player_armour_class_add_item(14, 0);

    player_armour_class_adjust(100);
    player_armour_class_adjust(-100);

    ASSERT_EQ_INT(16, player_armour_class());
}

/* 2 つの呪文は重なる（不死身 100 ＋ 祝福 2）。 */
TEST(two_spells_at_once_both_count) {
    given_a_character_wearing_nothing(0);

    player_armour_class_adjust(100);
    player_armour_class_adjust(2);

    ASSERT_EQ_INT(102, player_armour_class());
}

/* **呪文のぶんは着ているものの側に積まれる**（もとの `pac += 100`）。
 * 意味からすれば魔法の側に見えるが、もとのコードはそうしていない ——
 * 分けめが「点の出どころ」ではなく「どちらのフィールドに書かれていたか」
 * だという証拠で、セーブファイルの 2 本にそのまま出る。 */
TEST(a_spells_points_land_in_the_armour_half_not_the_magical_one) {
    given_a_character_wearing_nothing(3);

    player_armour_class_adjust(100);

    ASSERT_EQ_INT(100, player_armour_class_armour());
    ASSERT_EQ_INT(3, player_armour_class_magical());
}

/* **作りなおすと呪文のぶんも消える。** だから calc_bonuses() は
 * 効いている呪文のぶんを毎回足しなおす（moria1.c:152・:156）。 */
TEST(rebuilding_forgets_the_spell_and_the_caller_must_add_it_again) {
    given_a_character_wearing_nothing(0);
    player_armour_class_adjust(100);

    player_armour_class_reset(0);

    ASSERT_EQ_INT(0, player_armour_class());
}

/* ------------------------------------------------------------------
 * セーブファイル -- short 2 本をそのまま置く別の窓口
 * ------------------------------------------------------------------ */

TEST(loading_a_saved_game_brings_both_halves_back) {
    given_a_character_wearing_nothing(0);

    player_armour_class_set_parts(16, 3);

    ASSERT_EQ_INT(19, player_armour_class());
    ASSERT_EQ_INT(16, player_armour_class_armour());
    ASSERT_EQ_INT(3, player_armour_class_magical());
}

/* **ファイルの数は着ているものを含んだまま**なので、
 * `_reset()` のように着ているものぶんを 0 にしてはいけない。
 * 15 つめに立てた問い（読みもどしは同じ文か）への 2 度めの答えで、
 * 今度は「違う」。 */
TEST(the_file_keeps_the_worn_armour_and_the_restore_must_not_wipe_it) {
    given_a_character_wearing_nothing(0);

    player_armour_class_set_parts(35, 5);

    ASSERT_EQ_INT(35, player_armour_class_armour());
}

TEST(the_two_shorts_that_go_out_are_the_ones_that_came_in) {
    const int armour[] = {0, 1, 4, 16, 35, 135, -8, 32767, -32768};

    for (int i = 0; i < 9; i++) {
        player_armour_class_set_parts(armour[i], 0);

        ASSERT_EQ_INT(armour[i], player_armour_class_armour());
    }
}

/* 置きなおしは積みではない —— 大きい人物のところへ小さいセーブを
 * 読みこんでも混ざらない。 */
TEST(a_saved_game_replaces_rather_than_adds) {
    given_a_character_wearing_nothing(5);
    player_armour_class_add_item(35, 5);

    player_armour_class_set_parts(4, 0);

    ASSERT_EQ_INT(4, player_armour_class());
}

/* ------------------------------------------------------------------
 * 半分の分けめは誰にも見えない -- create.c の入れかえ
 * ------------------------------------------------------------------ */

/* **create.c は種族のときと階級のときで下駄の置き場を入れかえている。**
 * 種族のときは着ているものの側（`pac = toac_adj(); ptoac = 0;`）、
 * 階級のときは魔法の側（`ptoac = toac_adj(); pac = 0;`）。
 * **和はどちらも同じ**なので、読み手 26 か所には区別がつかない。 */
TEST(the_two_spellings_create_c_uses_add_up_to_the_same_number) {
    player_armour_class_set_parts(3, 0); /* 種族のとき */
    const int after_the_race = player_armour_class();

    player_armour_class_reset(3); /* 階級のとき */
    const int after_the_class = player_armour_class();

    ASSERT_EQ_INT(after_the_race, after_the_class);
    ASSERT_EQ_INT(3, after_the_class);
}

/* 分けめが見えるのはファイルの 2 本と画面の初期値だけ。
 * 上の 2 つの綴りは、半分ずつ見ればちゃんと違う。 */
TEST(the_halves_themselves_do_tell_the_two_spellings_apart) {
    player_armour_class_set_parts(3, 0);

    ASSERT_EQ_INT(3, player_armour_class_armour());
    ASSERT_EQ_INT(0, player_armour_class_magical());

    player_armour_class_reset(3);

    ASSERT_EQ_INT(0, player_armour_class_armour());
    ASSERT_EQ_INT(3, player_armour_class_magical());
}

int main(void) {
    RUN_TEST(a_character_wearing_nothing_has_only_the_dexterity_bonus);
    RUN_TEST(reading_the_points_twice_gives_the_same_answer);
    RUN_TEST(a_clumsy_character_is_easier_to_hit_than_a_bare_one);
    RUN_TEST(zero_is_a_real_answer_and_not_an_unset_one);

    RUN_TEST(rebuilding_throws_away_what_was_worn);
    RUN_TEST(rebuilding_replaces_the_dexterity_bonus_too);
    RUN_TEST(rebuilding_twice_does_not_double_the_bonus);

    RUN_TEST(one_worn_item_adds_its_own_armour);
    RUN_TEST(every_worn_slot_piles_on);
    RUN_TEST(a_magic_item_adds_to_both_halves_at_once);
    RUN_TEST(a_cursed_suit_can_take_more_away_than_it_gives);

    RUN_TEST(invulnerability_is_worth_a_hundred_points);
    RUN_TEST(a_blessing_is_worth_two_points);
    RUN_TEST(a_spell_that_runs_out_leaves_the_points_where_they_were);
    RUN_TEST(two_spells_at_once_both_count);
    RUN_TEST(a_spells_points_land_in_the_armour_half_not_the_magical_one);
    RUN_TEST(rebuilding_forgets_the_spell_and_the_caller_must_add_it_again);

    RUN_TEST(loading_a_saved_game_brings_both_halves_back);
    RUN_TEST(the_file_keeps_the_worn_armour_and_the_restore_must_not_wipe_it);
    RUN_TEST(the_two_shorts_that_go_out_are_the_ones_that_came_in);
    RUN_TEST(a_saved_game_replaces_rather_than_adds);

    RUN_TEST(the_two_spellings_create_c_uses_add_up_to_the_same_number);
    RUN_TEST(the_halves_themselves_do_tell_the_two_spellings_apart);

    return TEST_SUMMARY();
}
