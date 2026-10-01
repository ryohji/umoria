// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Whether the character is resting, and for how many more turns

#ifndef PLAYER_RESTING_H
#define PLAYER_RESTING_H

// Twenty-one callers in five files (dungeon.c twelve of them, creature.c three,
// rest_command.c three, misc3.c two, save.c two).
//
// THE QUESTION IS "IS THE CHARACTER RESTING, AND FOR HOW MANY MORE TURNS". The
// `R` command sets it, one block in dungeon.c moves it one step per turn, and
// reaching zero ends the rest.
//
// IT IS NOT ONE OF THE EIGHTEEN CLOCKS, and the three differences are why it is
// its own question:
//
//   - IT MOVES TOWARDS ZERO FROM BOTH SIDES. A positive count is "rest this many
//     turns" and counts down; a NEGATIVE count is "rest until the hit points and
//     the mana are full" and counts UP. rest_command.c puts -MAX_SHORT there when the
//     player answers the "how long" question with an asterisk.
//   - IT HAS TWO WAYS TO END: reaching zero, and -- while it is negative --
//     the hit points and the mana both being full. The second one reads two
//     other questions, so it stays with the caller (dungeon.c).
//   - READERS WHO DO NOT CARE ABOUT RESTING AT ALL. creature.c asks it three
//     times: a fast character only moves once per turn while resting, a sleeping
//     monster is much less likely to wake while the character rests, and
//     `abs(rest) % MON_MULT_ADJ` IS THE CLOCK THAT PACES BREEDING MONSTERS.
//     That last one has nothing to do with resting -- it just wants a number
//     that changes every turn.
//
// THE MARK IS SOMEWHERE ELSE, AND THIS MODULE DOES NOT TOUCH IT.
// player_status_flags.c keeps PY_REST ("the character is resting", its family
// 3), and the two are set and cleared together. Unlike the twelve pairs in
// player_timed_effects.c, THERE IS NOTHING TO FOLD: the count and the mark stand
// on adjacent lines in exactly two places, both in rest_command.c (rest() and
// rest_off()). Folding them would win two lines and cost this module the order
// of prt_state(), the digestion, and the "Press any key to stop resting"
// message. So the mark keeps its own callers, and status_line.c's state line goes on
// asking the MARK (the two agree in play, but the save file restores them
// separately).
//
// What this module deliberately does NOT do:
//
//   - -MAX_SHORT, and the check that the answer to "Rest for how long?" is
//     either that or a positive short. THAT IS READING THE PLAYER'S INPUT, and
//     it belongs with the question that asks it (rest_command.c),
//   - what starting and stopping lead to: the state line, the message, the
//     search mode that has to be turned off first, the digestion that speeds up
//     and slows down again (a different question -- player_food.c),
//   - whether the hit points and the mana are full (two other questions), which
//     is the second way a rest can end.

// No includes: the callers bring config.h, constant.h and types.h in first, the
// same as player_timed_effects.h and the eight before it.

// Resting at all: the count is not zero. Eight of the twenty-one places were
// this comparison, in both directions (`!= 0` and `== 0`).
bool player_resting(void);

// The count itself, SIGN AND ALL, for the three places that need more than "is
// the character resting": the state line (which shows the number when the option
// is on), the breeding clock in creature.c, and the save file's writer.
int player_rest_turns(void);

// Whether this is a "rest until healed" (the count is negative). dungeon.c needs
// it to know whether full hit points and mana should end the rest, and the state
// line shows "Rest *" instead of a number.
bool player_rest_is_until_healed(void);

// How long to rest. Positive is a number of turns, negative is "until healed",
// and the save file's reader uses this too -- it puts back whatever the file
// holds, WITHOUT the input check that rest_command.c does.
void player_rest_set(int turns);

// Not resting any more. Same as putting zero there; the name says which of the
// two the caller means, and every caller of this one is about to draw the state
// line again.
void player_rest_stop(void);

// dungeon.c's turn: ONE TURN OF RESTING PASSES. Moves the count one step
// TOWARDS zero -- down when it is positive, UP when it is negative -- and
// answers whether the rest ran out in this turn. Does nothing and answers false
// when the character is not resting, so that a count of zero cannot be pushed
// past it in either direction.
bool player_rest_count_down(void);

#endif
