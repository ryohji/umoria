/* 能力値の補正値算出のテスト -- 現在の実装を保護する
 *
 * src/misc3.c の 5 関数（stat_adj:258 / tohit_adj:681 / toac_adj:728 /
 * todis_adj:755 / todam_adj:786）はいずれも「能力値 -> 段階的な補正値」を
 * 返す if-else の階段だが、比較演算子が揃っていない。
 *   stat_adj  -- > の降順
 *   tohit_adj -- < の昇順（A_DEX と A_STR の 2 段構え）
 *   toac_adj  -- < と == の混在
 *   todis_adj -- < と == の混在
 *   todam_adj -- < の昇順
 *
 * ステップ B でこれを「境界値の昇順リスト -> 補正値」のテーブル引きに
 * 統一する。そこで最も起こりやすい間違いは境界を 1 つずらすこと
 * （< 18 を <= 18 にする、境界値そのものを取りちがえる）なので、
 * このテストは各段の境界の両側（境界値 - 1 と境界値）を対で押さえる。
 *
 * 写しにしなかった理由: 写しではテーブル化後の実体を検証できず
 * 保護にならない。src/misc3.c をそのままリンクしている。
 *
 * 期待値はすべて現在の実装が返した実際の値。仕様書はないので、
 * いまどう振るまうかを固定することが目的。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "fixture.h"

extern player_type py;

/* 検証対象（src/misc3.c）。externs.h は ncurses まで引きこむので、
 * 必要な宣言だけをここに書く。 */
int stat_adj(int stat);
int tohit_adj(void);
int toac_adj(void);
int todis_adj(void);
int todam_adj(void);
int chr_adj(void);

#define MU_SETUP() fixture_reset()

#include "minunit.h"

/* ------------------------------------------------------------------
 * 条件づくりの補助関数
 *
 * fixture_reset() が py を丸ごと 0 にするので、各テストは見たい能力値
 * だけを設定する。use_stat は uint8_t（src/types.h:183）なので
 * 0..255 しか入らない。負の値は入らない。
 * ------------------------------------------------------------------ */

static void given_stat(int which, int value)
{
    py.stats.use_stat[which] = (uint8_t)value;
}

/* ------------------------------------------------------------------
 * stat_adj() -- 引数で指定した能力値を見る。8 段。
 * 境界（降順の > を昇順に読みかえたもの）:
 *   ..7 -> 0、8..14 -> 1、15..17 -> 2、18..67 -> 3、
 *   68..87 -> 4、88..107 -> 5、108..117 -> 6、118.. -> 7
 * ------------------------------------------------------------------ */

TEST(stat_adj_returns_zero_at_seven)
{
    given_stat(A_STR, 7);
    ASSERT_EQ_INT(stat_adj(A_STR), 0);
}

TEST(stat_adj_returns_one_at_eight)
{
    given_stat(A_STR, 8);
    ASSERT_EQ_INT(stat_adj(A_STR), 1);
}

TEST(stat_adj_returns_one_at_fourteen)
{
    given_stat(A_STR, 14);
    ASSERT_EQ_INT(stat_adj(A_STR), 1);
}

TEST(stat_adj_returns_two_at_fifteen)
{
    given_stat(A_STR, 15);
    ASSERT_EQ_INT(stat_adj(A_STR), 2);
}

TEST(stat_adj_returns_two_at_seventeen)
{
    given_stat(A_STR, 17);
    ASSERT_EQ_INT(stat_adj(A_STR), 2);
}

TEST(stat_adj_returns_three_at_eighteen)
{
    given_stat(A_STR, 18);
    ASSERT_EQ_INT(stat_adj(A_STR), 3);
}

TEST(stat_adj_returns_three_at_sixty_seven)
{
    given_stat(A_STR, 67);
    ASSERT_EQ_INT(stat_adj(A_STR), 3);
}

TEST(stat_adj_returns_four_at_sixty_eight)
{
    given_stat(A_STR, 68);
    ASSERT_EQ_INT(stat_adj(A_STR), 4);
}

TEST(stat_adj_returns_four_at_eighty_seven)
{
    given_stat(A_STR, 87);
    ASSERT_EQ_INT(stat_adj(A_STR), 4);
}

TEST(stat_adj_returns_five_at_eighty_eight)
{
    given_stat(A_STR, 88);
    ASSERT_EQ_INT(stat_adj(A_STR), 5);
}

TEST(stat_adj_returns_five_at_one_hundred_seven)
{
    given_stat(A_STR, 107);
    ASSERT_EQ_INT(stat_adj(A_STR), 5);
}

TEST(stat_adj_returns_six_at_one_hundred_eight)
{
    given_stat(A_STR, 108);
    ASSERT_EQ_INT(stat_adj(A_STR), 6);
}

TEST(stat_adj_returns_six_at_one_hundred_seventeen)
{
    given_stat(A_STR, 117);
    ASSERT_EQ_INT(stat_adj(A_STR), 6);
}

TEST(stat_adj_returns_seven_at_one_hundred_eighteen)
{
    given_stat(A_STR, 118);
    ASSERT_EQ_INT(stat_adj(A_STR), 7);
}

/* SUSPICIOUS: stat_adj は最小が 0 で、負を返さない。他の 4 関数は
 * 能力値が低いと負を返す（tohit_adj -6、toac_adj -4、todis_adj -8、
 * todam_adj -2）。この非対称が意図的かは判断できないので固定するだけ。 */
TEST(stat_adj_returns_zero_and_never_negative_at_zero)
{
    given_stat(A_STR, 0);
    ASSERT_EQ_INT(stat_adj(A_STR), 0);
}

/* ------------------------------------------------------------------
 * stat_adj() の引数 -- 指定した能力値だけを見ていることの確認
 *
 * 6 つの能力値に全部違う値を入れ、それぞれ違う段に落ちるようにする。
 * 引数を無視して固定の能力値を見ていたら、どれかが必ず食いちがう。
 *   A_STR 7 -> 0、A_INT 8 -> 1、A_WIS 15 -> 2、
 *   A_DEX 18 -> 3、A_CON 68 -> 4、A_CHR 88 -> 5
 * ------------------------------------------------------------------ */

static void given_all_six_stats_in_different_bands(void)
{
    given_stat(A_STR, 7);
    given_stat(A_INT, 8);
    given_stat(A_WIS, 15);
    given_stat(A_DEX, 18);
    given_stat(A_CON, 68);
    given_stat(A_CHR, 88);
}

TEST(stat_adj_reads_strength_when_given_a_str)
{
    given_all_six_stats_in_different_bands();
    ASSERT_EQ_INT(stat_adj(A_STR), 0);
}

TEST(stat_adj_reads_intelligence_when_given_a_int)
{
    given_all_six_stats_in_different_bands();
    ASSERT_EQ_INT(stat_adj(A_INT), 1);
}

TEST(stat_adj_reads_wisdom_when_given_a_wis)
{
    given_all_six_stats_in_different_bands();
    ASSERT_EQ_INT(stat_adj(A_WIS), 2);
}

TEST(stat_adj_reads_dexterity_when_given_a_dex)
{
    given_all_six_stats_in_different_bands();
    ASSERT_EQ_INT(stat_adj(A_DEX), 3);
}

TEST(stat_adj_reads_constitution_when_given_a_con)
{
    given_all_six_stats_in_different_bands();
    ASSERT_EQ_INT(stat_adj(A_CON), 4);
}

TEST(stat_adj_reads_charisma_when_given_a_chr)
{
    given_all_six_stats_in_different_bands();
    ASSERT_EQ_INT(stat_adj(A_CHR), 5);
}

/* ------------------------------------------------------------------
 * todam_adj() -- A_STR を見る。9 段。単一の能力値を見る最も単純な形。
 * 境界: ..3 -> -2、4 -> -1、5..15 -> 0、16 -> 1、17 -> 2、
 *       18..93 -> 3、94..108 -> 4、109..116 -> 5、117.. -> 6
 * ------------------------------------------------------------------ */

TEST(todam_adj_returns_minus_two_at_strength_three)
{
    given_stat(A_STR, 3);
    ASSERT_EQ_INT(todam_adj(), -2);
}

TEST(todam_adj_returns_minus_one_at_strength_four)
{
    given_stat(A_STR, 4);
    ASSERT_EQ_INT(todam_adj(), -1);
}

TEST(todam_adj_returns_zero_at_strength_five)
{
    given_stat(A_STR, 5);
    ASSERT_EQ_INT(todam_adj(), 0);
}

TEST(todam_adj_returns_zero_at_strength_fifteen)
{
    given_stat(A_STR, 15);
    ASSERT_EQ_INT(todam_adj(), 0);
}

TEST(todam_adj_returns_one_at_strength_sixteen)
{
    given_stat(A_STR, 16);
    ASSERT_EQ_INT(todam_adj(), 1);
}

TEST(todam_adj_returns_two_at_strength_seventeen)
{
    given_stat(A_STR, 17);
    ASSERT_EQ_INT(todam_adj(), 2);
}

TEST(todam_adj_returns_three_at_strength_eighteen)
{
    given_stat(A_STR, 18);
    ASSERT_EQ_INT(todam_adj(), 3);
}

TEST(todam_adj_returns_three_at_strength_ninety_three)
{
    given_stat(A_STR, 93);
    ASSERT_EQ_INT(todam_adj(), 3);
}

TEST(todam_adj_returns_four_at_strength_ninety_four)
{
    given_stat(A_STR, 94);
    ASSERT_EQ_INT(todam_adj(), 4);
}

TEST(todam_adj_returns_four_at_strength_one_hundred_eight)
{
    given_stat(A_STR, 108);
    ASSERT_EQ_INT(todam_adj(), 4);
}

TEST(todam_adj_returns_five_at_strength_one_hundred_nine)
{
    given_stat(A_STR, 109);
    ASSERT_EQ_INT(todam_adj(), 5);
}

TEST(todam_adj_returns_five_at_strength_one_hundred_sixteen)
{
    given_stat(A_STR, 116);
    ASSERT_EQ_INT(todam_adj(), 5);
}

/* SUSPICIOUS: todam_adj の最上段の境界は 117 だが、stat_adj と
 * tohit_adj（A_DEX 側）と toac_adj と todis_adj の最上段は 118 か 117 で
 * 揃っていない。todam_adj / tohit_adj(A_STR) / toac_adj / todis_adj は 117、
 * stat_adj と tohit_adj(A_DEX) は 118。 */
TEST(todam_adj_returns_six_at_strength_one_hundred_seventeen)
{
    given_stat(A_STR, 117);
    ASSERT_EQ_INT(todam_adj(), 6);
}

/* todam_adj は A_STR だけを見る。A_DEX を最大にしても変わらない。 */
TEST(todam_adj_ignores_dexterity)
{
    given_stat(A_STR, 16);
    given_stat(A_DEX, 200);
    ASSERT_EQ_INT(todam_adj(), 1);
}

/* ------------------------------------------------------------------
 * toac_adj() -- A_DEX を見る。10 段。< と == が混在している。
 *
 * ステップ B で == を < に置きかえるので、== が効いている
 * stat = 3, 4, 5, 6, 7 を 1 つずつ個別に押さえる。
 * 境界: ..3 -> -4、4 -> -3、5 -> -2、6 -> -1、7..14 -> 0、
 *       15..17 -> 1、18..58 -> 2、59..93 -> 3、94..116 -> 4、117.. -> 5
 * ------------------------------------------------------------------ */

TEST(toac_adj_returns_minus_four_at_dexterity_three)
{
    given_stat(A_DEX, 3);
    ASSERT_EQ_INT(toac_adj(), -4);
}

TEST(toac_adj_returns_minus_three_at_dexterity_four)
{
    given_stat(A_DEX, 4);
    ASSERT_EQ_INT(toac_adj(), -3);
}

TEST(toac_adj_returns_minus_two_at_dexterity_five)
{
    given_stat(A_DEX, 5);
    ASSERT_EQ_INT(toac_adj(), -2);
}

TEST(toac_adj_returns_minus_one_at_dexterity_six)
{
    given_stat(A_DEX, 6);
    ASSERT_EQ_INT(toac_adj(), -1);
}

TEST(toac_adj_returns_zero_at_dexterity_seven)
{
    given_stat(A_DEX, 7);
    ASSERT_EQ_INT(toac_adj(), 0);
}

TEST(toac_adj_returns_zero_at_dexterity_fourteen)
{
    given_stat(A_DEX, 14);
    ASSERT_EQ_INT(toac_adj(), 0);
}

TEST(toac_adj_returns_one_at_dexterity_fifteen)
{
    given_stat(A_DEX, 15);
    ASSERT_EQ_INT(toac_adj(), 1);
}

TEST(toac_adj_returns_one_at_dexterity_seventeen)
{
    given_stat(A_DEX, 17);
    ASSERT_EQ_INT(toac_adj(), 1);
}

TEST(toac_adj_returns_two_at_dexterity_eighteen)
{
    given_stat(A_DEX, 18);
    ASSERT_EQ_INT(toac_adj(), 2);
}

TEST(toac_adj_returns_two_at_dexterity_fifty_eight)
{
    given_stat(A_DEX, 58);
    ASSERT_EQ_INT(toac_adj(), 2);
}

TEST(toac_adj_returns_three_at_dexterity_fifty_nine)
{
    given_stat(A_DEX, 59);
    ASSERT_EQ_INT(toac_adj(), 3);
}

TEST(toac_adj_returns_three_at_dexterity_ninety_three)
{
    given_stat(A_DEX, 93);
    ASSERT_EQ_INT(toac_adj(), 3);
}

TEST(toac_adj_returns_four_at_dexterity_ninety_four)
{
    given_stat(A_DEX, 94);
    ASSERT_EQ_INT(toac_adj(), 4);
}

TEST(toac_adj_returns_four_at_dexterity_one_hundred_sixteen)
{
    given_stat(A_DEX, 116);
    ASSERT_EQ_INT(toac_adj(), 4);
}

TEST(toac_adj_returns_five_at_dexterity_one_hundred_seventeen)
{
    given_stat(A_DEX, 117);
    ASSERT_EQ_INT(toac_adj(), 5);
}

/* ------------------------------------------------------------------
 * todis_adj() -- A_DEX を見る。12 段（表の「段数 ?」の答えは 12）。
 * < と == が混在している。
 * 境界: ..3 -> -8、4 -> -6、5 -> -4、6 -> -2、7 -> -1、8..12 -> 0、
 *       13..15 -> 1、16..17 -> 2、18..58 -> 4、59..93 -> 5、
 *       94..116 -> 6、117.. -> 8
 *
 * SUSPICIOUS: 戻り値が 2 から 4 に飛ぶ（3 がない）。負側も -8/-6/-4/-2/-1 と
 * 刻みが揃わず、最上段も 6 から 8 に飛ぶ。他の 4 関数は 1 刻みなので
 * この関数だけ値の並びが不規則。意図的かは判断できないので固定するだけ。
 * ------------------------------------------------------------------ */

TEST(todis_adj_returns_minus_eight_at_dexterity_three)
{
    given_stat(A_DEX, 3);
    ASSERT_EQ_INT(todis_adj(), -8);
}

TEST(todis_adj_returns_minus_six_at_dexterity_four)
{
    given_stat(A_DEX, 4);
    ASSERT_EQ_INT(todis_adj(), -6);
}

TEST(todis_adj_returns_minus_four_at_dexterity_five)
{
    given_stat(A_DEX, 5);
    ASSERT_EQ_INT(todis_adj(), -4);
}

TEST(todis_adj_returns_minus_two_at_dexterity_six)
{
    given_stat(A_DEX, 6);
    ASSERT_EQ_INT(todis_adj(), -2);
}

TEST(todis_adj_returns_minus_one_at_dexterity_seven)
{
    given_stat(A_DEX, 7);
    ASSERT_EQ_INT(todis_adj(), -1);
}

TEST(todis_adj_returns_zero_at_dexterity_eight)
{
    given_stat(A_DEX, 8);
    ASSERT_EQ_INT(todis_adj(), 0);
}

TEST(todis_adj_returns_zero_at_dexterity_twelve)
{
    given_stat(A_DEX, 12);
    ASSERT_EQ_INT(todis_adj(), 0);
}

TEST(todis_adj_returns_one_at_dexterity_thirteen)
{
    given_stat(A_DEX, 13);
    ASSERT_EQ_INT(todis_adj(), 1);
}

TEST(todis_adj_returns_one_at_dexterity_fifteen)
{
    given_stat(A_DEX, 15);
    ASSERT_EQ_INT(todis_adj(), 1);
}

TEST(todis_adj_returns_two_at_dexterity_sixteen)
{
    given_stat(A_DEX, 16);
    ASSERT_EQ_INT(todis_adj(), 2);
}

TEST(todis_adj_returns_two_at_dexterity_seventeen)
{
    given_stat(A_DEX, 17);
    ASSERT_EQ_INT(todis_adj(), 2);
}

/* 3 を飛ばして 4 になる段。境界そのものは 18。 */
TEST(todis_adj_returns_four_at_dexterity_eighteen)
{
    given_stat(A_DEX, 18);
    ASSERT_EQ_INT(todis_adj(), 4);
}

TEST(todis_adj_returns_four_at_dexterity_fifty_eight)
{
    given_stat(A_DEX, 58);
    ASSERT_EQ_INT(todis_adj(), 4);
}

TEST(todis_adj_returns_five_at_dexterity_fifty_nine)
{
    given_stat(A_DEX, 59);
    ASSERT_EQ_INT(todis_adj(), 5);
}

TEST(todis_adj_returns_five_at_dexterity_ninety_three)
{
    given_stat(A_DEX, 93);
    ASSERT_EQ_INT(todis_adj(), 5);
}

TEST(todis_adj_returns_six_at_dexterity_ninety_four)
{
    given_stat(A_DEX, 94);
    ASSERT_EQ_INT(todis_adj(), 6);
}

TEST(todis_adj_returns_six_at_dexterity_one_hundred_sixteen)
{
    given_stat(A_DEX, 116);
    ASSERT_EQ_INT(todis_adj(), 6);
}

/* 7 を飛ばして 8 になる最上段。 */
TEST(todis_adj_returns_eight_at_dexterity_one_hundred_seventeen)
{
    given_stat(A_DEX, 117);
    ASSERT_EQ_INT(todis_adj(), 8);
}

/* todis_adj は A_DEX だけを見る。A_STR を最大にしても変わらない。 */
TEST(todis_adj_ignores_strength)
{
    given_stat(A_DEX, 16);
    given_stat(A_STR, 200);
    ASSERT_EQ_INT(todis_adj(), 2);
}

/* ------------------------------------------------------------------
 * tohit_adj() -- A_DEX の段と A_STR の段を足しあわせる。9 段 + 8 段。
 *
 * ステップ B ではテーブルを 2 回引く形になるので、どちらのテーブルを
 * どちらの能力値に使うかを取りちがえる余地がある。それを検出するため、
 * 片方を「その段が 0 になる値」に固定してもう片方だけを動かす。
 * 固定値は A_DEX と A_STR で違う値にしてある（10 と 12）。
 *
 * A_DEX の段: ..3 -> -3、4..5 -> -2、6..7 -> -1、8..15 -> 0、
 *             16 -> 1、17 -> 2、18..68 -> 3、69..117 -> 4、118.. -> 5
 * A_STR の段: ..3 -> -3、4 -> -2、5..6 -> -1、7..17 -> 0、
 *             18..93 -> +1、94..108 -> +2、109..116 -> +3、117.. -> +4
 * ------------------------------------------------------------------ */

/* A_STR = 12 は A_STR の段で 0（7..17 -> 0）。A_DEX の段だけが残る。
 * A_DEX の段で 12 は 0 なので、取りちがえても 0 のままにならないよう
 * A_DEX 側は 0 以外になる値でも検証する（下の各テスト）。 */
static void given_strength_contributing_nothing(void)
{
    given_stat(A_STR, 12);
}

/* A_DEX = 10 は A_DEX の段で 0（8..15 -> 0）。A_STR の段だけが残る。 */
static void given_dexterity_contributing_nothing(void)
{
    given_stat(A_DEX, 10);
}

TEST(tohit_adj_returns_minus_three_at_dexterity_three)
{
    given_strength_contributing_nothing();
    given_stat(A_DEX, 3);
    ASSERT_EQ_INT(tohit_adj(), -3);
}

TEST(tohit_adj_returns_minus_two_at_dexterity_four)
{
    given_strength_contributing_nothing();
    given_stat(A_DEX, 4);
    ASSERT_EQ_INT(tohit_adj(), -2);
}

TEST(tohit_adj_returns_minus_two_at_dexterity_five)
{
    given_strength_contributing_nothing();
    given_stat(A_DEX, 5);
    ASSERT_EQ_INT(tohit_adj(), -2);
}

TEST(tohit_adj_returns_minus_one_at_dexterity_six)
{
    given_strength_contributing_nothing();
    given_stat(A_DEX, 6);
    ASSERT_EQ_INT(tohit_adj(), -1);
}

TEST(tohit_adj_returns_minus_one_at_dexterity_seven)
{
    given_strength_contributing_nothing();
    given_stat(A_DEX, 7);
    ASSERT_EQ_INT(tohit_adj(), -1);
}

TEST(tohit_adj_returns_zero_at_dexterity_eight)
{
    given_strength_contributing_nothing();
    given_stat(A_DEX, 8);
    ASSERT_EQ_INT(tohit_adj(), 0);
}

TEST(tohit_adj_returns_zero_at_dexterity_fifteen)
{
    given_strength_contributing_nothing();
    given_stat(A_DEX, 15);
    ASSERT_EQ_INT(tohit_adj(), 0);
}

TEST(tohit_adj_returns_one_at_dexterity_sixteen)
{
    given_strength_contributing_nothing();
    given_stat(A_DEX, 16);
    ASSERT_EQ_INT(tohit_adj(), 1);
}

TEST(tohit_adj_returns_two_at_dexterity_seventeen)
{
    given_strength_contributing_nothing();
    given_stat(A_DEX, 17);
    ASSERT_EQ_INT(tohit_adj(), 2);
}

TEST(tohit_adj_returns_three_at_dexterity_eighteen)
{
    given_strength_contributing_nothing();
    given_stat(A_DEX, 18);
    ASSERT_EQ_INT(tohit_adj(), 3);
}

TEST(tohit_adj_returns_three_at_dexterity_sixty_eight)
{
    given_strength_contributing_nothing();
    given_stat(A_DEX, 68);
    ASSERT_EQ_INT(tohit_adj(), 3);
}

TEST(tohit_adj_returns_four_at_dexterity_sixty_nine)
{
    given_strength_contributing_nothing();
    given_stat(A_DEX, 69);
    ASSERT_EQ_INT(tohit_adj(), 4);
}

TEST(tohit_adj_returns_four_at_dexterity_one_hundred_seventeen)
{
    given_strength_contributing_nothing();
    given_stat(A_DEX, 117);
    ASSERT_EQ_INT(tohit_adj(), 4);
}

/* SUSPICIOUS: A_DEX 側の最上段の境界は 118 だが、A_STR 側は 117。
 * 同じ関数の中で上端が揃っていない。 */
TEST(tohit_adj_returns_five_at_dexterity_one_hundred_eighteen)
{
    given_strength_contributing_nothing();
    given_stat(A_DEX, 118);
    ASSERT_EQ_INT(tohit_adj(), 5);
}

TEST(tohit_adj_returns_minus_three_at_strength_three)
{
    given_dexterity_contributing_nothing();
    given_stat(A_STR, 3);
    ASSERT_EQ_INT(tohit_adj(), -3);
}

TEST(tohit_adj_returns_minus_two_at_strength_four)
{
    given_dexterity_contributing_nothing();
    given_stat(A_STR, 4);
    ASSERT_EQ_INT(tohit_adj(), -2);
}

TEST(tohit_adj_returns_minus_one_at_strength_five)
{
    given_dexterity_contributing_nothing();
    given_stat(A_STR, 5);
    ASSERT_EQ_INT(tohit_adj(), -1);
}

TEST(tohit_adj_returns_minus_one_at_strength_six)
{
    given_dexterity_contributing_nothing();
    given_stat(A_STR, 6);
    ASSERT_EQ_INT(tohit_adj(), -1);
}

TEST(tohit_adj_returns_zero_at_strength_seven)
{
    given_dexterity_contributing_nothing();
    given_stat(A_STR, 7);
    ASSERT_EQ_INT(tohit_adj(), 0);
}

TEST(tohit_adj_returns_zero_at_strength_seventeen)
{
    given_dexterity_contributing_nothing();
    given_stat(A_STR, 17);
    ASSERT_EQ_INT(tohit_adj(), 0);
}

TEST(tohit_adj_returns_one_at_strength_eighteen)
{
    given_dexterity_contributing_nothing();
    given_stat(A_STR, 18);
    ASSERT_EQ_INT(tohit_adj(), 1);
}

TEST(tohit_adj_returns_one_at_strength_ninety_three)
{
    given_dexterity_contributing_nothing();
    given_stat(A_STR, 93);
    ASSERT_EQ_INT(tohit_adj(), 1);
}

TEST(tohit_adj_returns_two_at_strength_ninety_four)
{
    given_dexterity_contributing_nothing();
    given_stat(A_STR, 94);
    ASSERT_EQ_INT(tohit_adj(), 2);
}

TEST(tohit_adj_returns_two_at_strength_one_hundred_eight)
{
    given_dexterity_contributing_nothing();
    given_stat(A_STR, 108);
    ASSERT_EQ_INT(tohit_adj(), 2);
}

TEST(tohit_adj_returns_three_at_strength_one_hundred_nine)
{
    given_dexterity_contributing_nothing();
    given_stat(A_STR, 109);
    ASSERT_EQ_INT(tohit_adj(), 3);
}

TEST(tohit_adj_returns_three_at_strength_one_hundred_sixteen)
{
    given_dexterity_contributing_nothing();
    given_stat(A_STR, 116);
    ASSERT_EQ_INT(tohit_adj(), 3);
}

TEST(tohit_adj_returns_four_at_strength_one_hundred_seventeen)
{
    given_dexterity_contributing_nothing();
    given_stat(A_STR, 117);
    ASSERT_EQ_INT(tohit_adj(), 4);
}

/* ------------------------------------------------------------------
 * tohit_adj() -- 2 つの能力値が合算されること、および取りちがえの検出
 *
 * A_DEX = 17（DEX 側 +2）、A_STR = 18（STR 側 +1）。合計 3。
 * テーブルを入れかえて A_DEX に STR 表・A_STR に DEX 表を使うと
 * A_DEX=17 は STR 表で 0、A_STR=18 は DEX 表で 3 なので合計 3 になって
 * しまう。よって下の 2 件（片方だけ動かすテスト）で入れかえを検出する。
 * ------------------------------------------------------------------ */

TEST(tohit_adj_sums_the_dexterity_and_strength_contributions)
{
    given_stat(A_DEX, 17); /* +2 */
    given_stat(A_STR, 18); /* +1 */
    ASSERT_EQ_INT(tohit_adj(), 3);
}

/* A_STR を 18 に据えたまま A_DEX だけを 17 -> 16 に落とす。DEX 表なら
 * +2 -> +1 で合計 2。STR 表を A_DEX に当てていたら 16 も 17 も 0 なので
 * 合計は変わらず、この差で取りちがえを検出できる。 */
TEST(tohit_adj_changes_when_only_dexterity_changes)
{
    given_stat(A_DEX, 16); /* +1 */
    given_stat(A_STR, 18); /* +1 */
    ASSERT_EQ_INT(tohit_adj(), 2);
}

/* A_DEX を 17 に据えたまま A_STR だけを 18 -> 94 に上げる。STR 表なら
 * +1 -> +2 で合計 4。DEX 表を A_STR に当てていたら 18 も 94 も +3 なので
 * 合計は変わらず、この差で取りちがえを検出できる。 */
TEST(tohit_adj_changes_when_only_strength_changes)
{
    given_stat(A_DEX, 17); /* +2 */
    given_stat(A_STR, 94); /* +2 */
    ASSERT_EQ_INT(tohit_adj(), 4);
}

/* 両方を同じ値にすると段が違うので寄与も違う。117 は DEX 表で +4、
 * STR 表で +4 なので合計 8。 */
TEST(tohit_adj_sums_both_tables_when_both_stats_are_one_hundred_seventeen)
{
    given_stat(A_DEX, 117);
    given_stat(A_STR, 117);
    ASSERT_EQ_INT(tohit_adj(), 8);
}

/* 両方 0（fixture_reset 直後の既定値）なら -3 + -3 = -6 が最小値。 */
TEST(tohit_adj_returns_minus_six_when_both_stats_are_zero)
{
    ASSERT_EQ_INT(tohit_adj(), -6);
}

/* ------------------------------------------------------------------
 * chr_adj -- 魅力による店の価格調整（百分率）
 *
 * 他の 5 関数と違い、if 連鎖 5 段と switch 17 ケースが混在している。
 * 3..18 は 1 刻みで個別に決まり、19 以上は 5 段の範囲で決まる。
 * switch の default（100）は charisma < 3 の場合。
 *
 * 値が大きいほど安く買える（90 が最良、130 が最悪）。
 * ------------------------------------------------------------------ */

static void given_charisma(int value)
{
    py.stats.use_stat[A_CHR] = (uint8_t)value;
}

/* switch の default。3 未満はすべて 100（等倍） */
TEST(chr_adj_returns_one_hundred_below_charisma_three)
{
    given_charisma(2);
    ASSERT_EQ_INT(chr_adj(), 100);
}

/* 最低の魅力 3 で最も高くつく（130%） */
TEST(chr_adj_returns_one_hundred_thirty_at_charisma_three)
{
    given_charisma(3);
    ASSERT_EQ_INT(chr_adj(), 130);
}

/* 3 → 4 で 5 下がる。ここだけ刻みが大きい（130 → 125） */
TEST(chr_adj_drops_five_points_from_charisma_three_to_four)
{
    given_charisma(4);
    ASSERT_EQ_INT(chr_adj(), 125);
}

/* 4 → 5 は 3 刻み（125 → 122） */
TEST(chr_adj_returns_one_hundred_twenty_two_at_charisma_five)
{
    given_charisma(5);
    ASSERT_EQ_INT(chr_adj(), 122);
}

/* 5 以降はおおむね 2 刻み */
TEST(chr_adj_returns_one_hundred_twenty_at_charisma_six)
{
    given_charisma(6);
    ASSERT_EQ_INT(chr_adj(), 120);
}

/* 14 → 15 で刻みが 2 から 1 に変わる（104 → 103） */
TEST(chr_adj_returns_one_hundred_four_at_charisma_fourteen)
{
    given_charisma(14);
    ASSERT_EQ_INT(chr_adj(), 104);
}

TEST(chr_adj_narrows_to_one_point_steps_at_charisma_fifteen)
{
    given_charisma(15);
    ASSERT_EQ_INT(chr_adj(), 103);
}

/* 18 で等倍（100）。switch の最上位ケース */
TEST(chr_adj_returns_one_hundred_at_charisma_eighteen)
{
    given_charisma(18);
    ASSERT_EQ_INT(chr_adj(), 100);
}

/* 19 で switch を抜けて if 連鎖の範囲に入る（100 → 98） */
TEST(chr_adj_returns_ninety_eight_at_charisma_nineteen)
{
    given_charisma(19);
    ASSERT_EQ_INT(chr_adj(), 98);
}

/* 19..67 は同じ 98。範囲の上端 */
TEST(chr_adj_still_returns_ninety_eight_at_charisma_sixty_seven)
{
    given_charisma(67);
    ASSERT_EQ_INT(chr_adj(), 98);
}

/* 68 で次の段（98 → 96） */
TEST(chr_adj_returns_ninety_six_at_charisma_sixty_eight)
{
    given_charisma(68);
    ASSERT_EQ_INT(chr_adj(), 96);
}

TEST(chr_adj_returns_ninety_four_at_charisma_eighty_eight)
{
    given_charisma(88);
    ASSERT_EQ_INT(chr_adj(), 94);
}

TEST(chr_adj_returns_ninety_two_at_charisma_one_hundred_eight)
{
    given_charisma(108);
    ASSERT_EQ_INT(chr_adj(), 92);
}

/* 117 はまだ 92。最上段の境界の下側 */
TEST(chr_adj_still_returns_ninety_two_at_charisma_one_hundred_seventeen)
{
    given_charisma(117);
    ASSERT_EQ_INT(chr_adj(), 92);
}

/* 118 で最上段（92 → 90）。最も安く買える */
TEST(chr_adj_returns_ninety_at_charisma_one_hundred_eighteen)
{
    given_charisma(118);
    ASSERT_EQ_INT(chr_adj(), 90);
}

/* uint8_t の上限でも最上段のまま */
TEST(chr_adj_returns_ninety_at_maximum_charisma)
{
    given_charisma(255);
    ASSERT_EQ_INT(chr_adj(), 90);
}

int main(void)
{
    /* --- stat_adj: 8 段の境界を両側から押さえる --- */

    RUN_TEST(stat_adj_returns_zero_at_seven);
    RUN_TEST(stat_adj_returns_one_at_eight);
    RUN_TEST(stat_adj_returns_one_at_fourteen);
    RUN_TEST(stat_adj_returns_two_at_fifteen);
    RUN_TEST(stat_adj_returns_two_at_seventeen);
    RUN_TEST(stat_adj_returns_three_at_eighteen);
    RUN_TEST(stat_adj_returns_three_at_sixty_seven);
    RUN_TEST(stat_adj_returns_four_at_sixty_eight);
    RUN_TEST(stat_adj_returns_four_at_eighty_seven);
    RUN_TEST(stat_adj_returns_five_at_eighty_eight);
    RUN_TEST(stat_adj_returns_five_at_one_hundred_seven);
    RUN_TEST(stat_adj_returns_six_at_one_hundred_eight);
    RUN_TEST(stat_adj_returns_six_at_one_hundred_seventeen);
    RUN_TEST(stat_adj_returns_seven_at_one_hundred_eighteen);
    RUN_TEST(stat_adj_returns_zero_and_never_negative_at_zero);

    /* --- stat_adj: 引数が効いていること（6 能力値を別の段に置く） --- */

    RUN_TEST(stat_adj_reads_strength_when_given_a_str);
    RUN_TEST(stat_adj_reads_intelligence_when_given_a_int);
    RUN_TEST(stat_adj_reads_wisdom_when_given_a_wis);
    RUN_TEST(stat_adj_reads_dexterity_when_given_a_dex);
    RUN_TEST(stat_adj_reads_constitution_when_given_a_con);
    RUN_TEST(stat_adj_reads_charisma_when_given_a_chr);

    /* --- todam_adj: 9 段の境界を両側から押さえる --- */

    RUN_TEST(todam_adj_returns_minus_two_at_strength_three);
    RUN_TEST(todam_adj_returns_minus_one_at_strength_four);
    RUN_TEST(todam_adj_returns_zero_at_strength_five);
    RUN_TEST(todam_adj_returns_zero_at_strength_fifteen);
    RUN_TEST(todam_adj_returns_one_at_strength_sixteen);
    RUN_TEST(todam_adj_returns_two_at_strength_seventeen);
    RUN_TEST(todam_adj_returns_three_at_strength_eighteen);
    RUN_TEST(todam_adj_returns_three_at_strength_ninety_three);
    RUN_TEST(todam_adj_returns_four_at_strength_ninety_four);
    RUN_TEST(todam_adj_returns_four_at_strength_one_hundred_eight);
    RUN_TEST(todam_adj_returns_five_at_strength_one_hundred_nine);
    RUN_TEST(todam_adj_returns_five_at_strength_one_hundred_sixteen);
    RUN_TEST(todam_adj_returns_six_at_strength_one_hundred_seventeen);
    RUN_TEST(todam_adj_ignores_dexterity);

    /* --- toac_adj: == が効く 3..7 を個別に、続けて < の各段の境界 --- */

    RUN_TEST(toac_adj_returns_minus_four_at_dexterity_three);
    RUN_TEST(toac_adj_returns_minus_three_at_dexterity_four);
    RUN_TEST(toac_adj_returns_minus_two_at_dexterity_five);
    RUN_TEST(toac_adj_returns_minus_one_at_dexterity_six);
    RUN_TEST(toac_adj_returns_zero_at_dexterity_seven);
    RUN_TEST(toac_adj_returns_zero_at_dexterity_fourteen);
    RUN_TEST(toac_adj_returns_one_at_dexterity_fifteen);
    RUN_TEST(toac_adj_returns_one_at_dexterity_seventeen);
    RUN_TEST(toac_adj_returns_two_at_dexterity_eighteen);
    RUN_TEST(toac_adj_returns_two_at_dexterity_fifty_eight);
    RUN_TEST(toac_adj_returns_three_at_dexterity_fifty_nine);
    RUN_TEST(toac_adj_returns_three_at_dexterity_ninety_three);
    RUN_TEST(toac_adj_returns_four_at_dexterity_ninety_four);
    RUN_TEST(toac_adj_returns_four_at_dexterity_one_hundred_sixteen);
    RUN_TEST(toac_adj_returns_five_at_dexterity_one_hundred_seventeen);

    /* --- todis_adj: == が効く 4..7 を個別に、続けて < の各段の境界 --- */

    RUN_TEST(todis_adj_returns_minus_eight_at_dexterity_three);
    RUN_TEST(todis_adj_returns_minus_six_at_dexterity_four);
    RUN_TEST(todis_adj_returns_minus_four_at_dexterity_five);
    RUN_TEST(todis_adj_returns_minus_two_at_dexterity_six);
    RUN_TEST(todis_adj_returns_minus_one_at_dexterity_seven);
    RUN_TEST(todis_adj_returns_zero_at_dexterity_eight);
    RUN_TEST(todis_adj_returns_zero_at_dexterity_twelve);
    RUN_TEST(todis_adj_returns_one_at_dexterity_thirteen);
    RUN_TEST(todis_adj_returns_one_at_dexterity_fifteen);
    RUN_TEST(todis_adj_returns_two_at_dexterity_sixteen);
    RUN_TEST(todis_adj_returns_two_at_dexterity_seventeen);
    RUN_TEST(todis_adj_returns_four_at_dexterity_eighteen);
    RUN_TEST(todis_adj_returns_four_at_dexterity_fifty_eight);
    RUN_TEST(todis_adj_returns_five_at_dexterity_fifty_nine);
    RUN_TEST(todis_adj_returns_five_at_dexterity_ninety_three);
    RUN_TEST(todis_adj_returns_six_at_dexterity_ninety_four);
    RUN_TEST(todis_adj_returns_six_at_dexterity_one_hundred_sixteen);
    RUN_TEST(todis_adj_returns_eight_at_dexterity_one_hundred_seventeen);
    RUN_TEST(todis_adj_ignores_strength);

    /* --- tohit_adj: A_DEX の段（A_STR は寄与 0 に固定） --- */

    RUN_TEST(tohit_adj_returns_minus_three_at_dexterity_three);
    RUN_TEST(tohit_adj_returns_minus_two_at_dexterity_four);
    RUN_TEST(tohit_adj_returns_minus_two_at_dexterity_five);
    RUN_TEST(tohit_adj_returns_minus_one_at_dexterity_six);
    RUN_TEST(tohit_adj_returns_minus_one_at_dexterity_seven);
    RUN_TEST(tohit_adj_returns_zero_at_dexterity_eight);
    RUN_TEST(tohit_adj_returns_zero_at_dexterity_fifteen);
    RUN_TEST(tohit_adj_returns_one_at_dexterity_sixteen);
    RUN_TEST(tohit_adj_returns_two_at_dexterity_seventeen);
    RUN_TEST(tohit_adj_returns_three_at_dexterity_eighteen);
    RUN_TEST(tohit_adj_returns_three_at_dexterity_sixty_eight);
    RUN_TEST(tohit_adj_returns_four_at_dexterity_sixty_nine);
    RUN_TEST(tohit_adj_returns_four_at_dexterity_one_hundred_seventeen);
    RUN_TEST(tohit_adj_returns_five_at_dexterity_one_hundred_eighteen);

    /* --- tohit_adj: A_STR の段（A_DEX は寄与 0 に固定） --- */

    RUN_TEST(tohit_adj_returns_minus_three_at_strength_three);
    RUN_TEST(tohit_adj_returns_minus_two_at_strength_four);
    RUN_TEST(tohit_adj_returns_minus_one_at_strength_five);
    RUN_TEST(tohit_adj_returns_minus_one_at_strength_six);
    RUN_TEST(tohit_adj_returns_zero_at_strength_seven);
    RUN_TEST(tohit_adj_returns_zero_at_strength_seventeen);
    RUN_TEST(tohit_adj_returns_one_at_strength_eighteen);
    RUN_TEST(tohit_adj_returns_one_at_strength_ninety_three);
    RUN_TEST(tohit_adj_returns_two_at_strength_ninety_four);
    RUN_TEST(tohit_adj_returns_two_at_strength_one_hundred_eight);
    RUN_TEST(tohit_adj_returns_three_at_strength_one_hundred_nine);
    RUN_TEST(tohit_adj_returns_three_at_strength_one_hundred_sixteen);
    RUN_TEST(tohit_adj_returns_four_at_strength_one_hundred_seventeen);

    /* --- tohit_adj: 合算と、2 つの表の取りちがえの検出 --- */

    RUN_TEST(tohit_adj_sums_the_dexterity_and_strength_contributions);
    RUN_TEST(tohit_adj_changes_when_only_dexterity_changes);
    RUN_TEST(tohit_adj_changes_when_only_strength_changes);
    RUN_TEST(tohit_adj_sums_both_tables_when_both_stats_are_one_hundred_seventeen);
    RUN_TEST(tohit_adj_returns_minus_six_when_both_stats_are_zero);

    RUN_TEST(chr_adj_returns_one_hundred_below_charisma_three);
    RUN_TEST(chr_adj_returns_one_hundred_thirty_at_charisma_three);
    RUN_TEST(chr_adj_drops_five_points_from_charisma_three_to_four);
    RUN_TEST(chr_adj_returns_one_hundred_twenty_two_at_charisma_five);
    RUN_TEST(chr_adj_returns_one_hundred_twenty_at_charisma_six);
    RUN_TEST(chr_adj_returns_one_hundred_four_at_charisma_fourteen);
    RUN_TEST(chr_adj_narrows_to_one_point_steps_at_charisma_fifteen);
    RUN_TEST(chr_adj_returns_one_hundred_at_charisma_eighteen);
    RUN_TEST(chr_adj_returns_ninety_eight_at_charisma_nineteen);
    RUN_TEST(chr_adj_still_returns_ninety_eight_at_charisma_sixty_seven);
    RUN_TEST(chr_adj_returns_ninety_six_at_charisma_sixty_eight);
    RUN_TEST(chr_adj_returns_ninety_four_at_charisma_eighty_eight);
    RUN_TEST(chr_adj_returns_ninety_two_at_charisma_one_hundred_eight);
    RUN_TEST(chr_adj_still_returns_ninety_two_at_charisma_one_hundred_seventeen);
    RUN_TEST(chr_adj_returns_ninety_at_charisma_one_hundred_eighteen);
    RUN_TEST(chr_adj_returns_ninety_at_maximum_charisma);

    return TEST_SUMMARY();
}
