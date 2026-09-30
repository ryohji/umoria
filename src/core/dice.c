// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Dice rolls: what the monster and object tables write as 2d6
//
// The three functions came over unchanged from misc1.c, the last of it (#54).

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

// generates damage for 2d6 style dice rolls
int damroll(int num, int sides) {
    int sum = 0;
    for (int i = 0; i < num; i++) {
        sum += randint(sides);
    }
    return sum;
}

int pdamroll(const uint8_t *array) {
    return damroll((int)array[0], (int)array[1]);
}


// Gives Max hit points -RAK-
int max_hp(const uint8_t *array) {
    return (array[0] * array[1]);
}
