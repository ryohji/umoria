// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Random number generator

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

// Define this to compile as a standalone test
// #define TEST_RNG

// This alg uses a prime modulus multiplicative congruential generator
//
// (PMMLCG), also known as a Lehmer Grammer, which satisfies the following
// properties
//
//   (i)   modulus: m - a large prime integer
//   (ii)  multiplier: a - an integer in the range 2, 3, ..., m - 1
//   (iii) z[n+1] = f(z[n]), for n = 1, 2, ...
//   (iv)  f(z) = az mod m
//   (v)   u[n] = z[n] / m, for n = 1, 2, ...
//
// The sequence of z's must be initialized by choosing an initial seed z[1]
// from the range 1, 2, ..., m - 1.  The sequence of z's is a pseudo-random
// sequence drawn without replacement from the set 1, 2, ..., m - 1.
// The u's form a psuedo-random sequence of real numbers between (but not
// including) 0 and 1.
//
// Schrage's method is used to compute the sequence of z's.
// Let m = aq + r, where q = m div a, and r = m mod a.
// Then f(z) = az mod m = az - m * (az div m) =
//           = gamma(z) + m * delta(z)
// Where gamma(z) = a(z mod q) - r(z div q)
// and   delta(z) = (z div q) - (az div m)
//
// If r < q, then for all z in 1, 2, ..., m - 1:
//   (1) delta(z) is either 0 or 1
//   (2) both a(z mod q) and r(z div q) are in 0, 1, ..., m - 1
//   (3) absolute value of gamma(z) <= m - 1
//   (4) delta(z) = 1 iff gamma(z) < 0
//
// Hence each value of z can be computed exactly without overflow as long
// as m can be represented as an integer.


// a good random number generator, correct on any machine with 32 bit
// integers, this algorithm is from:
//
// Stephen K. Park and Keith W. Miller, "Random Number Generators:
//       Good ones are hard to find", Communications of the ACM, October 1988,
//       vol 31, number 10, pp. 1192-1201.
//
//  If this algorithm is implemented correctly,
//  then if z[1] = 1,
//  then z[10001] will equal 1043618065
//
//  Has a full period of 2^31 - 1.
//  Returns integers in the range 1 to 2^31-1.

#define RNG_M   2147483647L // m = 2^31 - 1
#define RNG_A   16807L
#define RNG_Q   127773L     // m div a
#define RNG_R   2836L       // m mod a

// 32 bit seed
static uint32_t rnd_seed;

static uint32_t get_rnd_seed(void) {
    return rnd_seed;
}

void set_rnd_seed(uint32_t seedval) {
    // set seed to value between 1 and m-1
    rnd_seed = (uint32_t)((seedval % (RNG_M - 1)) + 1);
}

// returns a pseudo-random number from set 1, 2, ..., RNG_M - 1
int32_t rnd(void) {
    int32_t high = (int32_t)(rnd_seed / RNG_Q);
    int32_t low = (int32_t)(rnd_seed % RNG_Q);
    int32_t test = (int32_t)(RNG_A * low - RNG_R * high);

    if (test > 0) {
        rnd_seed = (uint32_t)test;
    } else {
        rnd_seed = (uint32_t)(test + RNG_M);
    }
    return rnd_seed;
}

// this table is used to generate a psuedo-normal distribution.  See
// the function randnor() below, this is much faster than calling
// transcendental function to calculate a true normal distribution.
uint16_t normal_table[NORMAL_TABLE_SIZE] = {
     206,     613,    1022,    1430,    1838,    2245,    2652,    3058,
    3463,    3867,    4271,    4673,    5075,    5475,    5874,    6271,
    6667,    7061,    7454,    7845,    8234,    8621,    9006,    9389,
    9770,   10148,   10524,   10898,   11269,   11638,   12004,   12367,
   12727,   13085,   13440,   13792,   14140,   14486,   14828,   15168,
   15504,   15836,   16166,   16492,   16814,   17133,   17449,   17761,
   18069,   18374,   18675,   18972,   19266,   19556,   19842,   20124,
   20403,   20678,   20949,   21216,   21479,   21738,   21994,   22245,
   22493,   22737,   22977,   23213,   23446,   23674,   23899,   24120,
   24336,   24550,   24759,   24965,   25166,   25365,   25559,   25750,
   25937,   26120,   26300,   26476,   26649,   26818,   26983,   27146,
   27304,   27460,   27612,   27760,   27906,   28048,   28187,   28323,
   28455,   28585,   28711,   28835,   28955,   29073,   29188,   29299,
   29409,   29515,   29619,   29720,   29818,   29914,   30007,   30098,
   30186,   30272,   30356,   30437,   30516,   30593,   30668,   30740,
   30810,   30879,   30945,   31010,   31072,   31133,   31192,   31249,
   31304,   31358,   31410,   31460,   31509,   31556,   31601,   31646,
   31688,   31730,   31770,   31808,   31846,   31882,   31917,   31950,
   31983,   32014,   32044,   32074,   32102,   32129,   32155,   32180,
   32205,   32228,   32251,   32273,   32294,   32314,   32333,   32352,
   32370,   32387,   32404,   32420,   32435,   32450,   32464,   32477,
   32490,   32503,   32515,   32526,   32537,   32548,   32558,   32568,
   32577,   32586,   32595,   32603,   32611,   32618,   32625,   32632,
   32639,   32645,   32651,   32657,   32662,   32667,   32672,   32677,
   32682,   32686,   32690,   32694,   32698,   32702,   32705,   32708,
   32711,   32714,   32717,   32720,   32722,   32725,   32727,   32729,
   32731,   32733,   32735,   32737,   32739,   32740,   32742,   32743,
   32745,   32746,   32747,   32748,   32749,   32750,   32751,   32752,
   32753,   32754,   32755,   32756,   32757,   32757,   32758,   32758,
   32759,   32760,   32760,   32761,   32761,   32761,   32762,   32762,
   32763,   32763,   32763,   32764,   32764,   32764,   32764,   32765,
   32765,   32765,   32765,   32766,   32766,   32766,   32766,   32766,
};

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

// Should the object be enchanted -RAK-
bool magik(int chance) {
    if (randint(100) <= chance) {
        return true;
    } else {
        return false;
    }
}

#ifdef TEST_RNG

main() {
    set_rnd_seed(0L);

    for (int32_t i = 1; i < 10000; i++) {
        (void)rnd();
    }

    int32_t random = rnd();
    printf("z[10001] = %ld, should be 1043618065\n", random);
    if (random == 1043618065L) {
        printf("success!!!\n");
    }
}

#endif
