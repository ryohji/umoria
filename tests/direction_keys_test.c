// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// direction_command_key() が original_commands() と do_command() の switch と
// 一致することを確かめる。逆引きの command_key_direction() は do_command() の
// 方向ごとの case と一致することを確かめる。

#include "config.h"
#include "constant.h"
#include "types.h"

#include "ui_input.h"

#include "minunit.h"

// 書きかえる前の original_commands() の '.'（走る）の switch の写し。
static char legacy_run(int dir_val) {
    char com_val;
    switch (dir_val) {
    case 1:
        com_val = 'B';
        break;
    case 2:
        com_val = 'J';
        break;
    case 3:
        com_val = 'N';
        break;
    case 4:
        com_val = 'H';
        break;
    case 6:
        com_val = 'L';
        break;
    case 7:
        com_val = 'Y';
        break;
    case 8:
        com_val = 'K';
        break;
    case 9:
        com_val = 'U';
        break;
    default:
        com_val = ' ';
        break;
    }
    return com_val;
}

// 書きかえる前の original_commands() の 'T'（掘る）の switch の写し。
static char legacy_tunnel(int dir_val) {
    char com_val;
    switch (dir_val) {
    case 1:
        com_val = CTRL_KEY('B');
        break;
    case 2:
        com_val = CTRL_KEY('J');
        break;
    case 3:
        com_val = CTRL_KEY('N');
        break;
    case 4:
        com_val = CTRL_KEY('H');
        break;
    case 6:
        com_val = CTRL_KEY('L');
        break;
    case 7:
        com_val = CTRL_KEY('Y');
        break;
    case 8:
        com_val = CTRL_KEY('K');
        break;
    case 9:
        com_val = CTRL_KEY('U');
        break;
    default:
        com_val = ' ';
        break;
    }
    return com_val;
}

// 書きかえる前の do_command() の '-'（拾わずに歩く）の switch の写し。
static char legacy_walk(int dir_val) {
    char com_val;
    switch (dir_val) {
    case 1:
        com_val = 'b';
        break;
    case 2:
        com_val = 'j';
        break;
    case 3:
        com_val = 'n';
        break;
    case 4:
        com_val = 'h';
        break;
    case 6:
        com_val = 'l';
        break;
    case 7:
        com_val = 'y';
        break;
    case 8:
        com_val = 'k';
        break;
    case 9:
        com_val = 'u';
        break;
    default:
        com_val = '~';
        break;
    }
    return com_val;
}

TEST(run_keys_match_legacy) {
    int mismatch = 0;
    for (int dir = -20; dir <= 30; dir++) {
        char expected = legacy_run(dir);
        char actual = direction_command_key(dir, DIR_KEY_RUN);
        // app/dungeon.c では default の ' ' を返すが、新しい関数は 0 を返す。
        // 呼び手が ' ' や '~' を足す。
        if (expected == ' ') {
            expected = 0;
        }
        if (actual != expected) {
            mismatch++;
        }
    }
    ASSERT_EQ_INT(mismatch, 0);
}

TEST(tunnel_keys_match_legacy) {
    int mismatch = 0;
    for (int dir = -20; dir <= 30; dir++) {
        char expected = legacy_tunnel(dir);
        char actual = direction_command_key(dir, DIR_KEY_TUNNEL);
        if (expected == ' ') {
            expected = 0;
        }
        if (actual != expected) {
            mismatch++;
        }
    }
    ASSERT_EQ_INT(mismatch, 0);
}

TEST(walk_keys_match_legacy) {
    int mismatch = 0;
    for (int dir = -20; dir <= 30; dir++) {
        char expected = legacy_walk(dir);
        char actual = direction_command_key(dir, DIR_KEY_WALK);
        if (expected == '~') {
            expected = 0;
        }
        if (actual != expected) {
            mismatch++;
        }
    }
    ASSERT_EQ_INT(mismatch, 0);
}

// 書きかえる前の do_command() の、方向ごとの 24 の case の写し（^M は ^J と同じ
// case に並んでいるが、表には無いので do_command() に残す。下の別のテスト）。
// 歩く・走る・掘るを 'w'・'r'・'t' で、方向を *dir で返す。それ以外のキーは 0。
static char legacy_do_command_direction(char key, int *dir) {
    switch (key) {
    case 'b': *dir = 1; return 'w';
    case 'j': *dir = 2; return 'w';
    case 'n': *dir = 3; return 'w';
    case 'h': *dir = 4; return 'w';
    case 'l': *dir = 6; return 'w';
    case 'y': *dir = 7; return 'w';
    case 'k': *dir = 8; return 'w';
    case 'u': *dir = 9; return 'w';
    case 'B': *dir = 1; return 'r';
    case 'J': *dir = 2; return 'r';
    case 'N': *dir = 3; return 'r';
    case 'H': *dir = 4; return 'r';
    case 'L': *dir = 6; return 'r';
    case 'Y': *dir = 7; return 'r';
    case 'K': *dir = 8; return 'r';
    case 'U': *dir = 9; return 'r';
    case CTRL_KEY('B'): *dir = 1; return 't';
    case CTRL_KEY('J'): *dir = 2; return 't';
    case CTRL_KEY('N'): *dir = 3; return 't';
    case CTRL_KEY('H'): *dir = 4; return 't';
    case CTRL_KEY('L'): *dir = 6; return 't';
    case CTRL_KEY('Y'): *dir = 7; return 't';
    case CTRL_KEY('K'): *dir = 8; return 't';
    case CTRL_KEY('U'): *dir = 9; return 't';
    default: return 0;
    }
}

// command_key_direction() が、256 通りのキーのすべてで do_command() の 24 の case と
// 同じ向きと種類を返す。
TEST(directions_of_keys_match_the_do_command_cases) {
    int mismatch = 0;
    for (int k = 0; k < 256; k++) {
        char key = (char)k;
        int legacy_dir = 0;
        char legacy = legacy_do_command_direction(key, &legacy_dir);

        direction_key_style style = DIR_KEY_WALK;
        int dir = command_key_direction(key, &style);
        char actual = 0;
        if (dir != 0) {
            actual = style == DIR_KEY_WALK ? 'w' : style == DIR_KEY_RUN ? 'r' : 't';
        }
        if (actual != legacy || (legacy != 0 && dir != legacy_dir)) {
            mismatch++;
        }
    }
    ASSERT_EQ_INT(mismatch, 0);
}

// ^M（復帰）は表に無い。do_command() は ^J と同じく下へ掘る case を残す。
TEST(return_is_not_a_direction_key) {
    direction_key_style style = DIR_KEY_WALK;
    ASSERT_EQ_INT(command_key_direction(CTRL_KEY('M'), &style), 0);
}

int main(void) {
    RUN_TEST(run_keys_match_legacy);
    RUN_TEST(tunnel_keys_match_legacy);
    RUN_TEST(walk_keys_match_legacy);
    RUN_TEST(directions_of_keys_match_the_do_command_cases);
    RUN_TEST(return_is_not_a_direction_key);
    TEST_SUMMARY();
}
