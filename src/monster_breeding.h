// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How many monsters have been bred on this level, and what that forbids

#ifndef MONSTER_BREEDING_H
#define MONSTER_BREEDING_H

// ONE NUMBER, AND IT ANSWERS ONE QUESTION: may another monster be bred here?
//
// Some monsters multiply (CM_MULTIPLY -- rats, flies, worm masses). One of them
// taking its turn can put a copy of itself in a free square beside it, and the
// copy breeds too, so a level left alone fills up. The number below is what
// stops that: each birth spends one of it, and once it is past MAX_MON_MULT the
// breeders stop breeding until the level is left.
//
// TWO OTHER THINGS PACE BREEDING AND NEITHER IS HERE. The monster has to have
// the CM_MULTIPLY flag, and the attempt only comes round when
// `abs(rest) % MON_MULT_ADJ` is zero -- the resting counter doubles as the clock
// (see player_resting.h). This module holds the budget and nothing else.
//
// IT IS A BUDGET, NOT A CENSUS. It does not count the monsters now alive, and it
// does not count only the bred ones:
//   - a birth spends one, wherever the newborn later goes;
//   - a monster removed the delayed way gives one back (fix1_delete_monster in
//     moria3.c) WHETHER OR NOT IT WAS EVER BRED -- that path is taken by any
//     monster that dies on its own turn, and by one that gets eaten;
//   - a monster removed the ordinary way gives nothing back (delete_monster).
// So the same level can hold far more monsters than the budget allows births,
// and clearing a room of hand-placed monsters can buy the breeders more room.
// That asymmetry is upstream's and is kept exactly.
//
// PER LEVEL. Going anywhere new resets it (dungeon.c), which is the whole
// meaning of the constant's comment, "Maximum reproductions on a level".
//
// This was `mon_tot_mult` in monsters.c -- an int16_t sitting next to the monster
// definition table, which is not where a per-level counter belongs. #18-14-3.

// A new level. Nothing has been bred here yet.
void monster_breeding_reset(void);

// May one more be bred? The comparison is unchanged from
// `MAX_MON_MULT >= mon_tot_mult`, which means the budget is MAX_MON_MULT + 1
// births, not MAX_MON_MULT: the count is allowed to equal the cap and still say
// yes, and the birth that answer permits takes it one past. Asked once, in
// mon_move() (creature.c), before any square is looked at.
bool monster_breeding_allowed(void);

// One has been bred. Two calls, both in multiply_monster() (creature.c), one for
// each way the new monster gets its square: the plain empty square, and the
// square freed by a cannibal eating what stood there. Both are counted only
// after place_monster() has actually succeeded -- a birth that fails because the
// monster list is full spends nothing.
void monster_breeding_note_birth(void);

// Give one back. The one call is fix1_delete_monster() (moria3.c); see the
// asymmetry above. Floors at zero, as the old `if (mon_tot_mult > 0)` did, so a
// level whose monsters are killed without any breeding stays at nothing spent.
void monster_breeding_note_death(void);

// The number itself, for the save file only (save.c writes it as a short and
// reads it back, so a restored level keeps the budget it had spent). Nothing
// else wants the raw count -- the readers above are the question (the same
// arrangement as missile_serial_value()).
int16_t monster_breeding_count(void);
void set_monster_breeding_count(int16_t count);

#endif // MONSTER_BREEDING_H
