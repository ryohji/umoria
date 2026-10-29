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
int tot_dam(inven_type *, int, creature_iterator);
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
bool hit_trap(int, int);
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
bool multiply_monster(int, int, creature_iterator, int);

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
bool place_monster(int, int, creature_iterator, int);
void place_win_monster(void);

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

// --- constants ---
// Note that the following constants are all related, if you change one, you
// must also change all succeeding ones. Also, player_init[] and store_choice[]
// may also have to be changed.
#define OBJ_OPEN_DOOR           367
#define OBJ_CLOSED_DOOR         368

// Constants describing limits of certain objects
#define OBJ_BOLT_RANGE           18   // Maximum range of bolts and balls

// Trap constants
#define MAX_TRAP                 18   // Number of defined traps

// Player constants
#define WEAPON_WEIGHT_PER_STR    15   // Weapon effective weight limit per strength point

// Column for stats
#define STAT_COLUMN          0

#define TR_BLIND        0x08000000L
#define TR_TIMID        0x10000000L

// definitions for chests
#define CH_LOCKED       0x00000001L
#define CH_TRAPPED      0x000001F0L
#define CH_LOSE_STR     0x00000010L
#define CH_POISON       0x00000020L
#define CH_PARALYSED    0x00000040L
#define CH_EXPLODE      0x00000080L
#define CH_SUMMON       0x00000100L

#define CM_PHASE        0x00040000L

// creature defense flags
#define CD_LIGHT        0x0100

// indexes into the special name table
#define SN_EMPTY              46
#define SN_LOCKED             47

#endif // WORLD_INTERNAL_H
