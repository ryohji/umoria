// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How deep this character has ever been

#ifndef PLAYER_MAX_DEPTH_H
#define PLAYER_MAX_DEPTH_H

// THE QUESTION. How far down has this character ever got? Not where it is now --
// that is dun_level, which changes every time a staircase is used and goes back
// up as well as down. This number only ever grows.
//
// It is measured in dungeon levels, the same unit as dun_level, and the town is
// level 0. So zero means "has never gone below the town", which is where every
// character starts.
//
// THE FIRST QUESTION OUT OF struct misc that is not about the character's body
// or skill at all: it is a record of where the character has been. Three
// different files ask it for three unrelated reasons (the score, the scroll of
// word-of-recall, and the saved file), and only one place answers it.

// The deepest level ever reached. Zero means the character has never left town.
int player_max_depth(void);

// Remember that this level has been reached. KEEPS THE DEEPER OF THE TWO -- the
// comparison used to sit in dungeon.c ("Check for a maximum level"), and it is
// in here now because there is one caller, the reason can be said in the words
// of this module, and afterwards no `>` against this number is left outside.
//
// Going back up the stairs therefore cannot lower the record, which is what the
// question means.
void player_note_depth_reached(int level);

// Put a record back, REPLACING whatever is there. For the saved file only, the
// same as hp_table_slots(): rd_short() used to write straight into the field, and
// a load has to restore the number that was written rather than merge with it.
//
// Play may not use this -- the window above is the only way a character records
// having got deeper, and it cannot go backwards. (Today the two would agree
// anyway, because a game is loaded once, at startup, into a store that is still
// zero. The two windows are still separate, because "remember I reached level 12"
// and "the file says level 12" are different sentences, and only one of them is
// allowed to be a step backwards.)
void player_max_depth_set(int level);

// WHAT THIS MODULE DOES NOT ANSWER -- four things, all of them still in the
// callers:
//
//   1. WHICH LEVEL THE CHARACTER IS ON NOW. That is dun_level, a global of its
//      own, and it is the input to the window above rather than something this
//      module keeps.
//   2. WHAT THE RECORD IS WORTH. total_points() in death.c pays 100 points per
//      level. Scoring reads this number, but how generous the score is has
//      nothing to do with where the character has been.
//   3. WHERE WORD-OF-RECALL DROPS YOU. dungeon.c reads the record and hands it
//      to leave_for_level(); the "nowhere to be yanked to" branch for a
//      character still in town is about what the scroll does, not about the
//      record.
//   4. THE POSITION OF THE SHORT IN THE SAVED FILE. save.c keeps the byte order,
//      as it does for every question on this road.

#endif // PLAYER_MAX_DEPTH_H
