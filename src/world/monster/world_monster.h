// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Public interface of world/monster/.

#ifndef WORLD_MONSTER_H
#define WORLD_MONSTER_H

#include "config.h"
#include "constant.h"
#include "types.h"

// --- creature.c ---
// The monsters' turns.
void creatures(void);

// --- monster_view.c ---
// Which monsters the player can see, without moving any of them; the dead ones
// are taken off the list.
void update_monsters(void);

// --- monster_breeding.c ---
// How many monsters have been bred on this level, and what that forbids
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
// (see player_condition.h). This module holds the budget and nothing else.
//
// IT IS A BUDGET, NOT A CENSUS. It does not count the monsters now alive, and it
// does not count only the bred ones:
//   - a birth spends one, wherever the newborn later goes;
//   - a monster removed the delayed way gives one back (fix1_delete_monster in
//     monster_death.c) WHETHER OR NOT IT WAS EVER BRED -- that path is taken by any
//     monster that dies on its own turn, and by one that gets eaten;
//   - a monster removed the ordinary way gives nothing back (delete_monster).
// So the same level can hold far more monsters than the budget allows births,
// and clearing a room of hand-placed monsters can buy the breeders more room.
// That asymmetry is upstream's and is kept exactly.
//
// PER LEVEL. Going anywhere new resets it (app/dungeon.c), which is the whole
// meaning of the constant's comment, "Maximum reproductions on a level".

// A new level. Nothing has been bred here yet.
void monster_breeding_reset(void);

// The number itself, for the save file only (save.c writes it as a short and
// reads it back). Nothing else wants the raw count -- the readers above are the
// question (the same arrangement as missile_serial_value()).
//
// THE VALUE READ BACK IS THEN THROWN AWAY, and that is upstream's behaviour, not
// an accident of this module: main() calls dungeon() as the first thing in its
// loop whether the cave was generated or restored, and dungeon() resets this
// counter before the first turn. So a restored level always starts with the
// whole budget, and the short in the file is a slot nobody reads. The windows
// keep the pair because the file format is fixed -- taking the write out would
// shift every field after it.
int16_t monster_breeding_count(void);
void set_monster_breeding_count(int16_t count);

// --- monster_death.c ---
int delete_object(int, int);

// --- monster_place.c ---
bool compact_monsters(void);
void alloc_monster(int, int, int);
bool summon_monster(int *, int *, int);
bool summon_undead(int *, int *);

#endif // WORLD_MONSTER_H
