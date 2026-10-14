// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Losing a stat point to poison, bad food or a potion.

#ifndef STAT_LOSS_H
#define STAT_LOSS_H

// Lowers `stat` (A_STR to A_CHR) by one point unless it is sustained, and says
// which happened. Each stat has its own pair of messages.
void lose_stat(int stat);

#endif // STAT_LOSS_H
