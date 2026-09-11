/* オプション（ユーザー設定 11 個）のテスト -- 現在のふるまいを保護する
 *
 * 11 個のオプションについて、3 箇所が同じことを知っていた。
 *   misc2.c:856  表示名と値の置き場（設定画面の並び順）
 *   save.c:64    値 → セーブファイルのビット（書くとき）
 *   save.c:571   セーブファイルのビット → 値（読むとき）
 * 一致していなければセーブファイルが壊れるのに、一致を保証するものが
 * 何も無かった。src/options.c の 1 つの表に寄せる。
 *
 * 保護のとりかた。書きこみ・読みこみの実体は save.c の sv_write() /
 * rd_*() の中に直に並んだ if 文で、関数として外から呼べない。だから
 * 「先にテストを書いて、本体を変えずにグリーン」の形がとれない。
 * 代わりに #7（device.c）と同じ「テスト側に写す」方式をとる。
 * 変更前の save.c の if 文の並びをこのファイルに写しとり（legacy_pack /
 * legacy_unpack）、新しい実体（game_options_pack / game_options_unpack）と
 * 2^11 = 2048 通りすべてで突きあわせる。写しが正しいことは目で照合できる
 * 形にしてあり、等価性のほうは全数検査で機械が保証する。
 *
 * セーブファイル 5.2.2 以前の互換（sound_beep_flag と display_counts を
 * 強制的に on にする）はここには写していない。あれはビットの対応ではなく
 * 「古い形式には該当ビットが無い」という別の話で、save.c に残す。
 */
/* externs.h は単体では読めない（vtype も bool も types.h 側にある）。
 * 本体の .c と同じ順で先に並べる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"
#include "options.h"

#include "minunit.h"

/* --- 変更前の save.c の写し ---------------------------------------------
 * save.c:64-97 の if 文 11 個。順序も値もそのまま。 */
static uint32_t legacy_pack(void)
{
    uint32_t l = 0;

    if (find_cut) {
        l |= 0x1;
    }
    if (find_examine) {
        l |= 0x2;
    }
    if (find_prself) {
        l |= 0x4;
    }
    if (find_bound) {
        l |= 0x8;
    }
    if (prompt_carry_flag) {
        l |= 0x10;
    }
    if (rogue_like_commands) {
        l |= 0x20;
    }
    if (show_weight_flag) {
        l |= 0x40;
    }
    if (highlight_seams) {
        l |= 0x80;
    }
    if (find_ignore_doors) {
        l |= 0x100;
    }
    if (sound_beep_flag) {
        l |= 0x200;
    }
    if (display_counts) {
        l |= 0x400;
    }
    return l;
}

/* save.c:573-635 の if/else 11 組。互換処理の 2 つは上記のとおり除く。 */
static void legacy_unpack(uint32_t l)
{
    if (l & 0x1) {
        find_cut = true;
    } else {
        find_cut = false;
    }
    if (l & 0x2) {
        find_examine = true;
    } else {
        find_examine = false;
    }
    if (l & 0x4) {
        find_prself = true;
    } else {
        find_prself = false;
    }
    if (l & 0x8) {
        find_bound = true;
    } else {
        find_bound = false;
    }
    if (l & 0x10) {
        prompt_carry_flag = true;
    } else {
        prompt_carry_flag = false;
    }
    if (l & 0x20) {
        rogue_like_commands = true;
    } else {
        rogue_like_commands = false;
    }
    if (l & 0x40) {
        show_weight_flag = true;
    } else {
        show_weight_flag = false;
    }
    if (l & 0x80) {
        highlight_seams = true;
    } else {
        highlight_seams = false;
    }
    if (l & 0x100) {
        find_ignore_doors = true;
    } else {
        find_ignore_doors = false;
    }
    if (l & 0x200) {
        sound_beep_flag = true;
    } else {
        sound_beep_flag = false;
    }
    if (l & 0x400) {
        display_counts = true;
    } else {
        display_counts = false;
    }
}

/* --- 11 個の値をまとめて出し入れする ------------------------------------
 * 表を使わずに並べる。表そのものを検査したいので、検査側が表に頼っていては
 * 意味がない。ここでの並び順は組みあわせの数えあげの都合だけで、画面の
 * 並び順やビットの順とは関係しない。 */
static bool *const values[] = {
    &find_cut, &find_examine, &find_prself, &find_bound, &prompt_carry_flag,
    &rogue_like_commands, &show_weight_flag, &highlight_seams,
    &find_ignore_doors, &sound_beep_flag, &display_counts,
};
#define VALUE_COUNT ((int)(sizeof values / sizeof values[0]))
#define COMBINATIONS (1u << VALUE_COUNT) /* 2048 通り */

static void spread(unsigned combination)
{
    for (int i = 0; i < VALUE_COUNT; i++) {
        *values[i] = ((combination >> i) & 1u) != 0;
    }
}

static unsigned gather(void)
{
    unsigned combination = 0;

    for (int i = 0; i < VALUE_COUNT; i++) {
        if (*values[i]) {
            combination |= 1u << i;
        }
    }
    return combination;
}

/* --- テスト ------------------------------------------------------------- */

TEST(the_table_has_eleven_options)
{
    ASSERT_EQ_INT(game_options_count(), VALUE_COUNT);
}

TEST(the_table_ends_with_a_null_prompt)
{
    /* set_options() は終端を探して項目数を数える（misc2.c:876）。
     * 終端が無いと表の外を読む。 */
    ASSERT_TRUE(game_options[game_options_count()].prompt == NULL);
}

TEST(packing_matches_the_original_for_every_combination)
{
    unsigned mismatch = COMBINATIONS;

    for (unsigned c = 0; c < COMBINATIONS; c++) {
        spread(c);
        if (game_options_pack() != legacy_pack()) {
            mismatch = c;
            break;
        }
    }
    ASSERT_EQ_INT(mismatch, COMBINATIONS);
}

TEST(unpacking_matches_the_original_for_every_combination)
{
    unsigned mismatch = COMBINATIONS;

    /* 入れる語は 11 ビットぶん。写しと新しい実体に同じ語を渡し、
     * 11 個の値がそろって同じになることを見る。 */
    for (unsigned bits = 0; bits < COMBINATIONS; bits++) {
        legacy_unpack(bits);
        unsigned expected = gather();

        spread(~bits & (COMBINATIONS - 1)); /* 前の結果が残らないよう反転で埋める */
        game_options_unpack(bits);
        if (gather() != expected) {
            mismatch = bits;
            break;
        }
    }
    ASSERT_EQ_INT(mismatch, COMBINATIONS);
}

TEST(packing_touches_only_the_eleven_option_bits)
{
    /* 語は死亡（0x80000000）と勝利済み（0x40000000）と同居している
     * （save.c:99-105）。オプション側がそこに触ると別の意味になる。 */
    spread(COMBINATIONS - 1);
    ASSERT_EQ_INT(game_options_pack(), 0x7FF);
}

TEST(unpacking_ignores_the_bits_that_share_the_word)
{
    game_options_unpack(0xC0000000u);
    ASSERT_EQ_INT(gather(), 0);
}

TEST(every_option_has_a_bit_of_its_own)
{
    uint32_t seen = 0;
    int overlap = -1;

    for (int i = 0; game_options[i].prompt != NULL; i++) {
        if (seen & game_options[i].save_bit) {
            overlap = i;
            break;
        }
        seen |= game_options[i].save_bit;
    }
    ASSERT_EQ_INT(overlap, -1);
}

TEST(the_prompts_are_in_the_order_the_options_screen_showed_them)
{
    /* misc2.c:857-867 の写し。設定画面の並びはユーザーに見えるふるまい
     * なので、表に寄せるときに動いてはいけない。 */
    static const char *const shown[] = {
        "Running: cut known corners",
        "Running: examine potential corners",
        "Running: print self during run",
        "Running: stop when map sector changes",
        "Running: run through open doors",
        "Prompt to pick up objects",
        "Rogue like commands",
        "Show weights in inventory",
        "Highlight and notice mineral seams",
        "Beep for invalid character",
        "Display rest/repeat counts",
    };
    int mismatch = -1;

    for (int i = 0; i < VALUE_COUNT; i++) {
        if (strcmp(game_options[i].prompt, shown[i]) != 0) {
            mismatch = i;
            break;
        }
    }
    ASSERT_EQ_INT(mismatch, -1);
}

int main(void)
{
    RUN_TEST(the_table_has_eleven_options);
    RUN_TEST(the_table_ends_with_a_null_prompt);
    RUN_TEST(packing_matches_the_original_for_every_combination);
    RUN_TEST(unpacking_matches_the_original_for_every_combination);
    RUN_TEST(packing_touches_only_the_eleven_option_bits);
    RUN_TEST(unpacking_ignores_the_bits_that_share_the_word);
    RUN_TEST(every_option_has_a_bit_of_its_own);
    RUN_TEST(the_prompts_are_in_the_order_the_options_screen_showed_them);
    return TEST_SUMMARY();
}
