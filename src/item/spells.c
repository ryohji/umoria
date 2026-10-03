// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Player/creature spells, breaths, wands, scrolls, etc. code

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "equipment.h"
#include "dungeon_level.h"
#include "dungeon_map.h"
#include "externs.h"
#include "floor_items.h"
#include "inventory.h"
#include "monster_levels.h"
#include "monster_list.h"
#include "monster_recall.h"
#include "panel.h"
#include "player_abilities.h"
#include "player_class.h"
#include "player_hp.h"
#include "player_level.h"
#include "player_pos.h"
#include "player_timed_effects.h"

static void replace_spot(int, int, int);

// Sleep creatures adjacent to player -RAK-
int sleep_monsters1(int y, int x) {
    bool sleep = false;

    for (int i = y - 1; i <= y + 1; i++) {
        for (int j = x - 1; j <= x + 1; j++) {
            cave_type *c_ptr = square_at(i, j);
            if (c_ptr->cptr > 1) {
                monster_type *m_ptr = monster_list_at(c_ptr->cptr);
                creature_type *r_ptr = monster_get_creature(m_ptr->creature);

                const char *cdesc = monster_name((vtype){0}, m_ptr);

                if ((randint(MAX_MONS_LEVEL) < r_ptr->level) || (CD_NO_SLEEP & r_ptr->cdefense)) {
                    if (m_ptr->ml && (r_ptr->cdefense & CD_NO_SLEEP)) {
                        recall_update_characteristics(m_ptr->creature, CD_NO_SLEEP);
                    }

                    msg_print(CONCAT(cdesc, " is unaffected."));
                } else {
                    sleep = true;
                    m_ptr->csleep = 500;

                    msg_print(CONCAT(cdesc, " falls asleep."));
                }
            }
        }
    }

    return sleep;
}

// Handle a monster caught in a newly-created wall or earthquake.
// Non-phasing monsters take damage and may die; earth elementals heal.
static void monster_caught_in_wall(int monster_index) {
    monster_type *m_ptr = monster_list_at(monster_index);
    creature_type *r_ptr = monster_get_creature(m_ptr->creature);

    if (!(r_ptr->cmove & CM_PHASE)) {
        int damage;

        if (r_ptr->cmove & CM_ATTACK_ONLY) {
            // this will kill everything
            damage = 3000;
        } else {
            damage = damroll(4, 8);
        }

        const char *cdesc = monster_name((vtype){0}, m_ptr);
        msg_print(CONCAT(cdesc, " wails out in pain!"));
        if (mon_take_hit(monster_index, damage)) {
            msg_print(CONCAT(cdesc, " is embedded in the rock."));
            prt_experience();
        }
    } else if (r_ptr->cchar == 'E' || r_ptr->cchar == 'X') {
        // must be an earth elemental or an earth spirit, or a Xorn
        // increase its hit points
        m_ptr->hp += damroll(4, 8);
    }
}

// Detect any treasure on the current panel -RAK-
int detect_treasure(void) {
    bool detect = false;

    for (int i = panel_top_row(); i <= panel_bottom_row(); i++) {
        for (int j = panel_left_col(); j <= panel_right_col(); j++) {
            cave_type *c_ptr = square_at(i, j);

            if ((c_ptr->tptr != 0) && (floor_item_at(c_ptr->tptr)->tval == TV_GOLD) &&
                !test_light(i, j)) {
                c_ptr->fm = true;
                lite_spot(i, j);
                detect = true;
            }
        }
    }

    return detect;
}

// Detect all objects on the current panel -RAK-
int detect_object(void) {
    bool detect = false;

    for (int i = panel_top_row(); i <= panel_bottom_row(); i++) {
        for (int j = panel_left_col(); j <= panel_right_col(); j++) {
            cave_type *c_ptr = square_at(i, j);

            if ((c_ptr->tptr != 0) &&
                (floor_item_at(c_ptr->tptr)->tval < TV_MAX_OBJECT) &&
                !test_light(i, j)) {
                c_ptr->fm = true;
                lite_spot(i, j);
                detect = true;
            }
        }
    }

    return detect;
}

// Locates and displays traps on current panel -RAK-
int detect_trap(void) {
    bool detect = false;

    for (int i = panel_top_row(); i <= panel_bottom_row(); i++) {
        for (int j = panel_left_col(); j <= panel_right_col(); j++) {
            cave_type *c_ptr = square_at(i, j);

            if (c_ptr->tptr != 0) {
                if (floor_item_at(c_ptr->tptr)->tval == TV_INVIS_TRAP) {
                    c_ptr->fm = true;
                    change_trap(i, j);
                    detect = true;
                } else if (floor_item_at(c_ptr->tptr)->tval == TV_CHEST) {
                    inven_type *t_ptr = floor_item_at(c_ptr->tptr);
                    known2(t_ptr);
                }
            }
        }
    }

    return detect;
}

// Locates and displays all secret doors on current panel -RAK-
int detect_sdoor(void) {
    bool detect = false;

    for (int i = panel_top_row(); i <= panel_bottom_row(); i++) {
        for (int j = panel_left_col(); j <= panel_right_col(); j++) {
            cave_type *c_ptr = square_at(i, j);

            if (c_ptr->tptr != 0) {
                if (floor_item_at(c_ptr->tptr)->tval == TV_SECRET_DOOR) {
                    // Secret doors

                    c_ptr->fm = true;
                    change_trap(i, j);
                    detect = true;
                } else if (((floor_item_at(c_ptr->tptr)->tval == TV_UP_STAIR) || (floor_item_at(c_ptr->tptr)->tval == TV_DOWN_STAIR)) && !c_ptr->fm) {
                    // Staircases

                    c_ptr->fm = true;
                    lite_spot(i, j);
                    detect = true;
                }
            }
        }
    }

    return detect;
}

// Scan monsters on panel, reveal those matching predicate, show message if any found.
static bool detect_monsters_by_predicate(bool (*predicate)(const creature_type *), const char *message) {
    bool flag = false;

    for (int i = monster_list_used() - 1; i >= MIN_MONIX; i--) {
        monster_type *m_ptr = monster_list_at(i);
        creature_type *r_ptr = monster_get_creature(m_ptr->creature);

        if (panel_contains((int)m_ptr->fy, (int)m_ptr->fx) && predicate(r_ptr)) {
            m_ptr->ml = true;
            // works correctly even if hallucinating
            print((char)r_ptr->cchar, (int)m_ptr->fy, (int)m_ptr->fx);
            flag = true;
        }
    }

    if (flag) {
        msg_print(message);
        msg_print(CNIL);

        // must unlight every monster just lighted
        creatures(false);
    }

    return flag;
}

static bool is_invisible(const creature_type *r_ptr) {
    return (r_ptr->cmove & CM_INVISIBLE) != 0;
}

static bool is_visible(const creature_type *r_ptr) {
    return (r_ptr->cmove & CM_INVISIBLE) == 0;
}

static bool is_evil(const creature_type *r_ptr) {
    return (r_ptr->cdefense & CD_EVIL) != 0;
}

// Locates and displays all invisible creatures on current panel -RAK-
int detect_invisible(void) {
    return detect_monsters_by_predicate(is_invisible, "You sense the presence of invisible creatures!");
}

// Light an area: -RAK-
//     1.  If corridor  light immediate area
//     2.  If room      light entire room plus immediate area.
int light_area(int y, int x) {
    if (!player_timed_in_force(PLAYER_TIMED_BLINDNESS)) {
        msg_print("You are surrounded by a white light.");
    }

    bool light = true;

    if (square_at(y, x)->lr && !player_is_in_town()) {
        light_room(y, x);
    }

    // Must always light immediate area, because one might be standing on
    // the edge of a room, or next to a destroyed area, etc.
    for (int i = y - 1; i <= y + 1; i++) {
        for (int j = x - 1; j <= x + 1; j++) {
            square_at(i, j)->pl = true;
            lite_spot(i, j);
        }
    }

    return light;
}

// Darken an area, opposite of light area -RAK-
int unlight_area(int y, int x) {
    bool unlight = false;

    if (square_at(y, x)->lr && !player_is_in_town()) {
        int tmp1 = (SCREEN_HEIGHT / 2);
        int tmp2 = (SCREEN_WIDTH / 2);
        int start_row = (y / tmp1) * tmp1 + 1;
        int start_col = (x / tmp2) * tmp2 + 1;
        int end_row = start_row + tmp1 - 1;
        int end_col = start_col + tmp2 - 1;

        for (int i = start_row; i <= end_row; i++) {
            for (int j = start_col; j <= end_col; j++) {
                cave_type *c_ptr = square_at(i, j);
                if (c_ptr->lr && c_ptr->fval <= MAX_CAVE_FLOOR) {
                    c_ptr->pl = false;
                    c_ptr->fval = DARK_FLOOR;
                    lite_spot(i, j);
                    if (!test_light(i, j)) {
                        unlight = true;
                    }
                }
            }
        }
    } else {
        for (int i = y - 1; i <= y + 1; i++) {
            for (int j = x - 1; j <= x + 1; j++) {
                cave_type *c_ptr = square_at(i, j);
                if ((c_ptr->fval == CORR_FLOOR) && c_ptr->pl) {
                    // pl could have been set by star-lite wand, etc
                    c_ptr->pl = false;
                    unlight = true;
                }
            }
        }
    }

    if (unlight && !player_timed_in_force(PLAYER_TIMED_BLINDNESS)) {
        msg_print("Darkness surrounds you.");
    }

    return unlight;
}

// Map the current area plus some -RAK-
void map_area(void) {
    int i = panel_top_row() - randint(10);
    int j = panel_bottom_row() + randint(10);
    int k = panel_left_col() - randint(20);
    int l = panel_right_col() + randint(20);

    for (int m = i; m <= j; m++) {
        for (int n = k; n <= l; n++) {
            if (in_bounds(m, n) && (square_at(m, n)->fval <= MAX_CAVE_FLOOR)) {
                for (int i7 = m - 1; i7 <= m + 1; i7++) {
                    for (int i8 = n - 1; i8 <= n + 1; i8++) {
                        cave_type *c_ptr = square_at(i7, i8);

                        if (c_ptr->fval >= MIN_CAVE_WALL) {
                            c_ptr->pl = true;
                        } else if ((c_ptr->tptr != 0) && (floor_item_at(c_ptr->tptr)->tval >= TV_MIN_VISIBLE) && (floor_item_at(c_ptr->tptr)->tval <= TV_MAX_VISIBLE)) {
                            c_ptr->fm = true;
                        }
                    }
                }
            }
        }
    }

    prt_map();
}

// Identify an object -RAK-
int ident_spell(void) {

    bool ident = false;

    int item_val;
    if (get_item(&item_val, "Item you wish identified?", 0, inventory_and_equipment_slot_count(), CNIL, CNIL)) {
        ident = true;
        identify(&item_val);

        inven_type *i_ptr = inventory_and_equipment_at(item_val);
        known2(i_ptr);

        bigvtype tmp_str;
        objdes(tmp_str, i_ptr, true);

        msgtype out_val;
        if (item_val >= INVEN_WIELD) {
            calc_bonuses();
            (void)snprintf(out_val, sizeof(out_val), "%s: %s", describe_use(item_val), tmp_str);
        } else {
            (void)snprintf(out_val, sizeof(out_val), "%c %s", item_val + 97, tmp_str);
        }
        msg_print(out_val);
    }

    return ident;
}

// Get all the monsters on the level pissed off. -RAK-
int aggravate_monster(int dis_affect) {
    monster_type *m_ptr;

    bool aggravate = false;

    for (int i = monster_list_used() - 1; i >= MIN_MONIX; i--) {
        m_ptr = monster_list_at(i);
        m_ptr->csleep = 0;
        if ((m_ptr->cdis <= dis_affect) && (m_ptr->cspeed < 2)) {
            m_ptr->cspeed++;
            aggravate = true;
        }
    }

    if (aggravate) {
        msg_print("You hear a sudden stirring in the distance!");
    }

    return aggravate;
}

// Surround the fool with traps (chuckle) -RAK-
int trap_creation(void) {
    bool trap = true;

    for (int i = player_row() - 1; i <= player_row() + 1; i++) {
        for (int j = player_col() - 1; j <= player_col() + 1; j++) {
            // Don't put a trap under the player, since this can lead to
            // strange situations, e.g. falling through a trap door while
            // trying to rest, setting off a falling rock trap and ending
            // up under the rock.
            if (i == player_row() && j == player_col()) {
                continue;
            }

            cave_type *c_ptr = square_at(i, j);

            if (c_ptr->fval <= MAX_CAVE_FLOOR) {
                if (c_ptr->tptr != 0) {
                    (void)delete_object(i, j);
                }
                place_trap(i, j, randint(MAX_TRAP) - 1);

                // don't let player gain exp from the newly created traps
                floor_item_at(c_ptr->tptr)->p1 = 0;

                // open pits are immediately visible, so call lite_spot
                lite_spot(i, j);
            }
        }
    }

    return trap;
}

// Surround the player with doors. -RAK-
int door_creation(void) {
    bool door = false;

    for (int i = player_row() - 1; i <= player_row() + 1; i++) {
        for (int j = player_col() - 1; j <= player_col() + 1; j++) {
            if ((i != player_row()) || (j != player_col())) {
                cave_type *c_ptr = square_at(i, j);

                if (c_ptr->fval <= MAX_CAVE_FLOOR) {
                    door = true;

                    if (c_ptr->tptr != 0) {
                        (void)delete_object(i, j);
                    }

                    int k = popt();
                    c_ptr->fval = BLOCKED_FLOOR;
                    c_ptr->tptr = k;
                    invcopy(floor_item_at(k), OBJ_CLOSED_DOOR);
                    lite_spot(i, j);
                }
            }
        }
    }

    return door;
}

// Destroys any adjacent door(s)/trap(s) -RAK-
int td_destroy(void) {
    bool destroy = false;

    for (int i = player_row() - 1; i <= player_row() + 1; i++) {
        for (int j = player_col() - 1; j <= player_col() + 1; j++) {
            cave_type *c_ptr = square_at(i, j);
            if (c_ptr->tptr != 0) {
                if (((floor_item_at(c_ptr->tptr)->tval >= TV_INVIS_TRAP) &&
                     (floor_item_at(c_ptr->tptr)->tval <= TV_CLOSED_DOOR) &&
                     (floor_item_at(c_ptr->tptr)->tval != TV_RUBBLE)) ||
                    (floor_item_at(c_ptr->tptr)->tval == TV_SECRET_DOOR)) {
                    if (delete_object(i, j)) {
                        destroy = true;
                    }
                } else if ((floor_item_at(c_ptr->tptr)->tval == TV_CHEST) &&
                           (floor_item_at(c_ptr->tptr)->flags != 0)) {
                    // destroy traps on chest and unlock
                    floor_item_at(c_ptr->tptr)->flags &= ~(CH_TRAPPED | CH_LOCKED);
                    floor_item_at(c_ptr->tptr)->name2 = SN_UNLOCKED;
                    msg_print("You have disarmed the chest.");
                    known2(floor_item_at(c_ptr->tptr));
                    destroy = true;
                }
            }
        }
    }

    return destroy;
}

// Display all creatures on the current panel -RAK-
int detect_monsters(void) {
    return detect_monsters_by_predicate(is_visible, "You sense the presence of monsters!");
}

// Leave a line of light in given dir, blue light can sometimes
// hurt creatures. -RAK-
void light_line(int dir, int y, int x) {
    bool flag = false;
    int dist = -1;

    do {
        // put mmove at end because want to light up current spot
        dist++;

        cave_type *c_ptr = square_at(y, x);

        if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE) {
            flag = true;
        } else {
            if (!c_ptr->pl && !c_ptr->tl) {
                // set pl so that lite_spot will work
                c_ptr->pl = true;
                if (c_ptr->fval == LIGHT_FLOOR) {
                    if (panel_contains(y, x)) {
                        light_room(y, x);
                    }
                } else {
                    lite_spot(y, x);
                }
            }

            // set pl in case tl was true above
            c_ptr->pl = true;
            if (c_ptr->cptr > 1) {
                monster_type *m_ptr = monster_list_at(c_ptr->cptr);
                creature_type *r_ptr = monster_get_creature(m_ptr->creature);

                // light up and draw monster
                update_mon((int)c_ptr->cptr);

                const char *cdesc = monster_name((vtype){0}, m_ptr);

                if (CD_LIGHT & r_ptr->cdefense) {
                    if (m_ptr->ml) {
                        recall_update_characteristics(m_ptr->creature, CD_LIGHT);
                    }

                    if (mon_take_hit(c_ptr->cptr, damroll(2, 8))) {
                        msg_print(CONCAT(cdesc, " shrivels away in the light!"));
                        prt_experience();
                    } else {
                        msg_print(CONCAT(cdesc, " cringes from the light!"));
                    }
                }
            }
        }
        (void)mmove(dir, &y, &x);
    } while (!flag);
}

// Light line in all directions -RAK-
void starlite(int y, int x) {
    if (!player_timed_in_force(PLAYER_TIMED_BLINDNESS)) {
        msg_print("The end of the staff bursts into a blue shimmering light.");
    }

    for (int i = 1; i <= 9; i++) {
        if (i != 5) {
            light_line(i, y, x);
        }
    }
}

// Disarms all traps/chests in a given direction -RAK-
int disarm_all(int dir, int y, int x) {
    bool disarm = false;
    int dist = -1;

    cave_type *c_ptr;

    do {
        // put mmove at end, in case standing on a trap
        dist++;

        c_ptr = square_at(y, x);

        // note, must continue upto and including the first non open space,
        // because secret doors have fval greater than MAX_OPEN_SPACE
        if (c_ptr->tptr != 0) {
            inven_type *t_ptr = floor_item_at(c_ptr->tptr);

            if ((t_ptr->tval == TV_INVIS_TRAP) ||
                (t_ptr->tval == TV_VIS_TRAP)) {
                if (delete_object(y, x)) {
                    disarm = true;
                }
            } else if (t_ptr->tval == TV_CLOSED_DOOR) {
                // Locked or jammed doors become merely closed.
                t_ptr->p1 = 0;
            } else if (t_ptr->tval == TV_SECRET_DOOR) {
                c_ptr->fm = true;
                change_trap(y, x);
                disarm = true;
            } else if ((t_ptr->tval == TV_CHEST) && (t_ptr->flags != 0)) {
                msg_print("Click!");
                t_ptr->flags &= ~(CH_TRAPPED | CH_LOCKED);
                disarm = true;
                t_ptr->name2 = SN_UNLOCKED;
                known2(t_ptr);
            }
        }
        (void)mmove(dir, &y, &x);
    } while ((dist <= OBJ_BOLT_RANGE) && c_ptr->fval <= MAX_OPEN_SPACE);

    return disarm;
}

// Recharge a wand, staff, or rod.  Sometimes the item breaks. -RAK-
int recharge(int num) {
    int i, j, item_val;

    bool res = false;

    if (!find_range(TV_STAFF, TV_WAND, &i, &j)) {
        msg_print("You have nothing to recharge.");
    } else if (get_item(&item_val, "Recharge which item?", i, j, CNIL, CNIL)) {
        inven_type *i_ptr = inventory_at(item_val);

        res = true;

        // recharge  I = recharge(20) = 1/6  failure for empty 10th level wand
        // recharge II = recharge(60) = 1/10 failure for empty 10th level wand
        //
        // make it harder to recharge high level, and highly charged wands, note
        // that i can be negative, so check its value before trying to call randint().
        i = num + 50 - (int)i_ptr->level - i_ptr->p1;
        if (i < 19) {
            // Automatic failure.
            i = 1;
        } else {
            i = randint(i / 10);
        }

        if (i == 1) {
            msg_print("There is a bright flash of light.");
            inven_destroy(item_val);
        } else {
            num = (num / (i_ptr->level + 2)) + 1;
            i_ptr->p1 += 2 + randint(num);
            if (known2_p(i_ptr)) {
                clear_known2(i_ptr);
            }
            clear_empty(i_ptr);
        }
    }
    return res;
}

// Increase or decrease a creatures hit points -RAK-
int hp_monster(int dir, int y, int x, int dam) {
    bool monster = false;
    bool flag = false;
    int dist = 0;

    do {
        (void)mmove(dir, &y, &x);
        dist++;

        cave_type *c_ptr = square_at(y, x);

        if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE) {
            flag = true;
        } else if (c_ptr->cptr > 1) {
            flag = true;

            monster_type *m_ptr = monster_list_at(c_ptr->cptr);

            const char *cdesc = monster_name((vtype){0}, m_ptr);
            monster = true;
            if (mon_take_hit(c_ptr->cptr, dam)) {
                msg_print(CONCAT(cdesc, " dies in a fit of agony."));
                prt_experience();
            } else if (dam > 0) {
                msg_print(CONCAT(cdesc, " screams in agony."));
            }
        }
    } while (!flag);

    return monster;
}

// Drains life; note it must be living. -RAK-
int drain_life(int dir, int y, int x) {
    bool drain = false;
    bool flag = false;
    int dist = 0;

    do {
        (void)mmove(dir, &y, &x);
        dist++;

        cave_type *c_ptr = square_at(y, x);

        if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE) {
            flag = true;
        } else if (c_ptr->cptr > 1) {
            flag = true;

            monster_type *m_ptr = monster_list_at(c_ptr->cptr);
            creature_type *r_ptr = monster_get_creature(m_ptr->creature);

            if ((r_ptr->cdefense & CD_UNDEAD) == 0) {
                drain = true;

                const char *cdesc = monster_name((vtype){0}, m_ptr);
                if (mon_take_hit(c_ptr->cptr, 75)) {
                    msg_print(CONCAT(cdesc, " dies in a fit of agony."));
                    prt_experience();
                } else {
                    msg_print(CONCAT(cdesc, " screams in agony."));
                }
            } else {
                recall_update_characteristics(m_ptr->creature, CD_UNDEAD);
            }
        }
    } while (!flag);

    return drain;
}

// Increase or decrease a creatures speed -RAK-
// NOTE: cannot slow a winning creature (BALROG)
int speed_monster(int dir, int y, int x, int spd) {
    bool speed = false;
    bool flag = false;
    int dist = 0;

    do {
        (void)mmove(dir, &y, &x);
        dist++;

        cave_type *c_ptr = square_at(y, x);

        if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE) {
            flag = true;
        } else if (c_ptr->cptr > 1) {
            flag = true;

            monster_type *m_ptr = monster_list_at(c_ptr->cptr);
            creature_type *r_ptr = monster_get_creature(m_ptr->creature);

            const char *cdesc = monster_name((vtype){0}, m_ptr);
            if (spd > 0) {
                m_ptr->cspeed += spd;
                m_ptr->csleep = 0;
                msg_print(CONCAT(cdesc, " starts moving faster."));
                speed = true;
            } else if (randint(MAX_MONS_LEVEL) > r_ptr->level) {
                m_ptr->cspeed += spd;
                m_ptr->csleep = 0;
                msg_print(CONCAT(cdesc, " starts moving slower."));
                speed = true;
            } else {
                m_ptr->csleep = 0;
                msg_print(CONCAT(cdesc, " is unaffected."));
            }
        }
    } while (!flag);

    return speed;
}

// Confuse a creature -RAK-
int confuse_monster(int dir, int y, int x) {
    bool confuse = false;
    bool flag = false;
    int dist = 0;

    do {
        (void)mmove(dir, &y, &x);
        dist++;
        cave_type *c_ptr = square_at(y, x);

        if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE) {
            flag = true;
        } else if (c_ptr->cptr > 1) {
            monster_type *m_ptr = monster_list_at(c_ptr->cptr);
            creature_type *r_ptr = monster_get_creature(m_ptr->creature);

            const char *cdesc = monster_name((vtype){0}, m_ptr);
            flag = true;
            if ((randint(MAX_MONS_LEVEL) < r_ptr->level) || (CD_NO_SLEEP & r_ptr->cdefense)) {
                if (m_ptr->ml && (r_ptr->cdefense & CD_NO_SLEEP)) {
                    recall_update_characteristics(m_ptr->creature, CD_NO_SLEEP);
                }

                // Monsters which resisted the attack should wake up.
                // Monsters with innate resistence ignore the attack.
                if (!(CD_NO_SLEEP & r_ptr->cdefense)) {
                    m_ptr->csleep = 0;
                }

                msg_print(CONCAT(cdesc, " is unaffected."));
            } else {
                if (m_ptr->confused) {
                    m_ptr->confused += 3;
                } else {
                    m_ptr->confused = 2 + randint(16);
                }
                confuse = true;
                m_ptr->csleep = 0;

                msg_print(CONCAT(cdesc, " appears confused."));
            }
        }
    } while (!flag);

    return confuse;
}

// Sleep a creature. -RAK-
int sleep_monster(int dir, int y, int x) {
    bool sleep = false;
    bool flag = false;
    int dist = 0;

    do {
        (void)mmove(dir, &y, &x);
        dist++;

        cave_type *c_ptr = square_at(y, x);

        if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE) {
            flag = true;
        } else if (c_ptr->cptr > 1) {
            flag = true;

            monster_type *m_ptr = monster_list_at(c_ptr->cptr);
            creature_type *r_ptr = monster_get_creature(m_ptr->creature);

            const char *cdesc = monster_name((vtype){0}, m_ptr);

            if ((randint(MAX_MONS_LEVEL) < r_ptr->level) || (CD_NO_SLEEP & r_ptr->cdefense)) {
                if (m_ptr->ml && (r_ptr->cdefense & CD_NO_SLEEP)) {
                    recall_update_characteristics(m_ptr->creature, CD_NO_SLEEP);
                }

                msg_print(CONCAT(cdesc, " is unaffected."));
            } else {
                m_ptr->csleep = 500;
                sleep = true;

                msg_print(CONCAT(cdesc, " falls asleep."));
            }
        }
    } while (!flag);

    return sleep;
}

// Turn stone to mud, delete wall. -RAK-
int wall_to_mud(int dir, int y, int x) {
    bool wall = false;
    bool flag = false;
    int dist = 0;

    do {
        (void)mmove(dir, &y, &x);
        dist++;

        cave_type *c_ptr = square_at(y, x);

        // note, this ray can move through walls as it turns them to mud
        if (dist == OBJ_BOLT_RANGE) {
            flag = true;
        }

        if ((c_ptr->fval >= MIN_CAVE_WALL) && (c_ptr->fval != BOUNDARY_WALL)) {
            flag = true;
            (void)twall(y, x, 1, 0);

            if (test_light(y, x)) {
                msg_print("The wall turns into mud.");
                wall = true;
            }
        } else if ((c_ptr->tptr != 0) && (c_ptr->fval >= MIN_CLOSED_SPACE)) {
            flag = true;
            if (panel_contains(y, x) && test_light(y, x)) {
                msgtype out_val;
                bigvtype tmp_str;
                objdes(tmp_str, floor_item_at(c_ptr->tptr), false);
                (void)snprintf(out_val, sizeof(out_val), "The %s turns into mud.", tmp_str);
                msg_print(out_val);
                wall = true;
            }

            if (floor_item_at(c_ptr->tptr)->tval == TV_RUBBLE) {
                (void)delete_object(y, x);
                if (randint(10) == 1) {
                    place_object(y, x, false);
                    if (test_light(y, x)) {
                        msg_print("You have found something!");
                    }
                }
                lite_spot(y, x);
            } else {
                (void)delete_object(y, x);
            }
        }

        if (c_ptr->cptr > 1) {
            monster_type *m_ptr = monster_list_at(c_ptr->cptr);
            creature_type *r_ptr = monster_get_creature(m_ptr->creature);

            if (CD_STONE & r_ptr->cdefense) {
                const char *cdesc = monster_name((vtype){0}, m_ptr);
                // Should get these messages even if the monster is not visible.
                if (mon_take_hit(c_ptr->cptr, 100)) {
                    msg_print(CONCAT(cdesc, " dissolves!"));
                    prt_experience(); // print msg before calling prt_exp
                } else {
                    msg_print(CONCAT(cdesc, " grunts in pain!"));
                }
                recall_update_characteristics(m_ptr->creature, CD_STONE);
                flag = true;
            }
        }
    } while (!flag);

    return wall;
}

// Destroy all traps and doors in a given direction -RAK-
int td_destroy2(int dir, int y, int x) {
    bool destroy2 = false;
    int dist = 0;

    cave_type *c_ptr;

    do {
        (void)mmove(dir, &y, &x);
        dist++;

        c_ptr = square_at(y, x);

        // must move into first closed spot, as it might be a secret door
        if (c_ptr->tptr != 0) {
            inven_type *t_ptr = floor_item_at(c_ptr->tptr);

            if ((t_ptr->tval == TV_INVIS_TRAP) ||
                (t_ptr->tval == TV_CLOSED_DOOR) ||
                (t_ptr->tval == TV_VIS_TRAP) ||
                (t_ptr->tval == TV_OPEN_DOOR) ||
                (t_ptr->tval == TV_SECRET_DOOR)) {
                if (delete_object(y, x)) {
                    msg_print("There is a bright flash of light!");
                    destroy2 = true;
                }
            } else if ((t_ptr->tval == TV_CHEST) && (t_ptr->flags != 0)) {
                msg_print("Click!");
                t_ptr->flags &= ~(CH_TRAPPED | CH_LOCKED);
                destroy2 = true;
                t_ptr->name2 = SN_UNLOCKED;
                known2(t_ptr);
            }
        }
    } while ((dist <= OBJ_BOLT_RANGE) || c_ptr->fval <= MAX_OPEN_SPACE);

    return destroy2;
}

// Polymorph a monster -RAK-
// NOTE: cannot polymorph a winning creature (BALROG)
int poly_monster(int dir, int y, int x) {
    bool poly = false;
    bool flag = false;
    int dist = 0;

    do {
        (void)mmove(dir, &y, &x);
        dist++;

        cave_type *const c_ptr = square_at(y, x);

        if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE) {
            flag = true;
        } else if (c_ptr->cptr > 1) {
            monster_type *m_ptr = monster_list_at(c_ptr->cptr);
            creature_type *r_ptr = monster_get_creature(m_ptr->creature);

            if (randint(MAX_MONS_LEVEL) > r_ptr->level) {
                flag = true;
                delete_monster((int)c_ptr->cptr);

                // Place_monster() should always return true here. Any monster
                // but a town one, so the band starts above them.
                const int first = first_monster_at_level(1);
                const int16_t m = randint(monsters_up_to_level(MAX_MONS_LEVEL) - first) - 1 + first;
                poly = place_monster(y, x, monster_make_creature_handle(m), false);

                // don't test c_ptr->fm here, only pl/tl
                if (poly && panel_contains(y, x) && (c_ptr->tl || c_ptr->pl)) {
                    poly = true;
                }
            } else {
                const char *cdesc = monster_name((vtype){0}, m_ptr);
                msg_print(CONCAT(cdesc, " is unaffected."));
            }
        }
    } while (!flag);

    return poly;
}

// Create a wall. -RAK-
int build_wall(int dir, int y, int x) {
    bool build = false;
    int dist = 0;
    bool flag = false;

    int i = 0;
    do {
        (void)mmove(dir, &y, &x);
        dist++;

        cave_type *c_ptr = square_at(y, x);

        if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE) {
            flag = true;
        } else {
            if (c_ptr->tptr != 0) {
                (void)delete_object(y, x);
            }

            if (c_ptr->cptr > 1) {
                // stop the wall building
                flag = true;
                monster_caught_in_wall(c_ptr->cptr);
            }

            c_ptr->fval = MAGMA_WALL;
            c_ptr->fm = false;

            // Permanently light this wall if it is lit by player's lamp.
            c_ptr->pl = (c_ptr->tl || c_ptr->pl);
            lite_spot(y, x);
            i++;
            build = true;
        }
    } while (!flag);

    return build;
}

// Replicate a creature -RAK-
bool clone_monster(int dir, int y, int x) {
    int dist = 0;
    bool flag = false;

    do {
        (void)mmove(dir, &y, &x);
        dist++;

        cave_type *c_ptr = square_at(y, x);

        if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE) {
            flag = true;
        } else if (c_ptr->cptr > 1) {
            monster_list_at(c_ptr->cptr)->csleep = 0;

            // monptr of 0 is safe here, since can't reach here from creatures
            return multiply_monster(y, x, monster_list_at(c_ptr->cptr)->creature, 0);
        }
    } while (!flag);

    return false;
}

// Move the creature record to a new location -RAK-
void teleport_away(int monptr, int dis) {
    int yn, xn;

    monster_type *m_ptr = monster_list_at(monptr);
    int ctr = 0;

    do {
        do {
            yn = m_ptr->fy + (randint(2 * dis + 1) - (dis + 1));
            xn = m_ptr->fx + (randint(2 * dis + 1) - (dis + 1));
        } while (!in_bounds(yn, xn));

        ctr++;
        if (ctr > 9) {
            ctr = 0;
            dis += 5;
        }
    } while ((square_at(yn, xn)->fval >= MIN_CLOSED_SPACE) || (square_at(yn, xn)->cptr != 0));

    move_rec((int)m_ptr->fy, (int)m_ptr->fx, yn, xn);
    lite_spot((int)m_ptr->fy, (int)m_ptr->fx);
    m_ptr->fy = yn;
    m_ptr->fx = xn;

    // this is necessary, because the creature is
    // not currently visible in its new position.
    m_ptr->ml = false;
    m_ptr->cdis = distance(player_row(), player_col(), yn, xn);
    update_mon(monptr);
}

// Teleport player to spell casting creature -RAK-
void teleport_to(int ny, int nx) {
    int dis = 1;
    int ctr = 0;

    int y, x;
    do {
        y = ny + (randint(2 * dis + 1) - (dis + 1));
        x = nx + (randint(2 * dis + 1) - (dis + 1));
        ctr++;
        if (ctr > 9) {
            ctr = 0;
            dis++;
        }
    } while (!in_bounds(y, x) || (square_at(y, x)->fval >= MIN_CLOSED_SPACE) || (square_at(y, x)->cptr >= 2));

    move_rec(player_row(), player_col(), y, x);

    for (int i = player_row() - 1; i <= player_row() + 1; i++) {
        for (int j = player_col() - 1; j <= player_col() + 1; j++) {
            cave_type *c_ptr = square_at(i, j);
            c_ptr->tl = false;
            lite_spot(i, j);
        }
    }

    lite_spot(player_row(), player_col());
    player_place(y, x);
    check_view();

    // light creatures
    creatures(false);
}

// Teleport all creatures in a given direction away -RAK-
int teleport_monster(int dir, int y, int x) {
    bool flag = false;
    bool result = false;
    int dist = 0;

    do {
        (void)mmove(dir, &y, &x);
        dist++;

        cave_type *c_ptr = square_at(y, x);

        if ((dist > OBJ_BOLT_RANGE) || c_ptr->fval >= MIN_CLOSED_SPACE) {
            flag = true;
        } else if (c_ptr->cptr > 1) {
            monster_list_at(c_ptr->cptr)->csleep = 0; // wake it up
            teleport_away((int)c_ptr->cptr, MAX_SIGHT);
            result = true;
        }
    } while (!flag);

    return result;
}

// Delete all creatures within max_sight distance -RAK-
// NOTE : Winning creatures cannot be genocided
int mass_genocide(void) {
    bool result = false;

    for (int i = monster_list_used() - 1; i >= MIN_MONIX; i--) {
        monster_type *m_ptr = monster_list_at(i);
        creature_type *r_ptr = monster_get_creature(m_ptr->creature);

        if ((m_ptr->cdis <= MAX_SIGHT) && ((r_ptr->cmove & CM_WIN) == 0)) {
            delete_monster(i);
            result = true;
        }
    }

    return result;
}

// Delete all creatures of a given type from level. -RAK-
// This does not keep creatures of type from appearing later.
// NOTE : Winning creatures can not be genocided.
int genocide(void) {
    bool killed = false;

    char typ;
    if (get_com("Which type of creature do you wish exterminated?", &typ)) {
        for (int i = monster_list_used() - 1; i >= MIN_MONIX; i--) {
            monster_type *m_ptr = monster_list_at(i);
            creature_type *r_ptr = monster_get_creature(m_ptr->creature);
            if (typ == r_ptr->cchar) {
                if ((r_ptr->cmove & CM_WIN) == 0) {
                    delete_monster(i);
                    killed = true;
                } else {
                    // genocide is a powerful spell, so we will let the player
                    // know the names of the creatures he did not destroy,
                    // this message makes no sense otherwise
                    vtype out_val;
                    (void)sprintf(out_val, "The %s is unaffected.", r_ptr->name);
                    msg_print(out_val);
                }
            }
        }
    }

    return killed;
}

// Change speed of any creature . -RAK-
// NOTE: cannot slow a winning creature (BALROG)
int speed_monsters(int spd) {
    bool speed = false;

    for (int i = monster_list_used() - 1; i >= MIN_MONIX; i--) {
        monster_type *m_ptr = monster_list_at(i);
        creature_type *r_ptr = monster_get_creature(m_ptr->creature);

        const char *cdesc = monster_name((vtype){0}, m_ptr);
        if ((m_ptr->cdis > MAX_SIGHT) || !los(player_row(), player_col(), (int)m_ptr->fy, (int)m_ptr->fx)) {
            ; // do nothing
        } else if (spd > 0) {
            m_ptr->cspeed += spd;
            m_ptr->csleep = 0;

            if (m_ptr->ml) {
                speed = true;
                msg_print(CONCAT(cdesc, " starts moving faster."));
            }
        } else if (randint(MAX_MONS_LEVEL) > r_ptr->level) {
            m_ptr->cspeed += spd;
            m_ptr->csleep = 0;

            if (m_ptr->ml) {
                msg_print(CONCAT(cdesc, " starts moving slower."));
                speed = true;
            }
        } else if (m_ptr->ml) {
            m_ptr->csleep = 0;
            msg_print(CONCAT(cdesc, " is unaffected."));
        }
    }

    return speed;
}

// Sleep any creature . -RAK-
int sleep_monsters2(void) {
    bool sleep = false;

    for (int i = monster_list_used() - 1; i >= MIN_MONIX; i--) {
        monster_type *m_ptr = monster_list_at(i);
        creature_type *r_ptr = monster_get_creature(m_ptr->creature);

        const char *cdesc = monster_name((vtype){0}, m_ptr);
        if ((m_ptr->cdis > MAX_SIGHT) || !los(player_row(), player_col(), (int)m_ptr->fy, (int)m_ptr->fx)) {
            ; // do nothing
        } else if ((randint(MAX_MONS_LEVEL) < r_ptr->level) || (CD_NO_SLEEP & r_ptr->cdefense)) {
            if (m_ptr->ml) {
                if (r_ptr->cdefense & CD_NO_SLEEP) {
                    recall_update_characteristics(m_ptr->creature, CD_NO_SLEEP);
                }
                msg_print(CONCAT(cdesc, " is unaffected."));
            }
        } else {
            m_ptr->csleep = 500;
            if (m_ptr->ml) {
                msg_print(CONCAT(cdesc, " falls asleep."));
                sleep = true;
            }
        }
    }

    return sleep;
}

// Polymorph any creature that player can see. -RAK-
// NOTE: cannot polymorph a winning creature (BALROG)
int mass_poly(void) {
    bool mass = false;

    for (int i = monster_list_used() - 1; i >= MIN_MONIX; i--) {
        monster_type *m_ptr = monster_list_at(i);
        if (m_ptr->cdis <= MAX_SIGHT) {
            creature_type *r_ptr = monster_get_creature(m_ptr->creature);

            if ((r_ptr->cmove & CM_WIN) == 0) {
                int y = m_ptr->fy;
                int x = m_ptr->fx;
                delete_monster(i);

                // Place_monster() should always return true here. Any monster
                // but a town one, so the band starts above them.
                const int first = first_monster_at_level(1);
                const int16_t m = randint(monsters_up_to_level(MAX_MONS_LEVEL) - first) - 1 + first;
                mass = place_monster(y, x, monster_make_creature_handle(m), false);
            }
        }
    }

    return mass;
}

// Display evil creatures on current panel -RAK-
int detect_evil(void) {
    return detect_monsters_by_predicate(is_evil, "You sense the presence of evil!");
}

// Change players hit points in some manner -RAK-
int hp_player(int num) {
    bool res = false;

    // The window refuses to heal a character already at the top, and that
    // refusal is what decides whether anything is printed or said.
    if (player_heal_hp(num)) {
        prt_chp();

        num = num / 5;
        if (num < 3) {
            if (num == 0) {
                msg_print("You feel a little better.");
            } else {
                msg_print("You feel better.");
            }
        } else {
            if (num < 7) {
                msg_print("You feel much better.");
            } else {
                msg_print("You feel very good.");
            }
        }
        res = true;
    }

    return res;
}

// Shorten a timed effect to one turn, returning true if it was active.
// One turn is left on purpose: the message that the effect has passed
// comes out of the count-down in dungeon.c, so putting zero here would
// cure the character in silence.
static bool cure_timed_effect(player_timed_effect id) {
    if (player_timed_turns(id) > 1) {
        player_timed_shorten_to(id, 1);
        return true;
    }
    return false;
}

// Cure players confusion -RAK-
int cure_confusion(void) {
    return cure_timed_effect(PLAYER_TIMED_CONFUSION);
}

// Cure players blindness -RAK-
int cure_blindness(void) {
    return cure_timed_effect(PLAYER_TIMED_BLINDNESS);
}

// Cure poisoning -RAK-
int cure_poison(void) {
    return cure_timed_effect(PLAYER_TIMED_POISON);
}

// Cure the players fear -RAK-
int remove_fear(void) {
    return cure_timed_effect(PLAYER_TIMED_FEAR);
}

// This is a fun one.  In a given block, pick some walls and
// turn them into open spots.  Pick some open spots and turn
// them into walls.  An "Earthquake" effect. -RAK-
void earthquake(void) {
    for (int i = player_row() - 8; i <= player_row() + 8; i++) {
        for (int j = player_col() - 8; j <= player_col() + 8; j++) {
            if (((i != player_row()) || (j != player_col())) && in_bounds(i, j) && (randint(8) == 1)) {
                cave_type *c_ptr = square_at(i, j);

                if (c_ptr->tptr != 0) {
                    (void)delete_object(i, j);
                }

                if (c_ptr->cptr > 1) {
                    monster_caught_in_wall(c_ptr->cptr);
                }

                if ((c_ptr->fval >= MIN_CAVE_WALL) && (c_ptr->fval != BOUNDARY_WALL)) {
                    c_ptr->fval = CORR_FLOOR;
                    c_ptr->pl = false;
                    c_ptr->fm = false;
                } else if (c_ptr->fval <= MAX_CAVE_FLOOR) {
                    int tmp = randint(10);

                    if (tmp < 6) {
                        c_ptr->fval = QUARTZ_WALL;
                    } else if (tmp < 9) {
                        c_ptr->fval = MAGMA_WALL;
                    } else {
                        c_ptr->fval = GRANITE_WALL;
                    }

                    c_ptr->fm = false;
                }
                lite_spot(i, j);
            }
        }
    }
}

// Evil creatures don't like this. -RAK-
int protect_evil(void) {
    bool res;

    if (!player_timed_in_force(PLAYER_TIMED_PROTECTION_FROM_EVIL)) {
        res = true;
    } else {
        res = false;
    }
    player_timed_add(PLAYER_TIMED_PROTECTION_FROM_EVIL, randint(25) + 3 * player_level());

    return res;
}

// Create some high quality mush for the player. -RAK-
void create_food(void) {
    cave_type *c_ptr = square_at(player_row(), player_col());

    if (c_ptr->tptr != 0) {
        // take no action here, don't want to destroy object under player
        msg_print("There is already an object under you.");

        // set free_turn_flag so that scroll/spell points won't be used
        free_turn_flag = true;
    } else {
        place_object(player_row(), player_col(), false);
        invcopy(floor_item_at(c_ptr->tptr), OBJ_MUSH);
    }
}

// Attempts to destroy a type of creature.  Success depends on
// the creatures level VS. the player's level -RAK-
int dispel_creature(int cflag, int damage) {
    bool dispel = false;

    for (int i = monster_list_used() - 1; i >= MIN_MONIX; i--) {
        monster_type *m_ptr = monster_list_at(i);
        creature_type *r_ptr = monster_get_creature(m_ptr->creature);

        if ((m_ptr->cdis <= MAX_SIGHT) && (cflag & r_ptr->cdefense) && los(player_row(), player_col(), (int)m_ptr->fy, (int)m_ptr->fx)) {
            recall_update_characteristics(m_ptr->creature, cflag);

            const char *cdesc = monster_name((vtype){0}, m_ptr);
            // Should get these messages even if the monster is not visible.
            if (mon_take_hit(i, randint(damage))) {
                msg_print(CONCAT(cdesc, " dissolves!"));
                prt_experience();
            } else {
                msg_print(CONCAT(cdesc, " shudders."));
            }

            dispel = true;
        }
    }

    return dispel;
}

// Attempt to turn (confuse) undead creatures. -RAK-
int turn_undead(void) {
    bool turn_und = false;

    for (int i = monster_list_used() - 1; i >= MIN_MONIX; i--) {
        monster_type *m_ptr = monster_list_at(i);
        creature_type *r_ptr = monster_get_creature(m_ptr->creature);

        if (m_ptr->cdis <= MAX_SIGHT && CD_UNDEAD & r_ptr->cdefense && los(player_row(), player_col(), m_ptr->fy, m_ptr->fx) && m_ptr->ml) {
            const char *cdesc = monster_name((vtype){0}, m_ptr);
            if (((player_level() + 1) > r_ptr->level) || (randint(5) == 1)) {
                msg_print(CONCAT(cdesc, " runs frantically!"));
                turn_und = true;
                recall_update_characteristics(m_ptr->creature, CD_UNDEAD);
                m_ptr->confused = player_level();
            } else {
                msg_print(CONCAT(cdesc, " is unaffected."));
            }
        }
    }

    return turn_und;
}

// Leave a glyph of warding. Creatures will not pass over! -RAK-
void warding_glyph(void) {
    cave_type *c_ptr = square_at(player_row(), player_col());

    if (c_ptr->tptr == 0) {
        int i = popt();
        c_ptr->tptr = i;
        invcopy(floor_item_at(i), OBJ_SCARE_MON);
    }
}

// Lose a strength point. -RAK-
void lose_str(void) {
    if (!player_stat_sustained(A_STR)) {
        (void)dec_stat(A_STR);
        msg_print("You feel very sick.");
    } else {
        msg_print("You feel sick for a moment,  it passes.");
    }
}

// Lose an intelligence point. -RAK-
void lose_int(void) {
    if (!player_stat_sustained(A_INT)) {
        (void)dec_stat(A_INT);
        msg_print("You become very dizzy.");
    } else {
        msg_print("You become dizzy for a moment,  it passes.");
    }
}

// Lose a wisdom point. -RAK-
void lose_wis(void) {
    if (!player_stat_sustained(A_WIS)) {
        (void)dec_stat(A_WIS);
        msg_print("You feel very naive.");
    } else {
        msg_print("You feel naive for a moment,  it passes.");
    }
}

// Lose a dexterity point. -RAK-
void lose_dex(void) {
    if (!player_stat_sustained(A_DEX)) {
        (void)dec_stat(A_DEX);
        msg_print("You feel very sore.");
    } else {
        msg_print("You feel sore for a moment,  it passes.");
    }
}

// Lose a constitution point. -RAK-
void lose_con(void) {
    if (!player_stat_sustained(A_CON)) {
        (void)dec_stat(A_CON);
        msg_print("You feel very sick.");
    } else {
        msg_print("You feel sick for a moment,  it passes.");
    }
}

// Lose a charisma point. -RAK-
void lose_chr(void) {
    if (!player_stat_sustained(A_CHR)) {
        (void)dec_stat(A_CHR);
        msg_print("Your skin starts to itch.");
    } else {
        msg_print("Your skin starts to itch, but feels better now.");
    }
}

// Lose experience -RAK-
void lose_exp(int32_t amount) {
    player_lose_experience(amount);

    prt_experience();

    // counted again from the bottom of the price list, which can put the level
    // down as well as up
    if (player_recompute_level()) {
        calc_hitpoints();

        if (player_class_spell_type() == MAGE) {
            calc_spells(A_INT);
            calc_mana(A_INT);
        } else if (player_class_spell_type() == PRIEST) {
            calc_spells(A_WIS);
            calc_mana(A_WIS);
        }
        prt_level();
        prt_title();
    }
}

// Slow Poison -RAK-
int slow_poison(void) {
    bool slow = false;

    if (player_timed_in_force(PLAYER_TIMED_POISON)) {
        // Halved, but never down to nothing -- the same reason as the cures.
        int halved = player_timed_turns(PLAYER_TIMED_POISON) / 2;
        player_timed_set(PLAYER_TIMED_POISON, halved < 1 ? 1 : halved);
        slow = true;
        msg_print("The effect of the poison has been reduced.");
    }

    return slow;
}

// Bless -RAK-
void bless(int amount) {
    player_timed_add(PLAYER_TIMED_BLESSING, amount);
}

// Detect Invisible for period of time -RAK-
void detect_inv2(int amount) {
    player_timed_add(PLAYER_TIMED_SEEING_INVISIBLE, amount);
}

static void replace_spot(int y, int x, int typ) {
    cave_type *c_ptr = square_at(y, x);

    switch (typ) {
    case 1:
    case 2:
    case 3:
        c_ptr->fval = CORR_FLOOR;
        break;
    case 4:
    case 7:
    case 10:
        c_ptr->fval = GRANITE_WALL;
        break;
    case 5:
    case 8:
    case 11:
        c_ptr->fval = MAGMA_WALL;
        break;
    case 6:
    case 9:
    case 12:
        c_ptr->fval = QUARTZ_WALL;
        break;
    }

    c_ptr->pl = false;
    c_ptr->fm = false;
    c_ptr->lr = false; // this is no longer part of a room

    if (c_ptr->tptr != 0) {
        (void)delete_object(y, x);
    }

    if (c_ptr->cptr > 1) {
        delete_monster((int)c_ptr->cptr);
    }
}

// The spell of destruction. -RAK-
// NOTE : Winning creatures that are deleted will be considered
//        as teleporting to another level.  This will NOT win
//        the game.
void destroy_area(int y, int x) {
    if (!player_is_in_town()) {
        for (int i = (y - 15); i <= (y + 15); i++) {
            for (int j = (x - 15); j <= (x + 15); j++) {
                if (in_bounds(i, j) && (square_at(i, j)->fval != BOUNDARY_WALL)) {
                    int k = distance(i, j, y, x);

                    // clear player's spot, but don't put wall there
                    if (k == 0) {
                        replace_spot(i, j, 1);
                    } else if (k < 13) {
                        replace_spot(i, j, randint(6));
                    } else if (k < 16) {
                        replace_spot(i, j, randint(9));
                    }
                }
            }
        }
    }
    msg_print("There is a searing blast of light!");
    player_timed_add(PLAYER_TIMED_BLINDNESS, 10 + randint(10));
}

// Enchants a plus onto an item. -RAK-
// `limit` param is the maximum bonus allowed; usually 10,
// but weapon's maximum damage when enchanting melee weapons to damage.
bool enchant(int16_t *plusses, int16_t limit) {
    // avoid randint(0) call
    if (limit <= 0) {
        return false;
    }

    int chance = 0;
    bool res = false;

    if (*plusses > 0) {
        chance = *plusses;

        // very rarely allow enchantment over limit
        if (randint(100) == 1) {
            chance = randint(chance) - 1;
        }
    }

    if (randint(limit) > chance) {
        *plusses += 1;
        res = true;
    }

    return res;
}

// Removes curses from equipment -RAK-
// Only the slots up to INVEN_OUTER, so the light source and the second weapon
// keep their curse. Note the other two spells with this effect walk different
// ranges: magic.c reaches every equipment slot, prayer.c every slot at all.
int remove_curse(void) {
    bool result = false;

    for (int i = equipment_first_slot(); i <= INVEN_OUTER; i++) {
        inven_type *i_ptr = equipment_at(i);

        if (TR_CURSED & i_ptr->flags) {
            i_ptr->flags &= ~TR_CURSED;
            calc_bonuses();
            result = true;
        }
    }

    return result;
}

// Restores any drained experience -RAK-
int restore_level(void) {
    bool restore = false;

    if (player_max_experience() > player_experience()) {
        restore = true;
        msg_print("You feel your life energies returning.");

        // this while loop is not redundant, ptr_exp may reduce the exp level
        while (player_restore_experience()) {
            prt_experience();
        }
    }

    return restore;
}
