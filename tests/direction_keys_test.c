// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// direction_command_key() が original_commands() と do_command() の switch と
// 一致することを確かめる。

#include "config.h"
#include "constant.h"
#include "types.h"

#include "direction_keys.h"

#include "minunit.h"

// Legacy switches copied verbatim from dungeon.c (original_commands, case '.').
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

// Legacy switches copied verbatim from dungeon.c (original_commands, case 'T').
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

// Legacy switches copied verbatim from dungeon.c (do_command, case '-').
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
        // dungeon.c では default の ' ' を返すが、新しい関数は 0 を返す。
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

int main(void) {
    RUN_TEST(run_keys_match_legacy);
    RUN_TEST(tunnel_keys_match_legacy);
    RUN_TEST(walk_keys_match_legacy);
    TEST_SUMMARY();
}
