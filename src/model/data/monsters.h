// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#ifndef MONSTERS_H
#define MONSTERS_H

#include "config.h"
#include "constant.h"
#include "types.h"

// Following are creature arrays and variables
extern monster_type blank_monster; // Blank monster values

bool monster_attack_is_null(attack_handle h);
uint8_t monster_attack_get_type(attack_handle h);
uint8_t monster_attack_get_desc(attack_handle h);
uint8_t monster_attack_get_dice(attack_handle h);
uint8_t monster_attack_get_sides(attack_handle h);

// TODO: eliminate `monster_make_creature_handle`
// TODO: eliminate `m_ptr->creature.place` reference
creature_handle monster_make_creature_handle(uint16_t index);
creature_handle monster_get_creature_handle(creature_type *p);
creature_type *monster_get_creature(creature_handle h);

// Iteration over the creature definition table, first to last. Usage:
//   const creature_iterator end = monster_creature_end();
//   for (creature_iterator it = monster_creature_begin();
//        !monster_creature_same(it, end); it = monster_creature_next(it)) {
//       creature_type *const creature = monster_creature_get(it);
creature_iterator monster_creature_begin(void);
creature_iterator monster_creature_end(void);
// The entry at table index `index`, and the index of an iterator (for the
// save file and for tables kept beside this one).
creature_iterator monster_creature_at(int index);
int monster_creature_index(creature_iterator it);
bool monster_creature_same(creature_iterator a, creature_iterator b);
creature_iterator monster_creature_next(creature_iterator it);
creature_type *monster_creature_get(creature_iterator it);
// Random access: move `n` entries on (or back, when negative), and how many
// entries `to` lies after `from` (negative when it lies before). The result
// of advance must stay within the table or be end.
creature_iterator monster_creature_advance(creature_iterator it, int n);
int monster_creature_distance(creature_iterator from, creature_iterator to);

// Reverse iteration over the creature definition table. Usage:
//   const creature_rev_iterator end = monster_creature_rend();
//   for (creature_rev_iterator it = monster_creature_rbegin();
//        !monster_creature_rsame(it, end); it = monster_creature_rnext(it)) {
//       creature_type *const creature = monster_creature_rget(it);
creature_rev_iterator monster_creature_rbegin(void);
creature_rev_iterator monster_creature_rend(void);
bool monster_creature_rsame(creature_rev_iterator a, creature_rev_iterator b);
creature_rev_iterator monster_creature_rnext(creature_rev_iterator it);
creature_type *monster_creature_rget(creature_rev_iterator it);
// The forward iterator to the same entry as a non-end reverse iterator.
creature_iterator monster_creature_from_reverse(creature_rev_iterator it);

const char *monster_name(vtype, const monster_type *);
const char *monster_name_lower(vtype, const monster_type *);
const char *monster_name_or_something(vtype, const monster_type *);
const char *monster_name_indefinite(vtype, const creature_type *);

#endif // MONSTERS_H
