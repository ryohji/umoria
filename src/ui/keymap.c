// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The two key sets: the original keys as a map to the rogue-like ones, and the
// rogue-like keys as a map to the commands.

#include "constant.h"
#include "ui_input.h"

#include <stddef.h>

static const struct {
    char key;
    char command;
} original_keys[] = {
    // keys that mean another rogue-like key
    {CTRL_KEY('K'), 'Q'}, // ^K = exit
    {CTRL_KEY('J'), ' '}, // Enter does nothing
    {CTRL_KEY('M'), ' '},
    {'1', 'b'},
    {'2', 'j'},
    {'3', 'n'},
    {'4', 'h'},
    {'5', '.'}, // Rest one turn
    {'6', 'l'},
    {'7', 'y'},
    {'8', 'k'},
    {'9', 'u'},
    {'B', 'f'},
    {'L', 'W'},
    {'S', '#'},
    {'a', 'z'},
    {'b', 'P'},
    {'f', 't'},
    {'h', '?'},
    {'j', 'S'},
    {'l', 'x'},
    {'t', 'T'},
    {'u', 'Z'},
    {'x', 'X'},

    // keys that are the same in both sets
    {CTRL_KEY('P'), CTRL_KEY('P')}, // ^P = repeat
    {CTRL_KEY('W'), CTRL_KEY('W')}, // ^W = password
    {CTRL_KEY('X'), CTRL_KEY('X')}, // ^X = save
    {CTRL_KEY('V'), CTRL_KEY('V')}, // ^V = view license
    {' ', ' '},
    {'!', '!'},
    {'$', '$'},
    {'/', '/'},
    {'<', '<'},
    {'>', '>'},
    {'-', '-'},
    {'=', '='},
    {'{', '{'},
    {'?', '?'},
    {'A', 'A'},
    {'C', 'C'},
    {'D', 'D'},
    {'E', 'E'},
    {'F', 'F'},
    {'G', 'G'},
    {'M', 'M'},
    {'R', 'R'},
    {'V', 'V'},
    {'c', 'c'},
    {'d', 'd'},
    {'e', 'e'},
    {'i', 'i'},
    {'m', 'm'},
    {'o', 'o'},
    {'p', 'p'},
    {'q', 'q'},
    {'r', 'r'},
    {'s', 's'},
    {'v', 'v'},
    {'w', 'w'},

    // wizard mode commands
    {CTRL_KEY('A'), CTRL_KEY('A')}, // ^A = cure all
    {CTRL_KEY('B'), CTRL_KEY('O')}, // ^B = objects
    {CTRL_KEY('D'), CTRL_KEY('D')}, // ^D = up/down
    {CTRL_KEY('H'), '\\'},          // ^H = wizhelp
    {CTRL_KEY('I'), CTRL_KEY('I')}, // ^I = identify
    {CTRL_KEY('L'), '*'},           // ^L = wizlight
    {':', ':'},
    {CTRL_KEY('T'), CTRL_KEY('T')}, // ^T = teleport
    {CTRL_KEY('E'), CTRL_KEY('E')}, // ^E = wizchar
    {CTRL_KEY('F'), CTRL_KEY('F')}, // ^F = genocide
    {CTRL_KEY('G'), CTRL_KEY('G')}, // ^G = treasure
    {'@', '@'},
    {'+', '+'},
    {CTRL_KEY('U'), '&'}, // ^U = summon
};

original_key_kind original_key_meaning(char key, char *command) {
    if (key == '.') {
        return ORIGINAL_KEY_RUN_PREFIX;
    }
    if (key == 'T') {
        return ORIGINAL_KEY_TUNNEL_PREFIX;
    }
    for (size_t n = 0; n < sizeof original_keys / sizeof original_keys[0]; n++) {
        if (original_keys[n].key == key) {
            *command = original_keys[n].command;
            return ORIGINAL_KEY_COMMAND;
        }
    }
    return ORIGINAL_KEY_ILLEGAL;
}

// The keys of the rogue-like set, other than the walk, run and tunnel keys of
// the direction table (direction_keys.c).
static const struct {
    char key;
    player_command_id id;
} rogue_keys[] = {
    {'Q', CMD_QUIT},
    {CTRL_KEY('P'), CMD_PREVIOUS_MESSAGE},
    {CTRL_KEY('V'), CMD_VIEW_LICENSE},
    {CTRL_KEY('W'), CMD_WIZARD_MODE},
    {CTRL_KEY('X'), CMD_SAVE_AND_EXIT},
    {'=', CMD_OPTIONS},
    {'{', CMD_INSCRIBE},
    {'!', CMD_SHELL},
    {'$', CMD_SHELL},
    {ESCAPE, CMD_NONE},
    {' ', CMD_NONE},
    {'/', CMD_IDENTIFY_SYMBOL},
    {'.', CMD_STAY},
    {'<', CMD_GO_UP},
    {'>', CMD_GO_DOWN},
    {'?', CMD_HELP},
    {'f', CMD_BASH},
    {'C', CMD_CHARACTER},
    {'D', CMD_DISARM},
    {'E', CMD_EAT},
    {'F', CMD_FILL_LAMP},
    {'G', CMD_GAIN_SPELLS},
    {'V', CMD_VIEW_SCORES},
    {'W', CMD_LOCATE},
    {'R', CMD_REST},
    {'#', CMD_SEARCH_MODE},
    {'z', CMD_AIM},
    {'M', CMD_MAP},
    {'P', CMD_BROWSE},
    {'c', CMD_CLOSE},
    {'d', CMD_DROP},
    {'e', CMD_EQUIPMENT},
    {'t', CMD_THROW},
    {'i', CMD_INVENTORY},
    {'S', CMD_JAM},
    {'x', CMD_LOOK},
    {'m', CMD_CAST},
    {'o', CMD_OPEN},
    {'p', CMD_PRAY},
    {'q', CMD_QUAFF},
    {'r', CMD_READ},
    {'s', CMD_SEARCH},
    {'T', CMD_TAKE_OFF},
    {'Z', CMD_USE},
    {'v', CMD_VERSION},
    {'w', CMD_WEAR},
    {'X', CMD_EXCHANGE},
    {'-', CMD_WALK_NO_PICKUP},

    // wizard mode commands
    {CTRL_KEY('A'), CMD_WIZ_CURE_ALL},
    {CTRL_KEY('E'), CMD_WIZ_CHARACTER},
    {CTRL_KEY('F'), CMD_WIZ_GENOCIDE},
    {CTRL_KEY('G'), CMD_WIZ_TREASURE},
    {CTRL_KEY('D'), CMD_WIZ_GO_TO_LEVEL},
    {CTRL_KEY('O'), CMD_WIZ_OBJECTS},
    {'\\', CMD_WIZ_HELP},
    {CTRL_KEY('I'), CMD_WIZ_IDENTIFY},
    {'*', CMD_WIZ_LIGHT},
    {':', CMD_WIZ_MAP},
    {CTRL_KEY('T'), CMD_WIZ_TELEPORT},
    {'+', CMD_WIZ_EXPERIENCE},
    {'&', CMD_WIZ_SUMMON},
    {'@', CMD_WIZ_CREATE},
};

player_command rogue_key_command(char key) {
    player_command command = {CMD_ILLEGAL, 0};

    direction_key_style style;
    int dir = command_key_direction(key, &style);
    if (dir != 0) {
        command.id = style == DIR_KEY_WALK ? CMD_WALK : style == DIR_KEY_RUN ? CMD_RUN : CMD_TUNNEL;
        command.dir = dir;
        return command;
    }
    // Return is not in the direction table; it digs down like ^J.
    if (key == CTRL_KEY('M')) {
        command.id = CMD_TUNNEL;
        command.dir = 2;
        return command;
    }
    for (size_t n = 0; n < sizeof rogue_keys / sizeof rogue_keys[0]; n++) {
        if (rogue_keys[n].key == key) {
            command.id = rogue_keys[n].id;
            return command;
        }
    }
    return command;
}
