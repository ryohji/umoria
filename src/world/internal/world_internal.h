// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Declarations shared inside world/ only.

#ifndef WORLD_INTERNAL_H
#define WORLD_INTERNAL_H

#include "config.h"
#include "constant.h"
#include "types.h"

// --- combat/hit_rolls.c ---
int attack_blows(int, int *);
int tot_dam(inven_type *, int, creature_handle);
int critical_blow(int, int, int, int);
bool test_hit(int, int, int, int, int);

// --- combat/monster_damage.c ---
int mon_take_hit(int, int);

// --- combat/monster_melee.c ---
void make_attack(int);

// --- combat/player_damage.c ---
bool player_saves(void);
void corrode_gas(const char *);
void poison_gas(int, const char *);
void fire_dam(int, const char *);
void cold_dam(int, char *);
void light_dam(int, char *);
void acid_dam(int, const char *);

// --- combat/player_melee.c ---
void py_attack(int, int);
void py_bash(int, int);

// --- combat/projectiles.c ---
void breath(int, int, int, int, char *, int);

// --- dungeon/search.c ---
void change_trap(int, int);

// --- dungeon/lighting.c ---
void move_rec(int, int, int, int);
void light_room(int, int);
void lite_spot(int, int);
void move_light(int, int, int, int);

// --- dungeon/object_alloc.c ---
void place_trap(int, int, int);
void place_rubble(int, int);
void place_gold(int, int);
void place_object(int, int, bool);

// The function argument is set_room, set_corr or set_floor from sets.c. Each
// is a bool f(int) that takes cave[][].fval (the kind of floor).
void alloc_object(bool (*)(int), int, int);

// --- dungeon/terrain_commands.c ---
int twall(int, int, int, int);

// --- dungeon/traps.c ---
void hit_trap(int, int);
void chest_trap(int, int);

// --- item/desc.c ---
bool known1_p(inven_type *);
void clear_known2(inven_type *);
void clear_empty(inven_type *);
bool store_bought_p(inven_type *);
void sample(inven_type *);
void desc_charges(int);

// --- item/inven_ops.c ---
// Function argument is a predicate from sets.c (set_corrodes, set_flammable, etc.),
// each taking bool f(inven_type *).
int inven_damage(bool (*)(inven_type *), int);
bool inven_check_weight(inven_type *);

// --- item/spells.c ---
void teleport_away(int, int);
void teleport_to(int, int);

// --- monster/creature.c ---
void update_mon(int);
bool multiply_monster(int, int, creature_handle, int);

// --- monster/monster_breeding.c ---
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

// Give one back. The one call is fix1_delete_monster() (monster_death.c); see the
// asymmetry above. Floors at zero, as the old `if (mon_tot_mult > 0)` did, so a
// level whose monsters are killed without any breeding stays at nothing spent.
void monster_breeding_note_death(void);

// --- monster/monster_death.c ---
void delete_monster(int);
void fix1_delete_monster(int);
void fix2_delete_monster(int);
uint32_t monster_death(int, int, uint32_t);

// --- monster/monster_place.c ---
bool place_monster(int, int, creature_handle, int);
void place_win_monster(void);

// --- monster/monster_turn.c ---
// ONE NUMBER, AND IT ANSWERS ONE QUESTION: may the monster list be renumbered
// right now?
//
// The monster list is kept dense. delete_monster() closes a hole by moving the
// last monster down into it and taking the mark back one (monster_state.h), which
// renumbers every monster above the hole. Between turns that costs nothing. While creatures() is walking
// the list it is wrong: the walk is holding a number, and the monster that number
// names would change under it.
//
// So the two places that remove a monster ask here first. If the walk has not
// reached `index`, the hole may be closed (delete_monster). If it has, the hole
// must be left where it is (fix1_delete_monster, which leaves the mark alone).
//
// The upstream comment above each reader is worth keeping in mind -- "this is a
// horrible hack, the monster-list/creatures() code needs to be rewritten" -- and
// it is still true: the windows below do not fix the hack, they give it a name
// and one place to live.
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

// --- player/level_ops.c ---
void calc_hitpoints(void);

// --- player/run_path.c ---
void area_affect(int, int, int);

// --- player/stat_ops.c ---
void bst_stat(int, int);

// --- store/store_haggle.c ---
// Maximum final offer attempts before forced outcome
#define HAGGLE_FINAL_OFFER_LIMIT 3

// --- store/store_price.c ---
int32_t sell_price(int, int32_t *, int32_t *, inven_type *);
bool noneedtobargain(int, int32_t);
void updatebargain(int, int32_t, int32_t);

// --- view/map_view.c ---
uint8_t loc_symbol(int, int);
bool test_light(int, int);

// --- view/status_line.c ---
void prt_title(void);
void prt_level(void);

#endif // WORLD_INTERNAL_H
