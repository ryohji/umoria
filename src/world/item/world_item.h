// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Public interface of world/item/.

#ifndef WORLD_ITEM_H
#define WORLD_ITEM_H

#include "config.h"
#include "constant.h"
#include "types.h"
#include <stdbool.h>
#include <stdint.h>

// --- desc.c ---
void magic_init(void);
void known1(inven_type *);
void known2(inven_type *);
bool known2_p(inven_type *);
void store_bought(inven_type *);
void identify(int *);
void unmagic_name(inven_type *);
void objdes(char *, inven_type *, int);
void desc_remain(int);

// --- device_use.c ---
// Using a charged device (a staff or a wand): the steps the two commands share.
typedef struct {
    uint8_t tval;
    int penalty; // DEVICE_PENALTY_* (device.h)
    const char *none_carried;
    const char *prompt;
    const char *failed;
    const char *empty;
    // Called once the device is chosen, before the roll. Returns false to give
    // up; the turn is spent all the same. NULL when the device takes no
    // direction.
    bool (*choose_direction)(int *dir);
    // One effect, for bit `pos` (from 1) of the device's flags. Sets `*ident`
    // the way the effect wants. May replace `*flags`, the bits still to do.
    void (*effect)(int pos, int dir, uint32_t *flags, bool *ident);
} device_kind;

// Asks which device of the kind to use, rolls for it, then spends a charge on
// every effect it carries, or says it is empty. Returns whether that took a
// game turn (not when nothing was chosen or the direction was cancelled).
bool use_charged_device(const device_kind *kind);

// --- inscription.c ---
void scribe_object(void);
void add_inscribe(inven_type *, uint8_t);
void inscribe(inven_type *, const char *);

// --- inven_ops.c ---
void inven_destroy(int);
void take_one_item(inven_type *, inven_type *);
int weight_limit(void);
bool inven_check_num(inven_type *);
void check_strength(void);
int inven_carry(inven_type *);
int find_range(int, int, int *, int *);

// --- item_enchant.c ---
void magic_treasure(int, int);

// --- item_learn.c ---
// What the player learns by using a consumable item
// Declared here so this header does not depend on the whole of types.h.
typedef struct inven_type inven_type;

// Called after a potion / food / scroll has taken effect.
//
// When `effect_identified` is true and the item was not known yet, the player
// gains experience for the discovery and the item becomes identified.
// Otherwise the item is only marked as "tried".
//
// `item_val` is updated in place because identify() may merge stacks and
// renumber the inventory. The returned pointer is the item's new location;
// callers that keep an `inven_type *` must reassign it from the return value,
// as the old pointer can be stale.
inven_type *learn_item_effect(bool effect_identified, int *item_val);

// --- item_picker.c ---
// Item picker and display: showing inventory/equipment and choosing an item
// Display inventory items from r1 to r2.
// Returns the left column used.
int show_inven(int r1, int r2, bool weight, int col, const char *mask);

// Return a string describing how a given equipment item is carried.
const char *describe_use(int i);

// Display equipment items.
// Returns the left column used.
int show_equip(bool weight, int col);

// Remove item from equipment list.
void takeoff(int item_val, int posn);

// Get the ID of an item and return whether an item was chosen.
// Sets *com_val to the chosen item index.
// Asks "<prompt> <item description>?" and returns the answer.
int confirm_item(const char *prompt, int item);
int get_item(int *com_val, const char *pmt, int i, int j, const char *mask, const char *message);

// --- spellbook.c ---
void print_spells(int *, int, int, int);
void calc_spells(int);
bool gain_spells(void);
void calc_mana(int);
int cast_spell(const char *, int, int *, int *);

// --- spells.c ---

// What an effect that asks or checks first came to (ident_spell, recharge,
// create_food).
typedef enum {
    EFFECT_DONE,      // it took effect
    EFFECT_NOTHING,   // nothing to work on (recharge: no staff or wand); a turn still passes
    EFFECT_CANCELLED, // cancelled, or nothing happened; it takes no turn
} effect_outcome;

int sleep_monsters1(int, int);
int detect_treasure(void);
int detect_object(void);
int detect_trap(void);
int detect_sdoor(void);
int detect_invisible(void);
int light_area(int, int);
int unlight_area(int, int);
void map_area(void);
effect_outcome ident_spell(void);
int aggravate_monster(int);
int trap_creation(void);
int door_creation(void);
int td_destroy(void);
int detect_monsters(void);
void light_line(int, int, int);
void starlite(int, int);
int disarm_all(int, int, int);
effect_outcome recharge(int);
int hp_monster(int, int, int, int);
int drain_life(int, int, int);
int speed_monster(int, int, int, int);
int confuse_monster(int, int, int);
int sleep_monster(int, int, int);
int wall_to_mud(int, int, int);
int td_destroy2(int, int, int);
int poly_monster(int, int, int);
int build_wall(int, int, int);
bool clone_monster(int, int, int);
int teleport_monster(int, int, int);
int mass_genocide(void);
int genocide(void);
int speed_monsters(int);
int sleep_monsters2(void);
int mass_poly(void);
int detect_evil(void);
int hp_player(int);
int cure_confusion(void);
int cure_blindness(void);
int cure_poison(void);
int remove_fear(void);
void earthquake(void);
int protect_evil(void);
effect_outcome create_food(void);
int dispel_creature(int, int);
int turn_undead(void);
void warding_glyph(void);
void lose_exp(int32_t);
int slow_poison(void);
void bless(int);
void detect_inv2(int);
void destroy_area(int, int);
bool enchant(int16_t *, int16_t);
int remove_curse(void);
int restore_level(void);

#endif // WORLD_ITEM_H
