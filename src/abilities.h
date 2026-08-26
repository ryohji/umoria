// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Ratings of the player's miscellaneous abilities

#ifndef ABILITIES_H
#define ABILITIES_H

// The nine values shown in the "(Miscellaneous Abilities)" block.
//
// Returned as one value rather than through output parameters: nine
// out-parameters would be unreadable, and the caller needs all of them at once.
//
// `bth`, `bthb`, `fos`, `srh`, `stl`, `dis`, `save` and `dev` are fed to
// likert() by the callers; the divisor differs per ability so it stays there.
//
// `infra` is already formatted. Both call sites print it with the very same
// "%d feet", so keeping the format here is what stops the screen and the file
// from drifting apart again.
struct player_abilities {
    int bth;  // Fighting
    int bthb; // Bows/Throw
    int fos;  // Perception
    int srh;  // Searching
    int stl;  // Stealth
    int dis;  // Disarming
    int save; // Saving Throw
    int dev;  // Magic Device
    char infra[24]; // "%d feet"; 24 covers the whole int16_t range times ten
};

// Computes the ratings from the global player (py) and class_level_adj.
// Reads the player rather than taking it as a parameter, so that the two
// callers stay as they were.
struct player_abilities calc_player_abilities(void);

#endif // ABILITIES_H
