// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#ifndef SAVE_H
#define SAVE_H

bool save_char(void);
bool _save_char(char *);
bool get_char(bool *);
void set_fileptr(FILE *);
void wr_highscore(high_scores *);
void rd_highscore(high_scores *);

#endif // SAVE_H
