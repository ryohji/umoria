// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Which monster creatures() is acting for, and what that forbids

#ifndef MONSTER_TURN_H
#define MONSTER_TURN_H

// ONE NUMBER, AND IT ANSWERS ONE QUESTION: may the monster list be renumbered
// right now?
//
// The monster list is kept dense. delete_monster() closes a hole by moving the
// last monster down into it and taking the mark back one (monster_list.h), which
// renumbers every monster above the hole. Between turns that costs nothing. While creatures() is walking
// the list it is wrong: the walk is holding a number, and the monster that number
// names would change under it.
//
// So the two places that remove a monster ask here first. If the walk has not
// reached `index`, the hole may be closed (delete_monster). If it has, the hole
// must be left where it is (fix1_delete_monster, which leaves the mark alone).
//
// THE OLD NAME SAID WHAT IT WAS, NOT WHAT IT ANSWERED. This was
// `hack_monptr` in variable.c, and both readers spelled the question out by
// hand as `hack_monptr < i`. The upstream comment above each reader is worth
// keeping in mind -- "this is a horrible hack, the monster-list/creatures() code
// needs to be rewritten" -- and it is still true: the windows below do not fix the
// hack, they give it a name and one place to live. #18-14-1.
//
// NOT IN THE SAVE FILE. The number is only meaningful inside a call to
// creatures(), and save.c never writes it. A new game, and a restored one, both
// start at "nobody's turn".

// creatures() is about to do something on behalf of the monster at `index` that
// can compact the list. Five calls in creature.c, three kinds: the two summoning
// spells, the two place_monster() calls that breed a new monster, and the blow a
// monster takes for being stuck in a wall. The first four can fill the list and
// force compaction; the fifth can kill the monster whose turn it is, which is the
// other way into the question.
//
// THE BRACKET IS DELIBERATELY NARROW. Each pair wraps that one call and nothing
// else, exactly where the old assignments were. Widening them to the whole turn
// would change the game: any other delete that happened during that monster's
// turn would stop shifting the list and start leaving holes instead.
void monster_turn_begin(int index);

// Done. Nobody's turn again.
void monster_turn_end(void);

// May the hole at `index` be closed by renumbering? Asked by compact_monsters()
// (monster_place.c) and by the monster's death in mon_take_hit()
// (monster_damage.c), which each pick between delete_monster() and
// fix1_delete_monster() on the answer.
//
// True when nobody's turn is in progress, which is the normal case -- the
// resting value is -1 and every real index is at least MIN_MONIX (2), so the
// comparison says yes. It also says yes for a monster numbered above the one
// being acted for, which is what makes compaction possible at all during a
// summon: those entries are ones the walk has already passed.
//
// A THIRD PLACE ASKS THE SAME QUESTION WITHOUT COMING HERE, and it stays as it
// is: creature.c, where a monster eats another monster, writes `monptr <
// c_ptr->cptr` and picks between the same two deleters. It does not need the
// window because it is inside creatures() with the walk's own index in scope --
// the number this module keeps is the same number, passed the long way round for
// the benefit of code that is too far away to see it.
bool monster_delete_may_shift(int index);

// The raw number. One caller, the snapshot in game_state.c, which records it
// with the other "temporary" fields; nothing reads it back. Kept as a window so
// the number itself can stay private (the same arrangement as
// missile_serial_value()).
int monster_turn_index(void);

#endif // MONSTER_TURN_H
