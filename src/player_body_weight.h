// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How heavy this character's body is

#ifndef PLAYER_BODY_WEIGHT_H
#define PLAYER_BODY_WEIGHT_H

// THE QUESTION. How much does the character weigh? In pounds, not tenths of a
// pound -- a male Halfling is about sixty, a male Human about a hundred and
// eighty (src/player.c:107, the m_b_wt / f_b_wt columns of race[]).
//
// THE EIGHTH QUESTION OUT OF struct misc, after how deep the character has been,
// the hit die, the armour class, the base to-hit, the disarming skill, the saving
// throw and the race.
//
// NOT VANITY -- A PHYSICAL QUANTITY. The number is on the character sheet, but
// four of the six readers are rules, not display: a heavier character carries
// more (misc3.c's weight_limit()), bashes monsters with a shield harder
// (moria4.c's py_bash(), twice) and breaks doors down more easily
// (moria4.c:1000).
//
// WRITTEN ONCE IN A NORMAL CHARACTER'S LIFE, at creation (create.c, one line for
// each sex), plus a saved game being read back and the wizard's tweak.
//
// THERE IS NO `_adjust` WINDOW, and the reason is not the reason the race had
// one. THE RACE WAS NOT A NUMBER; THIS IS A NUMBER THAT NOBODY ADDS TO. Nothing
// in the game makes the character heavier or lighter -- not food, not the pack,
// not a curse. THE CHARACTER DOES NOT PUT ON WEIGHT. So the two windows below are
// the whole question, and this is the first unit on this road with fewer than
// three.

// The weight in pounds. EIGHT CALLERS: the carrying limit (misc3.c), the shield
// bash's to-hit and damage (moria4.c, twice, folded into one call), the door bash
// (moria4.c), the character sheet (misc3.c), the dumped character file (files.c),
// the saved file being written (save.c) and the wizard's prompt (wizard.c).
//
// An int, though the field is a uint16_t: every caller either divides it
// (`/ 10`, `/ 60`, `/ 2`), adds it to an int, or prints it with `(int)` already
// written in. Returning the narrow type would only move those casts around.
int player_body_weight(void);

// The weight outright. FOUR CALLERS: creation twice (one line per sex, because
// the male and female columns of race[] are different numbers), a saved file's
// short put back (save.c) and the wizard's "Weight =" prompt (wizard.c).
//
// THAT IS THE SEVENTH ANSWER to the question the fifteenth unit raised -- "does
// loading a saved game use the same window as the game itself?" -- and it is "the
// same", agreeing with the hit die, the base to-hit, the disarming skill, the
// saving throw and the race rather than with the armour class.
//
// IT REFUSES NOTHING, because the field refused nothing (ledger observation 24).
// The wizard's caller keeps its own `tmp_val > -1` check, which is about what
// that prompt will accept, not about what a body may weigh. WIDTH IS STILL A
// SHORT, so 65536 lands on 0 and -1 lands on 65535, exactly as
// `py.misc.wt = randnor(...)` behaved.
void player_body_weight_set(int pounds);

// WHAT THIS MODULE DOES NOT ANSWER -- three things, all still in the callers:
//
//   1. HOW MUCH THE CHARACTER CAN CARRY. misc3.c:942 computes
//      `use_stat[A_STR] * PLAYER_WEIGHT_CAP + weight`, capped at 3000. THE
//      SUBJECT OF THAT SENTENCE IS STRENGTH; the body's weight is the smaller
//      term. It also needs py.stats.use_stat[], which has no window yet, so even
//      if it belonged here it would have to wait (ledger observation 42) -- but
//      there is only one such site, so nothing is piling up.
//   2. HOW HARD A BASH LANDS. moria4.c:910 (`/ 10`), :920 (`/ 60 + 3`) and
//      :1000 (`/ 2`). Three different divisors for three different rules: those
//      are facts about bashing, not about the body, and there is no shared
//      expression to fold.
//   3. HOW THE NUMBER IS SHOWN. misc3.c:750 hands it to prt_num(), files.c:209
//      to fprintf("%6d"). Unlike the race's name, THE TWO SITES HAVE NO COMMON
//      EXPRESSION -- the receivers differ -- so there is no display window here.

#endif // PLAYER_BODY_WEIGHT_H
