// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Bolts, balls and breath: projecting an effect across the map

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

#include "dungeon_map.h"
#include "floor_items.h"
#include "monster_list.h"
#include "panel.h"
#include "player_status_flags.h"
#include "player_timed_effects.h"

// Return flags for given type area affect -RAK-
static void get_flags(int typ, uint32_t *weapon_type, int *harm_type, bool (**destroy)(inven_type *)) {
    switch (typ) {
    case GF_MAGIC_MISSILE:
        *weapon_type = 0;
        *harm_type = 0;
        *destroy = set_null;
        break;
    case GF_LIGHTNING:
        *weapon_type = CS_BR_LIGHT;
        *harm_type = CD_LIGHT;
        *destroy = set_lightning_destroy;
        break;
    case GF_POISON_GAS:
        *weapon_type = CS_BR_GAS;
        *harm_type = CD_POISON;
        *destroy = set_null;
        break;
    case GF_ACID:
        *weapon_type = CS_BR_ACID;
        *harm_type = CD_ACID;
        *destroy = set_acid_destroy;
        break;
    case GF_FROST:
        *weapon_type = CS_BR_FROST;
        *harm_type = CD_FROST;
        *destroy = set_frost_destroy;
        break;
    case GF_FIRE:
        *weapon_type = CS_BR_FIRE;
        *harm_type = CD_FIRE;
        *destroy = set_fire_destroy;
        break;
    case GF_HOLY_ORB:
        *weapon_type = 0;
        *harm_type = CD_EVIL;
        *destroy = set_null;
        break;
    default:
        msg_print("ERROR in get_flags()\n");
    }
}

// Shoot a bolt in a given direction -RAK-
void fire_bolt(int typ, int dir, int y, int x, int dam, const char *bolt_typ) {
    bool flag = false;

    bool (*dummy)(inven_type *);
    int harm_type = 0;
    uint32_t weapon_type;
    get_flags(typ, &weapon_type, &harm_type, &dummy);

    int oldy = y;
    int oldx = x;
    int dist = 0;

    do {
        (void)mmove(dir, &y, &x);
        dist++;

        cave_type *c_ptr = square_at(y, x);

        lite_spot(oldy, oldx);
        if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE) {
            flag = true;
        } else {
            if (c_ptr->cptr > 1) {
                flag = true;

                monster_type *m_ptr = monster_list_at(c_ptr->cptr);
                creature_type *r_ptr = monster_get_creature(m_ptr->creature);

                // light up monster and draw monster, temporarily set
                // pl so that update_mon() will work
                int i = c_ptr->pl;
                c_ptr->pl = true;
                update_mon((int)c_ptr->cptr);
                c_ptr->pl = i;

                // draw monster and clear previous bolt
                put_qio();

                const char *cdesc = monster_name_lower((vtype){0}, m_ptr);
                msg_print(CONCAT("The ", bolt_typ, " strikes ", cdesc, "."));
                if (harm_type & r_ptr->cdefense) {
                    dam = dam * 2;
                    if (m_ptr->ml) {
                        recall_update_characteristics(m_ptr->creature, harm_type);
                    }
                } else if (weapon_type & r_ptr->spells) {
                    dam = dam / 4;
                    if (m_ptr->ml) {
                        recall_update_spell(m_ptr->creature, weapon_type);
                    }
                }

                if (mon_take_hit(c_ptr->cptr, dam)) {
                    char *const out_val = CONCAT(cdesc, " dies in a fit of agony.");
                    out_val[0] = toupper(out_val[0]); // Capitalize
                    msg_print(out_val);
                    prt_experience();
                } else if (dam > 0) {
                    char *const out_val = CONCAT(cdesc, " screams in agony.");
                    out_val[0] = toupper(out_val[0]); // Capitalize
                    msg_print(out_val);
                }
            } else if (panel_contains(y, x) && !player_timed_in_force(PLAYER_TIMED_BLINDNESS)) {
                print('*', y, x);

                // show the bolt
                put_qio();
            }
        }
        oldy = y;
        oldx = x;
    } while (!flag);
}

// Shoot a ball in a given direction.  Note that balls have an area affect. -RAK-
void fire_ball(int typ, int dir, int y, int x, int dam_hp, const char *descrip) {
    int thit = 0;
    int tkill = 0;
    int max_dis = 2;

    bool (*destroy)(inven_type *);
    int harm_type;
    uint32_t weapon_type;
    get_flags(typ, &weapon_type, &harm_type, &destroy);

    bool flag = false;

    int oldy = y;
    int oldx = x;
    int dist = 0;

    do {
        (void)mmove(dir, &y, &x);
        dist++;
        lite_spot(oldy, oldx);
        if (dist > OBJ_BOLT_RANGE) {
            flag = true;
        } else {
            cave_type *c_ptr = square_at(y, x);

            if ((c_ptr->fval >= MIN_CLOSED_SPACE) || (c_ptr->cptr > 1)) {
                flag = true;
                if (c_ptr->fval >= MIN_CLOSED_SPACE) {
                    y = oldy;
                    x = oldx;
                }
                // The ball hits and explodes.
                // The explosion.
                for (int i = y - max_dis; i <= y + max_dis; i++) {
                    for (int j = x - max_dis; j <= x + max_dis; j++) {
                        if (in_bounds(i, j) && (distance(y, x, i, j) <= max_dis) && los(y, x, i, j)) {
                            c_ptr = square_at(i, j);

                            if ((c_ptr->tptr != 0) && (*destroy)(floor_item_at(c_ptr->tptr))) {
                                (void)delete_object(i, j);
                            }

                            if (c_ptr->fval <= MAX_OPEN_SPACE) {
                                if (c_ptr->cptr > 1) {
                                    monster_type *m_ptr = monster_list_at(c_ptr->cptr);
                                    creature_type *r_ptr = monster_get_creature(m_ptr->creature);

                                    // lite up creature if visible, temp set pl so that update_mon works
                                    int tmp = c_ptr->pl;
                                    c_ptr->pl = true;
                                    update_mon((int)c_ptr->cptr);

                                    thit++;
                                    int dam = dam_hp;

                                    if (harm_type & r_ptr->cdefense) {
                                        dam = dam * 2;
                                        if (m_ptr->ml) {
                                            recall_update_characteristics(m_ptr->creature, harm_type);
                                        }
                                    } else if (weapon_type & r_ptr->spells) {
                                        dam = dam / 4;
                                        if (m_ptr->ml) {
                                            recall_update_spell(m_ptr->creature, weapon_type);
                                        }
                                    }

                                    dam = (dam / (distance(i, j, y, x) + 1));
                                    int k = mon_take_hit((int)c_ptr->cptr, dam);

                                    if (k) {
                                        tkill++;
                                    }
                                    c_ptr->pl = tmp;
                                } else if (panel_contains(i, j) && !player_timed_in_force(PLAYER_TIMED_BLINDNESS)) {
                                    print('*', i, j);
                                }
                            }
                        }
                    }
                }

                // show ball of whatever
                put_qio();

                for (int i = (y - 2); i <= (y + 2); i++) {
                    for (int j = (x - 2); j <= (x + 2); j++) {
                        if (in_bounds(i, j) && panel_contains(i, j) && (distance(y, x, i, j) <= max_dis)) {
                            lite_spot(i, j);
                        }
                    }
                }
                // End  explosion.

                if (thit == 1) {
                    vtype out_val;
                    (void)sprintf(out_val, "The %s envelops a creature!", descrip);
                    msg_print(out_val);
                } else if (thit > 1) {
                    vtype out_val;
                    (void)sprintf(out_val, "The %s envelops several creatures!", descrip);
                    msg_print(out_val);
                }

                if (tkill == 1) {
                    msg_print("There is a scream of agony!");
                } else if (tkill > 1) {
                    msg_print("There are several screams of agony!");
                }

                if (tkill >= 0) {
                    prt_experience();
                }
                // End ball hitting.
            } else if (panel_contains(y, x) && !player_timed_in_force(PLAYER_TIMED_BLINDNESS)) {
                print('*', y, x);

                // show bolt
                put_qio();
            }
            oldy = y;
            oldx = x;
        }
    } while (!flag);
}

// Breath weapon works like a fire_ball, but affects the player.
// Note the area affect. -RAK-
void breath(int typ, int y, int x, int dam_hp, char *ddesc, int monptr) {
    int max_dis = 2;

    bool (*destroy)(inven_type *);
    int harm_type;
    uint32_t weapon_type;
    get_flags(typ, &weapon_type, &harm_type, &destroy);

    int dam;

    for (int i = y - 2; i <= y + 2; i++) {
        for (int j = x - 2; j <= x + 2; j++) {
            if (in_bounds(i, j) && (distance(y, x, i, j) <= max_dis) && los(y, x, i, j)) {
                cave_type *c_ptr = square_at(i, j);

                if ((c_ptr->tptr != 0) && (*destroy)(floor_item_at(c_ptr->tptr))) {
                    (void)delete_object(i, j);
                }

                if (c_ptr->fval <= MAX_OPEN_SPACE) {
                    // must ask for the MARK, not the clock, here: a previous
                    // monster could have added turns of blindness that have not
                    // been announced yet, and the breath should still be visible
                    // until they take effect
                    if (panel_contains(i, j) && !player_effect_in_force(PLAYER_EFFECT_BLIND)) {
                        print('*', i, j);
                    }

                    if (c_ptr->cptr > 1) {
                        monster_type *m_ptr = monster_list_at(c_ptr->cptr);
                        creature_type *r_ptr = monster_get_creature(m_ptr->creature);

                        dam = dam_hp;

                        if (harm_type & r_ptr->cdefense) {
                            dam = dam * 2;
                        } else if (weapon_type & r_ptr->spells) {
                            dam = (dam / 4);
                        }

                        dam = (dam / (distance(i, j, y, x) + 1));

                        // can not call mon_take_hit here, since player does not
                        // get experience for kill
                        m_ptr->hp = m_ptr->hp - dam;
                        m_ptr->csleep = 0;

                        if (m_ptr->hp < 0) {
                            const uint32_t treas = monster_death(m_ptr->fy, m_ptr->fx, r_ptr->cmove);

                            if (m_ptr->ml) {
                                recall_update_move(m_ptr->creature, treas & ~CM_TREASURE);
                                recall_update_carry(m_ptr->creature, (treas & CM_TREASURE) >> CM_TR_SHIFT);
                            }

                            // It ate an already processed monster. Handle normally.
                            if (monptr < c_ptr->cptr) {
                                delete_monster((int)c_ptr->cptr);
                            } else {
                                // If it eats this monster, an already processed monster
                                // will take its place, causing all kinds of havoc.
                                // Delay the kill a bit.
                                fix1_delete_monster((int)c_ptr->cptr);
                            }
                        }
                    } else if (c_ptr->cptr == 1) {
                        dam = (dam_hp / (distance(i, j, y, x) + 1));

                        // let's do at least one point of damage
                        // prevents randint(0) problem with poison_gas, also
                        if (dam == 0) {
                            dam = 1;
                        }

                        switch (typ) {
                        case GF_LIGHTNING:
                            light_dam(dam, ddesc);
                            break;
                        case GF_POISON_GAS:
                            poison_gas(dam, ddesc);
                            break;
                        case GF_ACID:
                            acid_dam(dam, ddesc);
                            break;
                        case GF_FROST:
                            cold_dam(dam, ddesc);
                            break;
                        case GF_FIRE:
                            fire_dam(dam, ddesc);
                            break;
                        }
                    }
                }
            }
        }
    }

    // show the ball of gas
    put_qio();

    for (int i = (y - 2); i <= (y + 2); i++) {
        for (int j = (x - 2); j <= (x + 2); j++) {
            if (in_bounds(i, j) && panel_contains(i, j) && (distance(y, x, i, j) <= max_dis)) {
                lite_spot(i, j);
            }
        }
    }
}
