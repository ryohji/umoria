/* 店 6 軒のセーブデータの読み書きのテスト -- 現在のふるまいを保護する
 *
 * 店の記録の並び（開店時刻・侮辱回数・店主・在庫数・値切りの成績、そして
 * 在庫のある枠だけ値段と品物）は save.c の 3 か所に書かれていた。
 *   save.c:227-241  書く（sv_write）
 *   save.c:699-716  読む（いまの形式）
 *   save.c:872-888  読む（5.1.3 より前の形式）
 * 読む 2 か所は字下げをのぞいて同一の 17 行。品物 1 つ（wr_item / rd_item）は
 * すでに関数になっていたのに、店 1 軒はなっていなかった。同じ形にして
 * wr_store() / rd_store() にまとめる。
 *
 * 保護のとりかた。変更してはいけないのはセーブファイルのバイト列そのもの
 * （既存のセーブファイルが読めなくなる）なので、#18-1 / #18-3 と同じ
 * 「テスト側に写す」方式で変更前の 3 か所をこのファイルに写しとり
 * （legacy_*）、新しい実体と突きあわせる。
 *   1. 同じ店を写しと実体で書き、出たバイト列が 1 バイトも違わないこと
 *   2. 写しが書いたものを実体が読み、店の記録が丸ごと戻ること
 *   3. 実体が書いたものを写しが読み、同じく戻ること
 *   4. 在庫数がありえない値なら読みを断ること（変更前の goto error）
 * 2 と 3 は memcmp で記録を丸ごと比べる。書く前に 0 で埋めておくので、
 * 復元しそこねた項目が 1 つでもあれば落ちる。
 *
 * 読み書きの実体は save.c の static なので、写しではなく実体を検証する
 * ためにこのテストが src/save.c を #include して取りこむ（tests/
 * save_bool_test.c と同じ作り。fileptr / xor_byte も static なので、
 * 一時ファイルと XOR の種をテスト側から直に置ける）。
 */
#include "save.c"

#include <string.h>

#define MU_TEARDOWN() end_of_file()
#include "minunit.h"

/* --- 一時ファイル（save_bool_test.c と同じ作り） ------------------------ */

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

/* --- 変更前の写し ------------------------------------------------------- */

/* save.c:227-241 の写し。 */
static void legacy_wr_stores(void)
{
    for (int i = 0; i < MAX_STORES; i++) {
        store_type *st_ptr = store_at(i);

        wr_long((uint32_t)st_ptr->store_open);
        wr_short((uint16_t)st_ptr->insult_cur);
        wr_byte(st_ptr->owner);
        wr_byte(st_ptr->store_ctr);
        wr_short(st_ptr->good_buy);
        wr_short(st_ptr->bad_buy);
        for (int j = 0; j < st_ptr->store_ctr; j++) {
            wr_long((uint32_t)st_ptr->store_inven[j].scost);
            wr_item(&st_ptr->store_inven[j].sitem);
        }
    }
}

/* save.c:699-716 / 872-888 の写し（この 2 つは同一だった）。goto error は
 * 呼びだし側が読みこみを投げだす合図なので、写しでは false を返す。 */
static bool legacy_rd_stores(void)
{
    for (int i = 0; i < MAX_STORES; i++) {
        store_type *st_ptr = store_at(i);

        rd_long((uint32_t *)&st_ptr->store_open);
        rd_short((uint16_t *)&st_ptr->insult_cur);
        rd_byte(&st_ptr->owner);
        rd_byte(&st_ptr->store_ctr);
        rd_short(&st_ptr->good_buy);
        rd_short(&st_ptr->bad_buy);
        if (st_ptr->store_ctr > STORE_INVEN_MAX) {
            return false;
        }
        for (int j = 0; j < st_ptr->store_ctr; j++) {
            rd_long((uint32_t *)&st_ptr->store_inven[j].scost);
            rd_item(&st_ptr->store_inven[j].sitem);
        }
    }

    return true;
}

/* 新しい実体を 6 軒ぶん回す側。呼びだし側（save.c:230, 691, 852）の写し。 */
static void wr_all_stores(void)
{
    for (int i = 0; i < store_count(); i++) {
        wr_store(store_at(i));
    }
}

static bool rd_all_stores(void)
{
    for (int i = 0; i < store_count(); i++) {
        if (!rd_store(store_at(i))) {
            return false;
        }
    }

    return true;
}

/* --- 突きあわせの下ごしらえ --------------------------------------------- */

/* 6 軒ぶんの手本。項目ごとに違う値を入れるので、書く順・読む順がずれれば
 * 必ず食いちがう。在庫は 0 枠・1 枠・満杯（24 枠）を混ぜる。 */
static store_type expected[MAX_STORES];

static void build_expected(void)
{
    /* 埋めない項目（構造体の詰めものも含む）を 0 にしておく。実体の側も
     * 読む前に 0 で埋めるので、memcmp で丸ごと比べられる。 */
    memset(expected, 0, sizeof(expected));

    for (int i = 0; i < MAX_STORES; i++) {
        store_type *s_ptr = &expected[i];

        s_ptr->store_open = 100000L + i;   /* 4 バイト。turn 番号なので大きい */
        s_ptr->insult_cur = (int16_t)(i + 1);
        s_ptr->owner = (uint8_t)(10 + i);
        s_ptr->good_buy = (uint16_t)(1000 + i);
        s_ptr->bad_buy = (uint16_t)(2000 + i);

        /* 在庫の枠数: 0, 1, 2, ..., ただし 1 軒は満杯にする */
        int items = i;
        if (i == MAX_STORES - 1) {
            items = STORE_INVEN_MAX;
        }
        s_ptr->store_ctr = (uint8_t)items;

        for (int j = 0; j < items; j++) {
            inven_record *rec = &s_ptr->store_inven[j];
            inven_type *item = &rec->sitem;

            rec->scost = -(1000L * (i + 1) + j); /* 値段は負で持っている */
            item->index = (uint16_t)(300 + j);
            item->name2 = (uint8_t)(j + 1);
            (void)strcpy(item->inscrip, "sale");
            item->flags = 0x80000000u >> j;
            item->tval = (uint8_t)(TV_SWORD);
            item->tchar = (uint8_t)'|';
            item->p1 = (int16_t)(-j - 1);
            item->cost = 5000L + j;
            item->subval = (uint8_t)(j + 2);
            item->number = (uint8_t)(j + 3);
            item->weight = (uint16_t)(70 + j);
            item->tohit = (int16_t)(-j);
            item->todam = (int16_t)(j + 1);
            item->ac = (int16_t)(j + 2);
            item->toac = (int16_t)(-j - 3);
            item->damage[0] = (uint8_t)(2 + j);
            item->damage[1] = (uint8_t)(5 + j);
            item->level = (uint8_t)(j + 4);
            item->ident = (uint8_t)(j + 5);
        }
    }
}

static void load_expected_into_module(void)
{
    for (int i = 0; i < MAX_STORES; i++) {
        memcpy(store_at(i), &expected[i], sizeof(store_type));
    }
}

static void clear_module(void)
{
    for (int i = 0; i < MAX_STORES; i++) {
        memset(store_at(i), 0, sizeof(store_type));
    }
}

/* 記録を丸ごと比べる。0 で埋めてから読んでいるので、復元しそこねた項目が
 * 1 つでもあれば（在庫の空き枠の中身までふくめて）ここで落ちる。 */
static int first_store_that_differs(void)
{
    for (int i = 0; i < MAX_STORES; i++) {
        if (memcmp(store_at(i), &expected[i], sizeof(store_type)) != 0) {
            return i;
        }
    }

    return -1;
}

/* 書いた結果のバイト列を取りだす。 */
#define CAPTURE_MAX 8192

static int capture(void (*writer)(void), uint8_t *buf)
{
    begin_recording();
    writer();
    rewind(fileptr);

    int n = (int)fread(buf, 1, CAPTURE_MAX, fileptr);
    end_of_file();

    return n;
}

/* --- テスト ------------------------------------------------------------- */

TEST(the_bytes_written_are_the_bytes_the_original_wrote)
{
    static uint8_t legacy_bytes[CAPTURE_MAX];
    static uint8_t actual_bytes[CAPTURE_MAX];

    /* 既存のセーブファイルが読めるかどうかは、結局ここに尽きる。 */
    build_expected();
    load_expected_into_module();

    int legacy_len = capture(legacy_wr_stores, legacy_bytes);
    int actual_len = capture(wr_all_stores, actual_bytes);

    ASSERT_TRUE(legacy_len > 0 && legacy_len == actual_len &&
                memcmp(legacy_bytes, actual_bytes, (size_t)legacy_len) == 0);
}

TEST(a_store_with_nothing_on_the_shelves_writes_only_its_counters)
{
    static uint8_t bytes[CAPTURE_MAX];

    /* 在庫の無い枠は書かない（在庫数のあとに、ある枠のぶんだけ続く）。
     * 6 軒とも空にすれば 1 軒 12 バイト（4+2+1+1+2+2）ずつになる。
     * この長さが変わると、そのあとに続く時刻の読みかたごとずれる。 */
    memset(expected, 0, sizeof(expected));
    load_expected_into_module();

    ASSERT_EQ_INT(capture(wr_all_stores, bytes), MAX_STORES * 12);
}

TEST(what_the_original_wrote_reads_back_whole)
{
    build_expected();
    load_expected_into_module();

    begin_recording();
    legacy_wr_stores();

    clear_module();
    begin_playback();
    bool ok = rd_all_stores();

    ASSERT_TRUE(ok && first_store_that_differs() < 0);
}

TEST(what_is_written_now_the_original_reads_back_whole)
{
    build_expected();
    load_expected_into_module();

    begin_recording();
    wr_all_stores();

    clear_module();
    begin_playback();
    bool ok = legacy_rd_stores();

    ASSERT_TRUE(ok && first_store_that_differs() < 0);
}

TEST(a_full_shop_survives_the_round_trip)
{
    /* 満杯の 1 軒だけを見る（上の 2 つに埋もれないように）。在庫 24 枠は
     * 記録の大半を占めるので、枠の数えかたを 1 つ間違えるとここで落ちる。 */
    build_expected();
    load_expected_into_module();

    begin_recording();
    wr_all_stores();

    clear_module();
    begin_playback();
    (void)rd_all_stores();

    store_type *last = store_at(MAX_STORES - 1);
    ASSERT_TRUE(last->store_ctr == STORE_INVEN_MAX &&
                memcmp(last, &expected[MAX_STORES - 1], sizeof(store_type)) == 0);
}

TEST(an_impossible_shelf_count_stops_the_load)
{
    /* 在庫数は「このあと何枠ぶん読むか」を決める値なので、収まらない数を
     * 信じると記録の外まで読みこむ。変更前は goto error で読みこみ全体を
     * 投げだしていた。 */
    begin_recording();
    wr_long(0);                          /* store_open */
    wr_short(0);                         /* insult_cur */
    wr_byte(0);                          /* owner */
    wr_byte(STORE_INVEN_MAX + 1);        /* store_ctr: 収まらない */
    wr_short(0);                         /* good_buy */
    wr_short(0);                         /* bad_buy */

    clear_module();
    begin_playback();

    ASSERT_FALSE(rd_store(store_at(0)));
}

TEST(the_largest_possible_shelf_count_is_still_accepted)
{
    /* 境界の向き。満杯（24 枠）は正しい値なので断ってはいけない。 */
    build_expected();
    load_expected_into_module();

    begin_recording();
    wr_store(store_at(MAX_STORES - 1));

    clear_module();
    begin_playback();

    ASSERT_TRUE(rd_store(store_at(0)));
}

int main(void)
{
    RUN_TEST(the_bytes_written_are_the_bytes_the_original_wrote);
    RUN_TEST(a_store_with_nothing_on_the_shelves_writes_only_its_counters);
    RUN_TEST(what_the_original_wrote_reads_back_whole);
    RUN_TEST(what_is_written_now_the_original_reads_back_whole);
    RUN_TEST(a_full_shop_survives_the_round_trip);
    RUN_TEST(an_impossible_shelf_count_stops_the_load);
    RUN_TEST(the_largest_possible_shelf_count_is_still_accepted);
    return TEST_SUMMARY();
}
