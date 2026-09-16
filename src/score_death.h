// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How this life ended and whether it counts: the death flag, what did it, when
// the character was born, and the reasons the score is not to be recorded.

#ifndef SCORE_DEATH_H
#define SCORE_DEATH_H

// vtype comes from types.h, which has to be included before this header.

// This is storage and only storage. The window does not range-check or clamp,
// and it does not decide anything on the caller's behalf, because the old code
// did neither and doing it here would change behaviour rather than preserve it.

// --- the death flag ------------------------------------------------------

// True once the player has died. Read from twelve files: the monster and the
// main loop both stop what they are doing when it goes true, so it is a
// "leave off" signal as much as a record of the ending.
bool player_is_dead(void);
void set_player_dead(bool dead);

// --- what did it ---------------------------------------------------------

// The cause of death, as it goes on the tombstone and into the score file.
// Also set for endings that are not deaths ("(saved)", "Interrupting",
// "(panic save %d)"), so it says how the run ended rather than what killed
// the character.
//
// The pointer addresses the record itself and stays valid for the rest of the
// run: the callers write through it with strcpy() and sprintf(), and the save
// file reads straight into it.
char *death_cause(void);

// --- when the character was born -----------------------------------------

// The wall clock at character creation. The score file keeps it to tell two
// runs of the same player apart.
int32_t character_birth_date(void);
void set_character_birth_date(int32_t date);

// --- whether the score counts --------------------------------------------

// The reasons this run is not to be scored, as a set of bits rather than a
// flag: 0x1 resurrected, 0x2 entered wizard mode, 0x4 already on the
// scoreboard. Callers test single bits (prt_winner) and also the whole word
// against zero ("is there any reason at all"), so the window hands the word
// over rather than answering either question.
int16_t score_disqualifications(void);
void set_score_disqualifications(int16_t reasons);

#endif // SCORE_DEATH_H
