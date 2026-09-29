// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Misc utility and initialization code, magic objects code

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"
#include "progress.h"

// gets a new random seed for the random number generator
void init_seeds(uint32_t seed) {
    uint32_t clock_var;

    if (seed == 0) {
        clock_var = (uint32_t)time((time_t *)0);
    } else {
        clock_var = seed;
    }
    progress_set_color_seed(clock_var);

    clock_var += 8762;
    progress_set_town_seed(clock_var);

    clock_var += 113452L;
    set_rnd_seed(clock_var);
    // make it a little more random
    for (clock_var = (uint32_t)randint(100); clock_var != 0; clock_var--) {
        (void)rnd();
    }
}

// holds the previous rnd state
static uint32_t old_seed;

// change to different random number generator state
void set_seed(uint32_t seed) {
    old_seed = get_rnd_seed();

    // want reproducible state here
    set_rnd_seed(seed);
}

// restore the normal random generator state
void reset_seed(void) {
    set_rnd_seed(old_seed);
}

// Generates a random integer x where 1<=X<=MAXVAL -RAK-
int randint(int maxval) {
    int32_t randval = rnd();
    return ((int)(randval % maxval) + 1);
}

// Generates a random integer number of NORMAL distribution -RAK-
int randnor(int mean, int stand) {
    // alternate randnor code, slower but much smaller since no table
    // 2 per 1,000,000 will be > 4*SD, max is 5*SD
    //
    // tmp = damroll(8, 99);   // mean 400, SD 81
    // tmp = (tmp - 400) * stand / 81;
    // return tmp + mean;

    int tmp = randint(MAX_SHORT);

    // off scale, assign random value between 4 and 5 times SD
    if (tmp == MAX_SHORT) {
        int offset = 4 * stand + randint(stand);

        // one half are negative
        if (randint(2) == 1) {
            offset = -offset;
        }

        return mean + offset;
    }

    // binary search normal normal_table to get index that
    // matches tmp this takes up to 8 iterations.
    int low = 0;
    int iindex = NORMAL_TABLE_SIZE >> 1;
    int high = NORMAL_TABLE_SIZE;

    while (true) {
        if ((normal_table[iindex] == tmp) || (high == (low + 1))) {
            break;
        }
        if (normal_table[iindex] > tmp) {
            high = iindex;
            iindex = low + ((iindex - low) >> 1);
        } else {
            low = iindex;
            iindex = iindex + ((high - iindex) >> 1);
        }
    }

    // might end up one below target, check that here
    if (normal_table[iindex] < tmp) {
        iindex = iindex + 1;
    }

    // normal_table is based on SD of 64, so adjust the
    // index value here, round the half way case up.
    int offset = ((stand * iindex) + (NORMAL_TABLE_SD >> 1)) / NORMAL_TABLE_SD;

    // one half should be negative
    if (randint(2) == 1) {
        offset = -offset;
    }

    return mean + offset;
}

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

// Should the object be enchanted -RAK-
bool magik(int chance) {
    if (randint(100) <= chance) {
        return true;
    } else {
        return false;
    }
}
