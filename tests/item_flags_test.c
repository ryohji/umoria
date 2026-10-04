// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* item_flags_test.c -- ident バイトの窓口
 *
 * item_flags.c の各窓口（setter 3 本・predicate 7 本）が、生のビット操作と
 * 一致することを見る。ident の値 0〜255 すべてで、窓口の結果と生の式の
 * 結果を比べる。不一致があれば件数を溜めて最後に assert する。
 */

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "item_flags.h"

#include "minunit.h"

/* 表示の指示を立てる窓口 */
TEST(item_show_hit_dam_sets_ID_SHOW_HITDAM) {
    int mismatch = 0;
    for (int val = 0; val < 256; val++) {
        inven_type i = {0};
        i.ident = val;
        item_show_hit_dam(&i);
        uint8_t expected = val | ID_SHOW_HITDAM;
        if (i.ident != expected) {
            mismatch++;
        }
    }
    ASSERT_EQ_INT(mismatch, 0);
}

TEST(item_show_p1_sets_ID_SHOW_P1) {
    int mismatch = 0;
    for (int val = 0; val < 256; val++) {
        inven_type i = {0};
        i.ident = val;
        item_show_p1(&i);
        uint8_t expected = val | ID_SHOW_P1;
        if (i.ident != expected) {
            mismatch++;
        }
    }
    ASSERT_EQ_INT(mismatch, 0);
}

TEST(item_hide_p1_sets_ID_NOSHOW_P1) {
    int mismatch = 0;
    for (int val = 0; val < 256; val++) {
        inven_type i = {0};
        i.ident = val;
        item_hide_p1(&i);
        uint8_t expected = val | ID_NOSHOW_P1;
        if (i.ident != expected) {
            mismatch++;
        }
    }
    ASSERT_EQ_INT(mismatch, 0);
}

/* 表示の指示を読む窓口 */
TEST(item_shows_hit_dam_reads_ID_SHOW_HITDAM) {
    int mismatch = 0;
    for (int val = 0; val < 256; val++) {
        inven_type i = {0};
        i.ident = val;
        bool actual = item_shows_hit_dam(&i);
        bool expected = (val & ID_SHOW_HITDAM) != 0;
        if (actual != expected) {
            mismatch++;
        }
    }
    ASSERT_EQ_INT(mismatch, 0);
}

TEST(item_shows_p1_reads_ID_SHOW_P1) {
    int mismatch = 0;
    for (int val = 0; val < 256; val++) {
        inven_type i = {0};
        i.ident = val;
        bool actual = item_shows_p1(&i);
        bool expected = (val & ID_SHOW_P1) != 0;
        if (actual != expected) {
            mismatch++;
        }
    }
    ASSERT_EQ_INT(mismatch, 0);
}

TEST(item_hides_p1_reads_ID_NOSHOW_P1) {
    int mismatch = 0;
    for (int val = 0; val < 256; val++) {
        inven_type i = {0};
        i.ident = val;
        bool actual = item_hides_p1(&i);
        bool expected = (val & ID_NOSHOW_P1) != 0;
        if (actual != expected) {
            mismatch++;
        }
    }
    ASSERT_EQ_INT(mismatch, 0);
}

/* 注記を読む窓口 */
TEST(item_noted_magical_reads_ID_MAGIK) {
    int mismatch = 0;
    for (int val = 0; val < 256; val++) {
        inven_type i = {0};
        i.ident = val;
        bool actual = item_noted_magical(&i);
        bool expected = (val & ID_MAGIK) != 0;
        if (actual != expected) {
            mismatch++;
        }
    }
    ASSERT_EQ_INT(mismatch, 0);
}

TEST(item_noted_empty_reads_ID_EMPTY) {
    int mismatch = 0;
    for (int val = 0; val < 256; val++) {
        inven_type i = {0};
        i.ident = val;
        bool actual = item_noted_empty(&i);
        bool expected = (val & ID_EMPTY) != 0;
        if (actual != expected) {
            mismatch++;
        }
    }
    ASSERT_EQ_INT(mismatch, 0);
}

TEST(item_noted_damned_reads_ID_DAMD) {
    int mismatch = 0;
    for (int val = 0; val < 256; val++) {
        inven_type i = {0};
        i.ident = val;
        bool actual = item_noted_damned(&i);
        bool expected = (val & ID_DAMD) != 0;
        if (actual != expected) {
            mismatch++;
        }
    }
    ASSERT_EQ_INT(mismatch, 0);
}

TEST(item_has_any_note_reads_all_three_note_bits) {
    int mismatch = 0;
    for (int val = 0; val < 256; val++) {
        inven_type i = {0};
        i.ident = val;
        bool actual = item_has_any_note(&i);
        bool expected = (val & (ID_MAGIK | ID_EMPTY | ID_DAMD)) != 0;
        if (actual != expected) {
            mismatch++;
        }
    }
    ASSERT_EQ_INT(mismatch, 0);
}

int main(void) {
    RUN_TEST(item_show_hit_dam_sets_ID_SHOW_HITDAM);
    RUN_TEST(item_show_p1_sets_ID_SHOW_P1);
    RUN_TEST(item_hide_p1_sets_ID_NOSHOW_P1);
    RUN_TEST(item_shows_hit_dam_reads_ID_SHOW_HITDAM);
    RUN_TEST(item_shows_p1_reads_ID_SHOW_P1);
    RUN_TEST(item_hides_p1_reads_ID_NOSHOW_P1);
    RUN_TEST(item_noted_magical_reads_ID_MAGIK);
    RUN_TEST(item_noted_empty_reads_ID_EMPTY);
    RUN_TEST(item_noted_damned_reads_ID_DAMD);
    RUN_TEST(item_has_any_note_reads_all_three_note_bits);
    TEST_SUMMARY();
}
