// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// 原文の鍵の組の表（ui/keymap.c の original_key_meaning()）が、書きかえる前の
// app/dungeon.c の original_commands() の switch と、256 通りのキーのすべてで
// 一致することを確かめる。

#include "constant.h"
#include "ui_input.h"

#include "minunit.h"

// 方向を訊いてから決める 2 つのキー（. と T）の印。char の範囲の外に置く。
#define LEGACY_RUN_PREFIX 256
#define LEGACY_TUNNEL_PREFIX 257

// 書きかえる前の original_commands() の写し。方向を訊く 2 か所（get_dir と
// direction_command_key）だけを、上の印を返す形に替えた。
static int legacy_original_commands(char com_val) {
    switch (com_val) {
    case CTRL_KEY('K'): // ^K = exit
        com_val = 'Q';
        break;
    case CTRL_KEY('J'):
    case CTRL_KEY('M'):
        com_val = '+';
        break;
    case CTRL_KEY('P'): // ^P = repeat
    case CTRL_KEY('W'): // ^W = password
    case CTRL_KEY('X'): // ^X = save
    case CTRL_KEY('V'): // ^V = view license
    case ' ':
    case '!':
    case '$':
        break;
    case '.':
        return LEGACY_RUN_PREFIX;
        break;
    case '/':
    case '<':
    case '>':
    case '-':
    case '=':
    case '{':
    case '?':
    case 'A':
        break;
    case '1':
        com_val = 'b';
        break;
    case '2':
        com_val = 'j';
        break;
    case '3':
        com_val = 'n';
        break;
    case '4':
        com_val = 'h';
        break;
    case '5': // Rest one turn
        com_val = '.';
        break;
    case '6':
        com_val = 'l';
        break;
    case '7':
        com_val = 'y';
        break;
    case '8':
        com_val = 'k';
        break;
    case '9':
        com_val = 'u';
        break;
    case 'B':
        com_val = 'f';
        break;
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'G':
        break;
    case 'L':
        com_val = 'W';
        break;
    case 'M':
        break;
    case 'R':
        break;
    case 'S':
        com_val = '#';
        break;
    case 'T':
        return LEGACY_TUNNEL_PREFIX;
        break;
    case 'V':
        break;
    case 'a':
        com_val = 'z';
        break;
    case 'b':
        com_val = 'P';
        break;
    case 'c':
    case 'd':
    case 'e':
        break;
    case 'f':
        com_val = 't';
        break;
    case 'h':
        com_val = '?';
        break;
    case 'i':
        break;
    case 'j':
        com_val = 'S';
        break;
    case 'l':
        com_val = 'x';
        break;
    case 'm':
    case 'o':
    case 'p':
    case 'q':
    case 'r':
    case 's':
        break;
    case 't':
        com_val = 'T';
        break;
    case 'u':
        com_val = 'Z';
        break;
    case 'v':
    case 'w':
        break;
    case 'x':
        com_val = 'X';
        break;

    // wizard mode commands follow
    case CTRL_KEY('A'): // ^A = cure all
        break;
    case CTRL_KEY('B'): // ^B = objects
        com_val = CTRL_KEY('O');
        break;
    case CTRL_KEY('D'): // ^D = up/down
        break;
    case CTRL_KEY('H'): // ^H = wizhelp
        com_val = '\\';
        break;
    case CTRL_KEY('I'): // ^I = identify
        break;
    case CTRL_KEY('L'): // ^L = wizlight
        com_val = '*';
        break;
    case ':':
    case CTRL_KEY('T'): // ^T = teleport
    case CTRL_KEY('E'): // ^E = wizchar
    case CTRL_KEY('F'): // ^F = genocide
    case CTRL_KEY('G'): // ^G = treasure
    case '@':
    case '+':
        break;
    case CTRL_KEY('U'): // ^U = summon
        com_val = '&';
        break;
    default:
        com_val = '~'; // Anything illegal.
        break;
    }
    return (unsigned char)com_val;
}

TEST(original_keys_match_the_switch) {
    int mismatch = 0;
    for (int k = 0; k < 256; k++) {
        char key = (char)k;
        int legacy = legacy_original_commands(key);

        char command = 0;
        int actual;
        switch (original_key_meaning(key, &command)) {
        case ORIGINAL_KEY_COMMAND:
            actual = (unsigned char)command;
            break;
        case ORIGINAL_KEY_RUN_PREFIX:
            actual = LEGACY_RUN_PREFIX;
            break;
        case ORIGINAL_KEY_TUNNEL_PREFIX:
            actual = LEGACY_TUNNEL_PREFIX;
            break;
        default:
            actual = '~';
            break;
        }
        if (actual != legacy) {
            mismatch++;
        }
    }
    ASSERT_EQ_INT(mismatch, 0);
}

// 不正なキーを '~' にするのは呼び手（app/dungeon.c）。表は「不正」とだけ言う。
// 表の中に '~' そのものは無い。
TEST(tilde_is_not_an_original_key) {
    char command = 0;
    ASSERT_EQ_INT(original_key_meaning('~', &command), ORIGINAL_KEY_ILLEGAL);
}

int main(void) {
    RUN_TEST(original_keys_match_the_switch);
    RUN_TEST(tilde_is_not_an_original_key);
    TEST_SUMMARY();
}
