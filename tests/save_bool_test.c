/* セーブデータ中の真偽値の読み書きのテスト -- 現在のふるまいを保護する
 *
 * panic_save（緊急セーブからの再開か）と total_winner（勝利済みか）は bool
 * なのに、セーブファイル上では wr_short() で 2 バイトとして書かれている
 * （save.c:247-248）。読むほうは rd_short((uint16_t *)&panic_save) と書いて
 * あり、1 バイトの bool に 2 バイト書きこんでいた。-Warray-bounds= が
 * 「partly outside array bounds of '_Bool[1]'」と指していたのはこれ。
 *
 * 配列の外に書いていること自体はコンパイラが見ており、直った証拠は警告が
 * 消えることで得られる。テストで押さえるのはそこではなく、直すときに
 * 変えてはいけないほう -- セーブファイルの形式（真偽値 1 つが 2 バイト）と
 * 値の対応（0 → false、1 → true）。ここが動くと既存のセーブファイルが
 * 読めなくなる。
 *
 * 読み書きの実体は save.c の static 関数なので、写しではなく実体を検証
 * するためにこのテストが src/save.c を #include して取りこむ。
 * fileptr / xor_byte も static で、テストから直接触れる。
 *
 * XOR による難読化は wr_byte / rd_byte が対称に行うので、書きはじめと
 * 読みはじめで xor_byte を同じ値にしておけば元の値に戻る。本体では
 * その種を保存ファイルの先頭バイトに置いている（save.c:434-444, 529-536）が、
 * ここで見たいのはバイト数と値の対応だけなので、両方 0 から始める。
 */
#include "save.c"

/* 一時ファイルはテストごとに閉じる。minunit.h が既定の空実装を用意する前に
 * 差しかえる（end_of_file() の定義は下にあるが、使われるのは main() の中）。 */
#define MU_TEARDOWN() end_of_file()
#include "minunit.h"

/* 読み書きの相手は実ファイルではなく一時ファイル。fopen は externs.h が
 * tfopen に置きかえてしまう（externs.h:262）ので tmpfile() を使う。 */
static void begin_recording(void)
{
    fileptr = tmpfile();
    xor_byte = 0;
}

static void begin_playback(void)
{
    rewind(fileptr);
    xor_byte = 0;
}

static void end_of_file(void)
{
    if (fileptr != NULL) {
        (void)fclose(fileptr);
        fileptr = NULL;
    }
}

/* 本体（save.c:247）と同じ書きかたで真偽値を書き、読みかえす。 */
static bool round_trip(bool value)
{
    begin_recording();
    wr_short((uint16_t)value);

    bool read_back = !value; /* 読めていないと落ちるように逆で埋める */
    begin_playback();
    rd_bool(&read_back);

    return read_back;
}

TEST(saved_false_reads_back_as_false)
{
    ASSERT_FALSE(round_trip(false));
}

TEST(saved_true_reads_back_as_true)
{
    ASSERT_TRUE(round_trip(true));
}

TEST(a_saved_bool_occupies_exactly_two_bytes)
{
    /* 真偽値のあとに続く値がずれずに読めることで、消費バイト数を見る。
     * ここがずれると既存のセーブファイルが読めなくなる。 */
    begin_recording();
    wr_short((uint16_t)true);
    wr_short(0x1234);

    bool flag = false;
    uint16_t next = 0;
    begin_playback();
    rd_bool(&flag);
    rd_short(&next);

    ASSERT_EQ_INT(next, 0x1234);
}

TEST(any_nonzero_value_reads_back_as_true)
{
    /* 本体が書くのは 0 か 1 だけなので、それ以外はこのプログラムが作った
     * ファイルには現れない。それでも下位バイトだけを見ると 0x0100 が false に
     * なるので、2 バイト全体を見て判定していることを固定しておく。 */
    begin_recording();
    wr_short(0x0100);

    bool flag = false;
    begin_playback();
    rd_bool(&flag);

    ASSERT_TRUE(flag);
}

int main(void)
{
    RUN_TEST(saved_false_reads_back_as_false);
    RUN_TEST(saved_true_reads_back_as_true);
    RUN_TEST(a_saved_bool_occupies_exactly_two_bytes);
    RUN_TEST(any_nonzero_value_reads_back_as_true);
    return TEST_SUMMARY();
}
