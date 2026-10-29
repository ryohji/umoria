// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// 原文の鍵の組の表（ui/keymap.c の original_key_meaning()）が、書きかえる前の
// app/dungeon.c の original_commands() の switch と、256 通りのキーのすべてで
// 一致することを確かめる。rogue-like の組の表（rogue_key_command()）も、書き
// かえる前の do_command() と wizard_command() の switch と突きあわせる。

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


// 書きかえる前の do_command() の switch のラベルの写し（道具で起こした）。case ごとの
// 最初の処理を名前にした。- は switch の前で方向を訊いて歩くキーに変えていた。
// 方向ごとの 24 キーは default で direction_keys の表を引いていた（direction_keys_test）。
static player_command_id legacy_do_command(char com_val, int *dir) {
    switch (com_val) {
    case 'Q':
        return CMD_QUIT;
    case CTRL_KEY('P'):
        return CMD_PREVIOUS_MESSAGE;
    case CTRL_KEY('V'):
        return CMD_VIEW_LICENSE;
    case CTRL_KEY('W'):
        return CMD_WIZARD_MODE;
    case CTRL_KEY('X'):
        return CMD_SAVE_AND_EXIT;
    case '=':
        return CMD_OPTIONS;
    case '{':
        return CMD_INSCRIBE;
    case '!':
    case '$':
        return CMD_SHELL;
    case ESCAPE:
    case ' ':
        return CMD_NONE;
    case '/':
        return CMD_IDENTIFY_SYMBOL;
    case '.':
        return CMD_STAY;
    case '<':
        return CMD_GO_UP;
    case '>':
        return CMD_GO_DOWN;
    case '?':
        return CMD_HELP;
    case 'f':
        return CMD_BASH;
    case 'C':
        return CMD_CHARACTER;
    case 'D':
        return CMD_DISARM;
    case 'E':
        return CMD_EAT;
    case 'F':
        return CMD_FILL_LAMP;
    case 'G':
        return CMD_GAIN_SPELLS;
    case 'V':
        return CMD_VIEW_SCORES;
    case 'W':
        return CMD_LOCATE;
    case 'R':
        return CMD_REST;
    case '#':
        return CMD_SEARCH_MODE;
    case CTRL_KEY('M'):
        *dir = 2;
        return CMD_TUNNEL;
    case 'z':
        return CMD_AIM;
    case 'M':
        return CMD_MAP;
    case 'P':
        return CMD_BROWSE;
    case 'c':
        return CMD_CLOSE;
    case 'd':
        return CMD_DROP;
    case 'e':
        return CMD_EQUIPMENT;
    case 't':
        return CMD_THROW;
    case 'i':
        return CMD_INVENTORY;
    case 'S':
        return CMD_JAM;
    case 'x':
        return CMD_LOOK;
    case 'm':
        return CMD_CAST;
    case 'o':
        return CMD_OPEN;
    case 'p':
        return CMD_PRAY;
    case 'q':
        return CMD_QUAFF;
    case 'r':
        return CMD_READ;
    case 's':
        return CMD_SEARCH;
    case 'T':
        return CMD_TAKE_OFF;
    case 'Z':
        return CMD_USE;
    case 'v':
        return CMD_VERSION;
    case 'w':
        return CMD_WEAR;
    case 'X':
        return CMD_EXCHANGE;
    case '-':
        return CMD_WALK_NO_PICKUP;
    default:
        break;
    }
    direction_key_style style;
    int d = command_key_direction(com_val, &style);
    if (d != 0) {
        *dir = d;
        return style == DIR_KEY_WALK ? CMD_WALK : style == DIR_KEY_RUN ? CMD_RUN : CMD_TUNNEL;
    }
    return CMD_ILLEGAL;
}

// 書きかえる前の wizard_command() の switch のラベルの写し。do_command() の default
// から呼ばれていた（do_command() の case に無いキーだけが届く）。
static player_command_id legacy_wizard_command(char com_val) {
    switch (com_val) {
    case CTRL_KEY('A'):
        return CMD_WIZ_CURE_ALL;
    case CTRL_KEY('E'):
        return CMD_WIZ_CHARACTER;
    case CTRL_KEY('F'):
        return CMD_WIZ_GENOCIDE;
    case CTRL_KEY('G'):
        return CMD_WIZ_TREASURE;
    case CTRL_KEY('D'):
        return CMD_WIZ_GO_TO_LEVEL;
    case CTRL_KEY('O'):
        return CMD_WIZ_OBJECTS;
    case '\\':
        return CMD_WIZ_HELP;
    case CTRL_KEY('I'):
        return CMD_WIZ_IDENTIFY;
    case '*':
        return CMD_WIZ_LIGHT;
    case ':':
        return CMD_WIZ_MAP;
    case CTRL_KEY('T'):
        return CMD_WIZ_TELEPORT;
    case '+':
        return CMD_WIZ_EXPERIENCE;
    case '&':
        return CMD_WIZ_SUMMON;
    case '@':
        return CMD_WIZ_CREATE;
    default:
        return CMD_ILLEGAL;
    }
}

TEST(rogue_keys_match_the_switches) {
    int mismatch = 0;
    for (int k = 0; k < 256; k++) {
        char key = (char)k;
        int legacy_dir = 0;
        player_command_id legacy = legacy_do_command(key, &legacy_dir);
        if (legacy == CMD_ILLEGAL) {
            legacy = legacy_wizard_command(key);
        }

        player_command actual = rogue_key_command(key);
        if (actual.id != legacy || (legacy_dir != 0 && actual.dir != legacy_dir)) {
            mismatch++;
        }
    }
    ASSERT_EQ_INT(mismatch, 0);
}

// 不正なキー（原文の組の写しが返す '~'）は、名前の無いコマンドになる。
TEST(an_illegal_key_has_no_command) {
    ASSERT_EQ_INT(rogue_key_command('~').id, CMD_ILLEGAL);
}

int main(void) {
    RUN_TEST(original_keys_match_the_switch);
    RUN_TEST(tilde_is_not_an_original_key);
    RUN_TEST(rogue_keys_match_the_switches);
    RUN_TEST(an_illegal_key_has_no_command);
    TEST_SUMMARY();
}
