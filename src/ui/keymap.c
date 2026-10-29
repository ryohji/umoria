// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The original key set, as a map to the rogue-like keys the commands go by.

#include "constant.h"
#include "ui_input.h"

#include <stddef.h>

static const struct {
    char key;
    char command;
} original_keys[] = {
    // keys that mean another rogue-like key
    {CTRL_KEY('K'), 'Q'}, // ^K = exit
    {CTRL_KEY('J'), '+'},
    {CTRL_KEY('M'), '+'},
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
