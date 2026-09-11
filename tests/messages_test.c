/* メッセージ履歴（リングバッファ）のテスト -- 現在のふるまいを保護する
 *
 * old_msg[MAX_SAVE_MSG] と last_msg は「上端の行に出したメッセージ」の
 * 履歴で、輪をなしている。輪の回りかたを 3 箇所が個別に知っていた。
 *   io.c:191      次の枠へ進める（last_msg + 1 == MAX_SAVE_MSG ? 0 : +1）
 *   dungeon.c:1115-1128  ^P で古いほうへ辿る（j == 0 ? MAX_SAVE_MSG - 1 : j - 1）
 *   save.c:212-214, 682-684  枠をそのまま並べて書く（セーブファイルの形式）
 * src/messages.c に寄せる。
 *
 * 保護のとりかた。「進める」も「辿る」も本体の関数の中に直に書かれていて
 * 外から呼べないので、#7 / #18-1 と同じ「テスト側に写す」方式をとる。
 * 変更前の dungeon.c の辿りかたを legacy_walk_back() に写しとり、
 * 新しい実体（msg_history_recent）と突きあわせる。押しこむ回数を
 * 0 回から 3 周ぶんまで変えて、輪の継ぎ目（枠 0 と枠 21 の境）を必ず
 * またぐようにしてある。
 *
 * セーブファイルの形式（枠を storage の順に MAX_SAVE_MSG 個 + 現在位置の
 * 添字）も押さえる。ここが動くと既存のセーブファイルが読めなくなる。
 */
/* externs.h を include していない。輪が messages.c の static になったので、
 * ふれる先は messages.h の窓口だけで足りる。 */
#include "config.h"
#include "constant.h"
#include "types.h"

#include "messages.h"

#include "minunit.h"

#include <stdio.h>

/* --- 変更前の dungeon.c:1115-1128 の写し -------------------------------
 * j = last_msg から始めて、1 つずつ古いほうへ。0 の次は MAX_SAVE_MSG - 1。
 * old_msg / last_msg は messages.c の static になったので、写しのほうは
 * セーブファイル用の窓口（枠そのものを見る側）から輪にふれる。辿りかた
 * ——保護したいところ——は変更前のままの形。 */
static const char *legacy_walk_back(int back)
{
    int j = msg_history_newest_slot();

    for (int step = 0; step < back; step++) {
        if (j == 0) {
            j = MAX_SAVE_MSG - 1;
        } else {
            j--;
        }
    }
    return msg_history_slot(j);
}

/* 履歴を空にしてから n 個入れる。中身は "msg 0", "msg 1", ... で、
 * どの枠に何が入ったかが読めばわかるようにする。 */
static void fill(int n)
{
    for (int slot = 0; slot < msg_history_slot_count(); slot++) {
        msg_history_slot(slot)[0] = '\0';
    }
    msg_history_set_newest_slot(0);

    for (int i = 0; i < n; i++) {
        char message[VTYPESIZ];

        (void)snprintf(message, sizeof message, "msg %d", i);
        msg_history_push(message);
    }
}

/* --- テスト ------------------------------------------------------------- */

TEST(the_ring_holds_twenty_two_messages)
{
    /* MAX_SAVE_MSG（constant.h:38）はセーブファイルの形式でもある。 */
    ASSERT_EQ_INT(msg_history_slot_count(), MAX_SAVE_MSG);
}

TEST(the_newest_message_is_the_one_just_pushed)
{
    fill(3);
    ASSERT_EQ_STR(msg_history_recent(0), "msg 2");
}

TEST(walking_back_one_gives_the_message_before_it)
{
    fill(3);
    ASSERT_EQ_STR(msg_history_recent(1), "msg 1");
}

TEST(walking_back_crosses_the_seam_of_the_ring)
{
    /* 枠を 1 周と 3 個ぶん埋めると、新しい 3 個は枠 1..3 に、その前は
     * 枠 0（= MAX_SAVE_MSG 番目に押しこんだもの）にある。 */
    fill(MAX_SAVE_MSG + 3);
    ASSERT_EQ_STR(msg_history_recent(3), "msg 21");
}

TEST(walking_back_matches_the_original_for_every_number_of_messages)
{
    int mismatch = -1;

    /* 0 個から 3 周ぶんまで。輪の継ぎ目を必ずまたぐ。 */
    for (int pushed = 0; pushed <= 3 * MAX_SAVE_MSG; pushed++) {
        fill(pushed);

        for (int back = 0; back < MAX_SAVE_MSG; back++) {
            if (strcmp(msg_history_recent(back), legacy_walk_back(back)) != 0) {
                mismatch = pushed;
                break;
            }
        }
        if (mismatch >= 0) {
            break;
        }
    }
    ASSERT_EQ_INT(mismatch, -1);
}

TEST(walking_back_a_whole_lap_comes_around_to_the_newest)
{
    /* dungeon.c は ^P の回数を MAX_SAVE_MSG で止める。止めるのは呼ぶ側の
     * 責任だが、1 周を超えても輪の外を読まないことは確かめておく。 */
    fill(MAX_SAVE_MSG);
    ASSERT_EQ_STR(msg_history_recent(MAX_SAVE_MSG), msg_history_recent(0));
}

/* ^P の表示は「どの行に、何個前のメッセージを出すか」の対応そのもので、
 * 変更前は行番号と枠の添字を同時に減らしていた（i-- と j--）。書きかえで
 * 添字の向きが逆になった（recent(x - 1 - i)）ので、対応が変わっていない
 * ことを確かめる。dungeon.c の case 文は外から呼べないので、両方の並べかたを
 * ここに写して突きあわせる。 */
TEST(the_previous_message_block_puts_the_oldest_at_the_top)
{
    int mismatch = -1;

    for (int pushed = 0; pushed <= 2 * MAX_SAVE_MSG; pushed++) {
        fill(pushed);

        /* x は ^P が出す行数。dungeon.c は 1 以上 MAX_SAVE_MSG 以下に丸める。 */
        for (int x = 2; x <= MAX_SAVE_MSG; x++) {
            int legacy_slot = msg_history_newest_slot();

            /* 変更後: 行 i に recent(x - 1 - i)。行は x-1 から 0 へ下る。 */
            for (int i = x - 1; i >= 0; i--) {
                if (strcmp(msg_history_recent(x - 1 - i), msg_history_slot(legacy_slot)) != 0) {
                    mismatch = x;
                    break;
                }
                /* 変更前: 行を 1 つ上げるたびに枠を 1 つ古いほうへ。 */
                legacy_slot = legacy_slot == 0 ? MAX_SAVE_MSG - 1 : legacy_slot - 1;
            }
            if (mismatch >= 0) {
                break;
            }
        }
        if (mismatch >= 0) {
            break;
        }
    }
    ASSERT_EQ_INT(mismatch, -1);
}

TEST(pushing_advances_the_slot_by_one)
{
    fill(0);
    msg_history_push("first");
    ASSERT_EQ_INT(msg_history_newest_slot(), 1);
}

TEST(pushing_wraps_the_slot_back_to_zero)
{
    /* io.c:191 の「+1 が MAX_SAVE_MSG になったら 0」。枠 0 から数えて
     * MAX_SAVE_MSG 回押しこむと、ちょうど枠 0 に戻る。 */
    fill(MAX_SAVE_MSG);
    ASSERT_EQ_INT(msg_history_newest_slot(), 0);
}

TEST(a_long_message_is_truncated_to_the_slot_size)
{
    char long_message[VTYPESIZ * 2];

    memset(long_message, 'x', sizeof long_message - 1);
    long_message[sizeof long_message - 1] = '\0';

    fill(0);
    msg_history_push(long_message);
    ASSERT_EQ_INT(strlen(msg_history_recent(0)), VTYPESIZ - 1);
}

TEST(appending_joins_the_two_messages_with_two_spaces)
{
    /* io.c は短い 2 つを 1 行にまとめて見せる。履歴側もまとめて 1 件に
     * なる（2 件にはならない）。 */
    fill(0);
    msg_history_push("You feel sick.");
    msg_history_append("You have 3 rations.");
    ASSERT_EQ_STR(msg_history_recent(0), "You feel sick.  You have 3 rations.");
}

TEST(appending_does_not_add_a_message)
{
    fill(0);
    msg_history_push("You feel sick.");
    msg_history_append("You have 3 rations.");
    ASSERT_EQ_INT(msg_history_newest_slot(), 1);
}

TEST(appending_stays_inside_the_slot)
{
    char nearly_full[VTYPESIZ];

    memset(nearly_full, 'x', sizeof nearly_full - 1);
    nearly_full[sizeof nearly_full - 1] = '\0';

    fill(0);
    msg_history_push(nearly_full);
    msg_history_append("more"); /* 呼ぶ側は入る分しか渡さないが、越えても書かない */
    ASSERT_EQ_INT(strlen(msg_history_recent(0)), VTYPESIZ - 1);
}

TEST(the_storage_view_is_what_the_save_file_writes)
{
    /* save.c は枠 0..MAX_SAVE_MSG-1 を storage の順に書き、別に現在位置の
     * 添字を書く。順序も添字もこの見かたのままでなければならない。 */
    fill(MAX_SAVE_MSG + 3);
    ASSERT_EQ_STR(msg_history_slot(1), "msg 22");
}

TEST(the_newest_slot_survives_a_round_trip)
{
    fill(MAX_SAVE_MSG + 3);
    int saved = msg_history_newest_slot();

    fill(0);
    msg_history_set_newest_slot(saved);
    ASSERT_EQ_INT(msg_history_newest_slot(), saved);
}

int main(void)
{
    RUN_TEST(the_ring_holds_twenty_two_messages);
    RUN_TEST(the_newest_message_is_the_one_just_pushed);
    RUN_TEST(walking_back_one_gives_the_message_before_it);
    RUN_TEST(walking_back_crosses_the_seam_of_the_ring);
    RUN_TEST(walking_back_matches_the_original_for_every_number_of_messages);
    RUN_TEST(walking_back_a_whole_lap_comes_around_to_the_newest);
    RUN_TEST(the_previous_message_block_puts_the_oldest_at_the_top);
    RUN_TEST(pushing_advances_the_slot_by_one);
    RUN_TEST(pushing_wraps_the_slot_back_to_zero);
    RUN_TEST(a_long_message_is_truncated_to_the_slot_size);
    RUN_TEST(appending_joins_the_two_messages_with_two_spaces);
    RUN_TEST(appending_does_not_add_a_message);
    RUN_TEST(appending_stays_inside_the_slot);
    RUN_TEST(the_storage_view_is_what_the_save_file_writes);
    RUN_TEST(the_newest_slot_survives_a_round_trip);
    return TEST_SUMMARY();
}
