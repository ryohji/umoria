// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 着ている防具を並べて 1 つ選ぶ（呪われた品を優先する）関数のテスト。
 * 着用の全 2^6 = 64 通り × 呪われかた × randint の答え 1..k の総当たりで、
 * 変更前の scrolls.c case 3 の字面を写した関数と突きあわせる。 */

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

#include "armor_selection.h"
#include "equipment.h"
#include "fixture.h"
#include "shared_stubs.h"

#define MU_SETUP() shared_stubs_reset()

#include "minunit.h"

/* scrolls.c case 3 の選択部分（:104-143）をそのまま写した legacy 関数。
 * 変更前の実装を凍結したもので、理想の書きかたではない。 */
static int legacy_random_and_cursed_armor(void) {
    int tmp[6];
    int k = 0;
    int l = 0;

    if (equipment_at(INVEN_BODY)->tval != TV_NOTHING) {
        tmp[k++] = INVEN_BODY;
    }
    if (equipment_at(INVEN_ARM)->tval != TV_NOTHING) {
        tmp[k++] = INVEN_ARM;
    }
    if (equipment_at(INVEN_OUTER)->tval != TV_NOTHING) {
        tmp[k++] = INVEN_OUTER;
    }
    if (equipment_at(INVEN_HANDS)->tval != TV_NOTHING) {
        tmp[k++] = INVEN_HANDS;
    }
    if (equipment_at(INVEN_HEAD)->tval != TV_NOTHING) {
        tmp[k++] = INVEN_HEAD;
    }
    if (equipment_at(INVEN_FEET)->tval != TV_NOTHING) {
        tmp[k++] = INVEN_FEET;
    }

    if (k > 0) {
        l = tmp[randint(k) - 1];
    }

    if (TR_CURSED & equipment_at(INVEN_BODY)->flags) {
        l = INVEN_BODY;
    } else if (TR_CURSED & equipment_at(INVEN_ARM)->flags) {
        l = INVEN_ARM;
    } else if (TR_CURSED & equipment_at(INVEN_OUTER)->flags) {
        l = INVEN_OUTER;
    } else if (TR_CURSED & equipment_at(INVEN_HEAD)->flags) {
        l = INVEN_HEAD;
    } else if (TR_CURSED & equipment_at(INVEN_HANDS)->flags) {
        l = INVEN_HANDS;
    } else if (TR_CURSED & equipment_at(INVEN_FEET)->flags) {
        l = INVEN_FEET;
    }

    return l;
}

/* 装備スロット 6 つの着用/非着用を指定して、各 TV_HARD_ARMOR で埋める。 */
static void set_armor_worn(bool body, bool arm, bool outer, bool hands, bool head, bool feet) {
    equipment_at(INVEN_BODY)->tval = body ? TV_HARD_ARMOR : TV_NOTHING;
    equipment_at(INVEN_ARM)->tval = arm ? TV_HARD_ARMOR : TV_NOTHING;
    equipment_at(INVEN_OUTER)->tval = outer ? TV_HARD_ARMOR : TV_NOTHING;
    equipment_at(INVEN_HANDS)->tval = hands ? TV_HARD_ARMOR : TV_NOTHING;
    equipment_at(INVEN_HEAD)->tval = head ? TV_HARD_ARMOR : TV_NOTHING;
    equipment_at(INVEN_FEET)->tval = feet ? TV_HARD_ARMOR : TV_NOTHING;
}

/* 6 つのスロットの呪われフラグを指定して設定する。 */
static void set_armor_cursed(bool body, bool arm, bool outer, bool hands, bool head, bool feet) {
    equipment_at(INVEN_BODY)->flags = body ? TR_CURSED : 0;
    equipment_at(INVEN_ARM)->flags = arm ? TR_CURSED : 0;
    equipment_at(INVEN_OUTER)->flags = outer ? TR_CURSED : 0;
    equipment_at(INVEN_HANDS)->flags = hands ? TR_CURSED : 0;
    equipment_at(INVEN_HEAD)->flags = head ? TR_CURSED : 0;
    equipment_at(INVEN_FEET)->flags = feet ? TR_CURSED : 0;
}

TEST(nothing_worn_gives_zero) {
    set_armor_worn(false, false, false, false, false, false);

    int result = pick_random_worn_armor();

    ASSERT_EQ_INT(result, 0);
    ASSERT_EQ_INT(fixture_randint_call_count(), 0);
}

TEST(one_worn_returns_that_slot_calling_randint_once) {
    /* 1 つだけ着用の 6 通りを、randint を 1 回だけ呼んで返すことを見る。
     * randint(1) の答えは必ず 1 なので、結果は常にその 1 つのスロット。 */
    const int slots[6] = {INVEN_BODY, INVEN_ARM, INVEN_OUTER, INVEN_HANDS, INVEN_HEAD, INVEN_FEET};
    int mismatch = 0;

    for (int i = 0; i < 6; i++) {
        set_armor_worn(i == 0, i == 1, i == 2, i == 3, i == 4, i == 5);
        shared_stubs_reset();
        fixture_set_randint(1);

        int result = pick_random_worn_armor();
        if (result != slots[i] || fixture_randint_call_count() != 1 ||
            fixture_randint_last_maxval() != 1) {
            mismatch++;
        }
    }

    ASSERT_EQ_INT(mismatch, 0);
}

TEST(all_64_worn_patterns_match_legacy_for_every_randint_answer) {
    /* 着用の全 64 通り × 各通りで randint の答え 1..k を突きあわせる。
     * 呪われは無い（cursed 側のテストで見る）。 */
    int mismatch = 0;

    for (int pattern = 0; pattern < 64; pattern++) {
        bool body = (pattern & 1) != 0;
        bool arm = (pattern & 2) != 0;
        bool outer = (pattern & 4) != 0;
        bool hands = (pattern & 8) != 0;
        bool head = (pattern & 16) != 0;
        bool feet = (pattern & 32) != 0;

        set_armor_worn(body, arm, outer, hands, head, feet);
        set_armor_cursed(false, false, false, false, false, false);

        /* この着用パターンで着ている個数を数える。 */
        int count = (body ? 1 : 0) + (arm ? 1 : 0) + (outer ? 1 : 0) +
                    (hands ? 1 : 0) + (head ? 1 : 0) + (feet ? 1 : 0);

        if (count == 0) {
            /* 0 個なら randint を呼ばず、両方 0 を返す。 */
            shared_stubs_reset();
            int result = pick_random_worn_armor();
            int calls = fixture_randint_call_count();

            shared_stubs_reset();
            int legacy = legacy_random_and_cursed_armor();
            int legacy_calls = fixture_randint_call_count();

            if (result != 0 || legacy != 0 || calls != 0 || legacy_calls != 0) {
                mismatch++;
            }
        } else {
            /* count > 0 なら、randint の答え 1..count のすべてを試す。 */
            for (int answer = 1; answer <= count; answer++) {
                shared_stubs_reset();
                fixture_set_randint(answer);

                int result = pick_random_worn_armor();
                int calls = fixture_randint_call_count();
                int maxval = fixture_randint_last_maxval();

                shared_stubs_reset();
                fixture_set_randint(answer);
                int legacy = legacy_random_and_cursed_armor();
                int legacy_calls = fixture_randint_call_count();
                int legacy_maxval = fixture_randint_last_maxval();

                if (result != legacy || calls != 1 || legacy_calls != 1 ||
                    maxval != count || legacy_maxval != count) {
                    mismatch++;
                }
            }
        }
    }

    ASSERT_EQ_INT(mismatch, 0);
}

TEST(cursed_override_matches_legacy) {
    /* 呪われた防具を優先する部分を見る。全 64 通りの着用パターンで、
     * 呪われパターンをいくつか試す。 */
    int mismatch = 0;

    for (int worn_pattern = 0; worn_pattern < 64; worn_pattern++) {
        bool body = (worn_pattern & 1) != 0;
        bool arm = (worn_pattern & 2) != 0;
        bool outer = (worn_pattern & 4) != 0;
        bool hands = (worn_pattern & 8) != 0;
        bool head = (worn_pattern & 16) != 0;
        bool feet = (worn_pattern & 32) != 0;

        set_armor_worn(body, arm, outer, hands, head, feet);

        /* 各着用パターンで、呪われの組みあわせを試す。
         * 全 2^6 は多いので、0（全部呪われていない）と、
         * 着ているものの最初の 1 つだけ呪われ、すべて呪われ、を試す。 */

        /* (1) 全部呪われていない。 */
        set_armor_cursed(false, false, false, false, false, false);

        int count = (body ? 1 : 0) + (arm ? 1 : 0) + (outer ? 1 : 0) +
                    (hands ? 1 : 0) + (head ? 1 : 0) + (feet ? 1 : 0);

        if (count > 0) {
            shared_stubs_reset();
            fixture_set_randint(1);
            int result_random = pick_random_worn_armor();
            int result_cursed = pick_first_cursed_armor();

            shared_stubs_reset();
            fixture_set_randint(1);
            int legacy = legacy_random_and_cursed_armor();

            /* legacy は cursed override を含むので、cursed が 0 なら
             * random の答えを使う。 */
            int expected = (result_cursed != 0) ? result_cursed : result_random;

            if (expected != legacy) {
                mismatch++;
            }
        }

        /* (2) 着ているものの最初の 1 つだけ呪われ。 */
        if (count > 0) {
            /* BODY, ARM, OUTER, HANDS, HEAD, FEET の順で最初の着用を呪う。 */
            bool cursed_body = body;
            bool cursed_arm = !body && arm;
            bool cursed_outer = !body && !arm && outer;
            bool cursed_hands = !body && !arm && !outer && hands;
            bool cursed_head = !body && !arm && !outer && !hands && head;
            bool cursed_feet = !body && !arm && !outer && !hands && !head && feet;

            set_armor_cursed(cursed_body, cursed_arm, cursed_outer,
                           cursed_hands, cursed_head, cursed_feet);

            shared_stubs_reset();
            fixture_set_randint(count);  /* 乱数では最後を選ぶ */
            int result_random = pick_random_worn_armor();
            int result_cursed = pick_first_cursed_armor();

            shared_stubs_reset();
            fixture_set_randint(count);
            int legacy = legacy_random_and_cursed_armor();

            int expected = (result_cursed != 0) ? result_cursed : result_random;

            if (expected != legacy) {
                mismatch++;
            }
        }

        /* (3) 全部呪われ。 */
        if (count > 0) {
            set_armor_cursed(body, arm, outer, hands, head, feet);

            shared_stubs_reset();
            fixture_set_randint(count);
            int result_random = pick_random_worn_armor();
            int result_cursed = pick_first_cursed_armor();

            shared_stubs_reset();
            fixture_set_randint(count);
            int legacy = legacy_random_and_cursed_armor();

            int expected = (result_cursed != 0) ? result_cursed : result_random;

            if (expected != legacy) {
                mismatch++;
            }
        }
    }

    ASSERT_EQ_INT(mismatch, 0);
}

TEST(cursed_priority_order_is_body_arm_outer_head_hands_feet) {
    /* 優先順が BODY, ARM, OUTER, HEAD, HANDS, FEET であることを見る。
     * 全部着用で、各優先位置だけ呪われの 6 通りを試す。 */
    const int slots[6] = {INVEN_BODY, INVEN_ARM, INVEN_OUTER, INVEN_HEAD, INVEN_HANDS, INVEN_FEET};
    int mismatch = 0;

    set_armor_worn(true, true, true, true, true, true);

    /* set_armor_cursed の引数順は (body, arm, outer, hands, head, feet) だが、
     * 優先順は (BODY, ARM, OUTER, HEAD, HANDS, FEET) なので、
     * i=3 では head を、i=4 では hands を呪う。 */
    for (int i = 0; i < 6; i++) {
        bool cursed_body = (i == 0);
        bool cursed_arm = (i == 1);
        bool cursed_outer = (i == 2);
        bool cursed_head = (i == 3);
        bool cursed_hands = (i == 4);
        bool cursed_feet = (i == 5);
        set_armor_cursed(cursed_body, cursed_arm, cursed_outer, cursed_hands, cursed_head, cursed_feet);

        int result = pick_first_cursed_armor();
        if (result != slots[i]) {
            mismatch++;
        }
    }

    ASSERT_EQ_INT(mismatch, 0);
}

TEST(no_cursed_armor_gives_zero) {
    set_armor_worn(true, true, true, true, true, true);
    set_armor_cursed(false, false, false, false, false, false);

    ASSERT_EQ_INT(pick_first_cursed_armor(), 0);
}

int main(void) {
    RUN_TEST(nothing_worn_gives_zero);
    RUN_TEST(one_worn_returns_that_slot_calling_randint_once);
    RUN_TEST(all_64_worn_patterns_match_legacy_for_every_randint_answer);
    RUN_TEST(cursed_override_matches_legacy);
    RUN_TEST(cursed_priority_order_is_body_arm_outer_head_hands_feet);
    RUN_TEST(no_cursed_armor_gives_zero);

    TEST_SUMMARY();
}
