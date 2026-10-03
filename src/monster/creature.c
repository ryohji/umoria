// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Handle monster movement and attacks

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "dungeon_map.h"
#include "equipment.h"
#include "externs.h"
#include "floor_items.h"
#include "monster_breeding.h"
#include "monster_list.h"
#include "monster_recall.h"
#include "monster_turn.h"
#include "panel.h"
#include "player_abilities.h"
#include "player_infra_range.h"
#include "player_light.h"
#include "player_mana.h"
#include "player_pos.h"
#include "player_resting.h"
#include "player_status_flags.h"
#include "player_stealth.h"
#include "player_timed_effects.h"
#include "progress.h"
#include "running.h"
#include "score_death.h"
#include "screen_touched.h"

// Updates screen when monsters move about -RAK-
void update_mon(int monptr) {
    cave_type *c_ptr;
    creature_type *r_ptr;

    bool flag = false;
    monster_type *m_ptr = monster_list_at(monptr);

    // Asked once and held in one place: the condition below read the same
    // number twice.
    const int infra_range = player_infra_range();

    if ((m_ptr->cdis <= MAX_SIGHT) && !player_effect_in_force(PLAYER_EFFECT_BLIND) &&
        (panel_contains((int)m_ptr->fy, (int)m_ptr->fx))) {
        if (progress_wizard_mode()) {
            // Wizard sight.
            flag = true;
        } else if (los(player_row(), player_col(), (int)m_ptr->fy, (int)m_ptr->fx)) {
            // Normal sight.
            c_ptr = square_at(m_ptr->fy, m_ptr->fx);
            r_ptr = monster_get_creature(m_ptr->creature);
            if (c_ptr->pl || c_ptr->tl || (player_is_running() && m_ptr->cdis < 2 && player_has_light())) {
                if ((CM_INVISIBLE & r_ptr->cmove) == 0) {
                    flag = true;
                } else if (player_can_see_invisible()) {
                    flag = true;
                    recall_update_move(m_ptr->creature, CM_INVISIBLE);
                }
            } else if (infra_range > 0 && m_ptr->cdis <= infra_range && (CD_INFRA & r_ptr->cdefense)) {
                // Infra vision. Two halves, and only the first one is about the
                // character: how far the warm blood carries (this module) and
                // whether THIS monster has any (CD_INFRA, which belongs to the
                // monster). The `> 0` guard is redundant in practice -- a
                // monster is never on the player's own square, so cdis is at
                // least one -- but removing it would change behaviour.

                flag = true;
                recall_update_characteristics(m_ptr->creature, CD_INFRA);
            }
        }
    }

    if (flag) {
        // Light it up.

        if (!m_ptr->ml) {
            disturb(1, 0);
            m_ptr->ml = true;
            lite_spot((int)m_ptr->fy, (int)m_ptr->fx);

            // notify inven_command
            note_screen_flushed();
        }
    } else if (m_ptr->ml) {
        // Turn it off.

        m_ptr->ml = false;
        lite_spot((int)m_ptr->fy, (int)m_ptr->fx);

        // notify inven_command
        note_screen_flushed();
    }
}

// How many times this creature acts on this turn. 0 means it does not act. -RAK-
//
// Fast creatures (speed > 0) act `speed` times, but only once while the
// player is resting. Slow creatures (speed <= 0, including exactly 0) act
// once every (2 - speed) turns: the longer the cycle, the slower they are.
//
// NOTE: the player must always move at least once per iteration; a slowed
// player is handled by moving monsters faster instead of the player slower.
static int moves_this_turn(int16_t speed) {
    if (speed > 0) {
        if (player_resting()) {
            return 1;
        } else {
            return speed;
        }
    } else {
        // On the first turn of the cycle it acts once, otherwise not at all.
        // Returning 1 or 0 explicitly, rather than the comparison itself,
        // says that this is a count of moves and not a truth value.
        return ((progress_turn() % (2 - speed)) == 0) ? 1 : 0;
    }
}

// Makes sure a new creature gets lit up. -CJS-
static bool check_mon_lite(int y, int x) {
    int monptr = square_at(y, x)->cptr;

    if (monptr <= 1) {
        return false;
    } else {
        update_mon(monptr);
        return monster_list_at(monptr)->ml;
    }
}

// Choose correct directions for monster movement -RAK-
static void get_moves(int monptr, int *mm) {
    int ay, ax, move_val;

    int y = monster_list_at(monptr)->fy - player_row();
    int x = monster_list_at(monptr)->fx - player_col();

    if (y < 0) {
        move_val = 8;
        ay = -y;
    } else {
        move_val = 0;
        ay = y;
    }
    if (x > 0) {
        move_val += 4;
        ax = x;
    } else {
        ax = -x;
    }

    // this has the advantage of preventing the diamond maneuvre, also faster
    if (ay > (ax << 1)) {
        move_val += 2;
    } else if (ax > (ay << 1)) {
        move_val++;
    }

    switch (move_val) {
    case 0:
        mm[0] = 9;
        if (ay > ax) {
            mm[1] = 8;
            mm[2] = 6;
            mm[3] = 7;
            mm[4] = 3;
        } else {
            mm[1] = 6;
            mm[2] = 8;
            mm[3] = 3;
            mm[4] = 7;
        }
        break;
    case 1:
    case 9:
        mm[0] = 6;
        if (y < 0) {
            mm[1] = 3;
            mm[2] = 9;
            mm[3] = 2;
            mm[4] = 8;
        } else {
            mm[1] = 9;
            mm[2] = 3;
            mm[3] = 8;
            mm[4] = 2;
        }
        break;
    case 2:
    case 6:
        mm[0] = 8;
        if (x < 0) {
            mm[1] = 9;
            mm[2] = 7;
            mm[3] = 6;
            mm[4] = 4;
        } else {
            mm[1] = 7;
            mm[2] = 9;
            mm[3] = 4;
            mm[4] = 6;
        }
        break;
    case 4:
        mm[0] = 7;
        if (ay > ax) {
            mm[1] = 8;
            mm[2] = 4;
            mm[3] = 9;
            mm[4] = 1;
        } else {
            mm[1] = 4;
            mm[2] = 8;
            mm[3] = 1;
            mm[4] = 9;
        }
        break;
    case 5:
    case 13:
        mm[0] = 4;
        if (y < 0) {
            mm[1] = 1;
            mm[2] = 7;
            mm[3] = 2;
            mm[4] = 8;
        } else {
            mm[1] = 7;
            mm[2] = 1;
            mm[3] = 8;
            mm[4] = 2;
        }
        break;
    case 8:
        mm[0] = 3;
        if (ay > ax) {
            mm[1] = 2;
            mm[2] = 6;
            mm[3] = 1;
            mm[4] = 9;
        } else {
            mm[1] = 6;
            mm[2] = 2;
            mm[3] = 9;
            mm[4] = 1;
        }
        break;
    case 10:
    case 14:
        mm[0] = 2;
        if (x < 0) {
            mm[1] = 3;
            mm[2] = 1;
            mm[3] = 6;
            mm[4] = 4;
        } else {
            mm[1] = 1;
            mm[2] = 3;
            mm[3] = 4;
            mm[4] = 6;
        }
        break;
    case 12:
        mm[0] = 1;
        if (ay > ax) {
            mm[1] = 2;
            mm[2] = 4;
            mm[3] = 3;
            mm[4] = 7;
        } else {
            mm[1] = 4;
            mm[2] = 2;
            mm[3] = 7;
            mm[4] = 3;
        }
        break;
    }
}

// Make the move if possible, five choices -RAK-
static void make_move(int monptr, int *mm, uint32_t *rcmove) {
    int newy, newx, stuck_door;
    cave_type *c_ptr;
    inven_type *t_ptr;

    int i = 0;
    bool do_turn = false;
    bool do_move = false;
    monster_type *m_ptr = monster_list_at(monptr);
    uint32_t movebits = monster_get_creature(m_ptr->creature)->cmove;

    do {
        // Get new position
        newy = m_ptr->fy;
        newx = m_ptr->fx;
        (void)mmove(mm[i], &newy, &newx);
        c_ptr = square_at(newy, newx);
        if (c_ptr->fval != BOUNDARY_WALL) {
            // Floor is open?
            if (c_ptr->fval <= MAX_OPEN_SPACE) {
                do_move = true;
            } else if (movebits & CM_PHASE) {
                // Creature moves through walls?

                do_move = true;
                *rcmove |= CM_PHASE;
            } else if (c_ptr->tptr != 0) {
                // Creature can open doors?

                t_ptr = floor_item_at(c_ptr->tptr);

                // Creature can open doors.
                if (movebits & CM_OPEN_DOOR) {
                    stuck_door = false;
                    if (t_ptr->tval == TV_CLOSED_DOOR) {
                        do_turn = true;

                        if (t_ptr->p1 == 0) {
                            // Closed doors

                            do_move = true;
                        } else if (t_ptr->p1 > 0) {
                            // Locked doors

                            if (randint((m_ptr->hp + 1) * (50 + t_ptr->p1)) <
                                40 * (m_ptr->hp - 10 - t_ptr->p1)) {
                                t_ptr->p1 = 0;
                            }
                        } else if (t_ptr->p1 < 0) {
                            // Stuck doors

                            if (randint((m_ptr->hp + 1) * (50 - t_ptr->p1)) <
                                40 * (m_ptr->hp - 10 + t_ptr->p1)) {
                                msg_print("You hear a door burst open!");
                                disturb(1, 0);
                                stuck_door = true;
                                do_move = true;
                            }
                        }
                    } else if (t_ptr->tval == TV_SECRET_DOOR) {
                        do_turn = true;
                        do_move = true;
                    }
                    if (do_move) {
                        invcopy(t_ptr, OBJ_OPEN_DOOR);

                        // 50% chance of breaking door
                        if (stuck_door) {
                            t_ptr->p1 = 1 - randint(2);
                        }
                        c_ptr->fval = CORR_FLOOR;
                        lite_spot(newy, newx);
                        *rcmove |= CM_OPEN_DOOR;
                        do_move = false;
                    }
                } else {
                    // Creature can not open doors, must bash them

                    if (t_ptr->tval == TV_CLOSED_DOOR) {
                        do_turn = true;
                        if (randint((m_ptr->hp + 1) * (80 + abs(t_ptr->p1))) <
                            40 * (m_ptr->hp - 20 - abs(t_ptr->p1))) {
                            invcopy(t_ptr, OBJ_OPEN_DOOR);

                            // 50% chance of breaking door
                            t_ptr->p1 = 1 - randint(2);
                            c_ptr->fval = CORR_FLOOR;
                            lite_spot(newy, newx);
                            msg_print("You hear a door burst open!");
                            disturb(1, 0);
                        }
                    }
                }
            }

            // Glyph of warding present?
            if (do_move && (c_ptr->tptr != 0) &&
                (floor_item_at(c_ptr->tptr)->tval == TV_VIS_TRAP) &&
                (floor_item_at(c_ptr->tptr)->subval == 99)) {
                if (randint(OBJ_RUNE_PROT) < monster_get_creature(m_ptr->creature)->level) {
                    if ((newy == player_row()) && (newx == player_col())) {
                        msg_print("The rune of protection is broken!");
                    }
                    (void)delete_object(newy, newx);
                } else {
                    do_move = false;

                    // If the creature moves only to attack,
                    // don't let it move if the glyph prevents
                    // it from attacking
                    if (movebits & CM_ATTACK_ONLY) {
                        do_turn = true;
                    }
                }
            }

            // Creature has attempted to move on player?
            if (do_move) {
                if (c_ptr->cptr == 1) {
                    // if the monster is not lit, must call update_mon, it
                    // may be faster than character, and hence could have
                    // just moved next to character this same turn.
                    if (!m_ptr->ml) {
                        update_mon(monptr);
                    }
                    make_attack(monptr);
                    do_move = false;
                    do_turn = true;
                } else if ((c_ptr->cptr > 1) && ((newy != m_ptr->fy) || (newx != m_ptr->fx))) {
                    // Creature is attempting to move on other creature?

                    // Creature eats other creatures?
                    if ((movebits & CM_EATS_OTHER) && (monster_get_creature(m_ptr->creature)->mexp >= monster_get_creature(monster_list_at(c_ptr->cptr)->creature)->mexp)) {
                        if (monster_list_at(c_ptr->cptr)->ml) {
                            *rcmove |= CM_EATS_OTHER;
                        }

                        // It ate an already processed monster. Handle normally.
                        if (monptr < c_ptr->cptr) {
                            delete_monster((int)c_ptr->cptr);
                        } else {
                            // If it eats this monster, an already processed
                            // monster will take its place, causing all kinds
                            // of havoc. Delay the kill a bit.
                            fix1_delete_monster((int)c_ptr->cptr);
                        }
                    } else {
                        do_move = false;
                    }
                }
            }

            // Creature has been allowed move.
            if (do_move) {
                // Pick up or eat an object
                if (movebits & CM_PICKS_UP) {
                    c_ptr = square_at(newy, newx);

                    if ((c_ptr->tptr != 0) && (floor_item_at(c_ptr->tptr)->tval <= TV_MAX_OBJECT)) {
                        *rcmove |= CM_PICKS_UP;
                        (void)delete_object(newy, newx);
                    }
                }

                // Move creature record
                move_rec((int)m_ptr->fy, (int)m_ptr->fx, newy, newx);
                if (m_ptr->ml) {
                    m_ptr->ml = false;
                    lite_spot((int)m_ptr->fy, (int)m_ptr->fx);
                }
                m_ptr->fy = newy;
                m_ptr->fx = newx;
                m_ptr->cdis = distance(player_row(), player_col(), newy, newx);
                do_turn = true;
            }
        }
        i++;

        // Up to 5 attempts at moving,  give up.
    } while ((!do_turn) && (i < 5));
}

// Creatures can cast spells too.  (Dragon Breath) -RAK-
//   cast_spell = true if creature changes position
//   took_turn  = true if creature casts a spell
static void mon_cast_spell(int monptr, bool *took_turn) {
    if (player_is_dead()) {
        return;
    }

    monster_type *m_ptr = monster_list_at(monptr);
    creature_type *r_ptr = monster_get_creature(m_ptr->creature);
    int chance = (int)(r_ptr->spells & CS_FREQ);

    if (randint(chance) != 1) {
        // 1 in x chance of casting spell

        *took_turn = false;
    } else if (m_ptr->cdis > MAX_SPELL_DIS) {
        // Must be within certain range

        *took_turn = false;
    } else if (!los(player_row(), player_col(), (int)m_ptr->fy, (int)m_ptr->fx)) {
        // Must have unobstructed Line-Of-Sight

        *took_turn = false;
    } else {
        // Creature is going to cast a spell

        *took_turn = true;

        // Check to see if monster should be lit.
        update_mon(monptr);

        // Describe the attack
        const char *cdesc = monster_name((vtype){0}, m_ptr);

        // For "DIED_FROM" string
        vtype ddesc;
        monster_name_indefinite(ddesc, r_ptr);

        // Extract all possible spells into spell_choice
        int spell_choice[30];
        uint32_t i = (r_ptr->spells & ~CS_FREQ);
        int k = 0;

        while (i != 0) {
            spell_choice[k] = bit_pos(&i);
            k++;
        }

        // Choose a spell to cast
        int thrown_spell = spell_choice[randint(k) - 1];
        thrown_spell++;

        // all except teleport_away() and drain mana spells always disturb
        if (thrown_spell > 6 && thrown_spell != 17) {
            disturb(1, 0);
        }

        // save some code/data space here, with a small time penalty
        if ((thrown_spell < 14 && thrown_spell > 6) || (thrown_spell == 16)) {
            msg_print(CONCAT(cdesc, " casts a spell."));
        }

        int y, x;

        // Cast the spell.
        switch (thrown_spell) {
        case 5: // Teleport Short
            teleport_away(monptr, 5);
            break;
        case 6: // Teleport Long
            teleport_away(monptr, MAX_SIGHT);
            break;
        case 7: // Teleport To
            teleport_to((int)m_ptr->fy, (int)m_ptr->fx);
            break;
        case 8: // Light Wound
            if (player_saves()) {
                msg_print("You resist the effects of the spell.");
            } else {
                take_hit(damroll(3, 8), ddesc);
            }
            break;
        case 9: // Serious Wound
            if (player_saves()) {
                msg_print("You resist the effects of the spell.");
            } else {
                take_hit(damroll(8, 8), ddesc);
            }
            break;
        case 10: // Hold Person
            if (player_never_paralyzed()) {
                msg_print("You are unaffected.");
            } else if (player_saves()) {
                msg_print("You resist the effects of the spell.");
            } else if (player_timed_in_force(PLAYER_TIMED_PARALYSIS)) {
                player_timed_add(PLAYER_TIMED_PARALYSIS, 2);
            } else {
                player_timed_set(PLAYER_TIMED_PARALYSIS, randint(5) + 4);
            }
            break;
        case 11: // Cause Blindness
            if (player_saves()) {
                msg_print("You resist the effects of the spell.");
            } else if (player_timed_in_force(PLAYER_TIMED_BLINDNESS)) {
                player_timed_add(PLAYER_TIMED_BLINDNESS, 6);
            } else {
                player_timed_add(PLAYER_TIMED_BLINDNESS, 12 + randint(3));
            }
            break;
        case 12: // Cause Confuse
            if (player_saves()) {
                msg_print("You resist the effects of the spell.");
            } else if (player_timed_in_force(PLAYER_TIMED_CONFUSION)) {
                player_timed_add(PLAYER_TIMED_CONFUSION, 2);
            } else {
                player_timed_set(PLAYER_TIMED_CONFUSION, randint(5) + 3);
            }
            break;
        case 13: // Cause Fear
            if (player_saves()) {
                msg_print("You resist the effects of the spell.");
            } else if (player_timed_in_force(PLAYER_TIMED_FEAR)) {
                player_timed_add(PLAYER_TIMED_FEAR, 2);
            } else {
                player_timed_set(PLAYER_TIMED_FEAR, randint(5) + 3);
            }
            break;
        case 14: // Summon Monster
            msg_print(CONCAT(cdesc, " magically summons a monster!"));
            y = player_row();
            x = player_col();

            // in case compact_monster() is called,it needs monptr
            monster_turn_begin(monptr);
            (void)summon_monster(&y, &x, false);
            monster_turn_end();
            update_mon((int)square_at(y, x)->cptr);
            break;
        case 15: // Summon Undead
            msg_print(CONCAT(cdesc, " magically summons an undead!"));
            y = player_row();
            x = player_col();

            // in case compact_monster() is called,it needs monptr
            monster_turn_begin(monptr);
            (void)summon_undead(&y, &x);
            monster_turn_end();
            update_mon((int)square_at(y, x)->cptr);
            break;
        case 16: // Slow Person
            if (player_never_paralyzed()) {
                msg_print("You are unaffected.");
            } else if (player_saves()) {
                msg_print("You resist the effects of the spell.");
            } else if (player_timed_in_force(PLAYER_TIMED_SLOWNESS)) {
                player_timed_add(PLAYER_TIMED_SLOWNESS, 2);
            } else {
                player_timed_set(PLAYER_TIMED_SLOWNESS, randint(5) + 3);
            }
            break;
        case 17: // Drain Mana
            if (player_mana() > 0) {
                disturb(1, 0);
                msg_print(CONCAT(cdesc, " draws psychic energy from you!"));
                if (m_ptr->ml) {
                    msg_print(CONCAT(cdesc, " appears healthier."));
                }

                int r1 = (randint((int)r_ptr->level) >> 1) + 1;

                // The monster is fed by what it actually got, which is less
                // than it drew for when the store runs dry.
                int drained = player_spend_mana(r1);
                prt_cmana();
                m_ptr->hp += 6 * drained;
            }
            break;
        case 20: // Breath Light
            msg_print(CONCAT(cdesc, " breathes lightning."));
            breath(GF_LIGHTNING, player_row(), player_col(), (m_ptr->hp / 4), ddesc, monptr);
            break;
        case 21: // Breath Gas
            msg_print(CONCAT(cdesc, " breathes gas."));
            breath(GF_POISON_GAS, player_row(), player_col(), (m_ptr->hp / 3), ddesc, monptr);
            break;
        case 22: // Breath Acid
            msg_print(CONCAT(cdesc, " breathes acid."));
            breath(GF_ACID, player_row(), player_col(), (m_ptr->hp / 3), ddesc, monptr);
            break;
        case 23: // Breath Frost
            msg_print(CONCAT(cdesc, " breathes frost."));
            breath(GF_FROST, player_row(), player_col(), (m_ptr->hp / 3), ddesc, monptr);
            break;
        case 24: // Breath Fire
            msg_print(CONCAT(cdesc, " breathes fire."));
            breath(GF_FIRE, player_row(), player_col(), (m_ptr->hp / 3), ddesc, monptr);
            break;
        default:
            msg_print(CONCAT(cdesc, " cast unknown spell."));
        }
        // End of spells

        if (m_ptr->ml) {
            recall_update_spell(m_ptr->creature, 1U << (thrown_spell - 1));
            recall_increment_spell_chance(m_ptr->creature);
            if (player_is_dead()) {
                recall_increment_death(m_ptr->creature);
            }
        }
    }
}

// Places creature adjacent to given location -RAK-
// Rats and Flys are fun!
bool multiply_monster(int y, int x, creature_handle creature, int monptr) {
    int j, k, result;
    cave_type *c_ptr;

    int i = 0;
    do {
        j = y - 2 + randint(3);
        k = x - 2 + randint(3);

        // don't create a new creature on top of the old one, that
        // causes invincible/invisible creatures to appear.
        if (in_bounds(j, k) && (j != y || k != x)) {
            c_ptr = square_at(j, k);
            if ((c_ptr->fval <= MAX_OPEN_SPACE) && (c_ptr->tptr == 0) && (c_ptr->cptr != 1)) {
                // Creature there already?
                if (c_ptr->cptr > 1) {
                    // Some critters are cannibalistic!
                    if ((monster_get_creature(creature)->cmove & CM_EATS_OTHER)
                        // Check the experience level -CJS-
                        && monster_get_creature(creature)->mexp >= monster_get_creature(monster_list_at(c_ptr->cptr)->creature)->mexp) {
                        // It ate an already processed monster.Handle * normally.
                        if (monptr < c_ptr->cptr) {
                            delete_monster((int)c_ptr->cptr);
                        } else {
                            // If it eats this monster, an already processed
                            // mosnter will take its place, causing all kinds
                            // of havoc. Delay the kill a bit.
                            fix1_delete_monster((int)c_ptr->cptr);
                        }

                        // in case compact_monster() is called,it needs monptr
                        monster_turn_begin(monptr);

                        // Place_monster() may fail if monster list full.
                        result = place_monster(j, k, creature, false);
                        monster_turn_end();
                        if (!result) {
                            return false;
                        }
                        monster_breeding_note_birth();
                        return check_mon_lite(j, k);
                    }
                } else {
                    // All clear,  place a monster

                    // in case compact_monster() is called,it needs monptr
                    monster_turn_begin(monptr);

                    // Place_monster() may fail if monster list full.
                    result = place_monster(j, k, creature, false);
                    monster_turn_end();
                    if (!result) {
                        return false;
                    }
                    monster_breeding_note_birth();
                    return check_mon_lite(j, k);
                }
            }
        }
        i++;
    } while (i <= 18);
    return false;
}

// Move the critters about the dungeon -RAK-
static void mon_move(int monptr, uint32_t *rcmove) {
    int i, k;

    monster_type *m_ptr = monster_list_at(monptr);
    creature_type *r_ptr = monster_get_creature(m_ptr->creature);

    // Does the critter multiply?
    // rest could be negative, to be safe, only use mod with positive values.
    int rest_val = abs(player_rest_turns());

    if ((r_ptr->cmove & CM_MULTIPLY) && monster_breeding_allowed() && ((rest_val % MON_MULT_ADJ) == 0)) {
        k = 0;
        for (i = m_ptr->fy - 1; i <= m_ptr->fy + 1; i++) {
            for (int j = m_ptr->fx - 1; j <= m_ptr->fx + 1; j++) {
                if (in_bounds(i, j) && (square_at(i, j)->cptr > 1)) {
                    k++;
                }
            }
        }

        // can't call randint with a value of zero, increment
        // counter to allow creature multiplication.
        if (k == 0) {
            k++;
        }
        if ((k < 4) && (randint(k * MON_MULT_ADJ) == 1)) {
            if (multiply_monster((int)m_ptr->fy, (int)m_ptr->fx, m_ptr->creature, monptr)) {
                *rcmove |= CM_MULTIPLY;
            }
        }
    }

    int mm[9];
    bool move_test = false;

    // if in wall, must immediately escape to a clear area
    if (!(r_ptr->cmove & CM_PHASE) && (square_at(m_ptr->fy, m_ptr->fx)->fval >= MIN_CAVE_WALL)) {
        // If the monster is already dead, don't kill it again!
        // This can happen for monsters moving faster than the player. They
        // will get multiple moves, but should not if they die on the first
        // move.  This is only a problem for monsters stuck in rock.
        if (m_ptr->hp < 0) {
            return;
        }

        k = 0;
        int dir = 1;

        // Note direction of for loops matches direction of keypad from 1 to 9
        // Do not allow attack against the player.
        // Must cast fy-1 to signed int, so that a nagative value
        // of i will fail the comparison.
        for (i = m_ptr->fy + 1; i >= (m_ptr->fy - 1); i--) {
            for (int j = m_ptr->fx - 1; j <= m_ptr->fx + 1; j++) {
                if ((dir != 5) && (square_at(i, j)->fval <= MAX_OPEN_SPACE) &&
                    (square_at(i, j)->cptr != 1)) {
                    mm[k++] = dir;
                }
                dir++;
            }
        }

        if (k != 0) {
            // put a random direction first
            dir = randint(k) - 1;
            i = mm[0];
            mm[0] = mm[dir];
            mm[dir] = i;
            make_move(monptr, mm, rcmove);
            // this can only fail if mm[0] has a rune of protection
        }

        // if still in a wall, let it dig itself out, but also apply some more damage
        if (square_at(m_ptr->fy, m_ptr->fx)->fval >= MIN_CAVE_WALL) {
            // in case the monster dies, may need to callfix1_delete_monster()
            // instead of delete_monsters()
            monster_turn_begin(monptr);
            i = mon_take_hit(monptr, damroll(8, 8));
            monster_turn_end();
            if (i) {
                msg_print("You hear a scream muffled by rock!");
                prt_experience();
            } else {
                msg_print("A creature digs itself out from the rock!");
                (void)twall((int)m_ptr->fy, (int)m_ptr->fx, 1, 0);
            }
        }
        // monster movement finished
        return;
    } else if (m_ptr->confused) {
        // Creature is confused or undead turned?

        // Undead only get confused from turn undead, so they should flee
        if (r_ptr->cdefense & CD_UNDEAD) {
            get_moves(monptr, mm);
            mm[0] = 10 - mm[0];
            mm[1] = 10 - mm[1];
            mm[2] = 10 - mm[2];
            mm[3] = randint(9); // May attack only if cornered
            mm[4] = randint(9);
        } else {
            mm[0] = randint(9);
            mm[1] = randint(9);
            mm[2] = randint(9);
            mm[3] = randint(9);
            mm[4] = randint(9);
        }

        // don't move him if he is not supposed to move!
        if (!(r_ptr->cmove & CM_ATTACK_ONLY)) {
            make_move(monptr, mm, rcmove);
        }
        m_ptr->confused--;
        move_test = true;
    } else if (r_ptr->spells & CS_FREQ) {
        // Creature may cast a spell
        mon_cast_spell(monptr, &move_test);
    }

    if (!move_test) {
        if ((r_ptr->cmove & CM_75_RANDOM) && (randint(100) < 75)) {
            // 75% random movement
            mm[0] = randint(9);
            mm[1] = randint(9);
            mm[2] = randint(9);
            mm[3] = randint(9);
            mm[4] = randint(9);
            *rcmove |= CM_75_RANDOM;
            make_move(monptr, mm, rcmove);
        } else if ((r_ptr->cmove & CM_40_RANDOM) && (randint(100) < 40)) {
            // 40% random movement
            mm[0] = randint(9);
            mm[1] = randint(9);
            mm[2] = randint(9);
            mm[3] = randint(9);
            mm[4] = randint(9);
            *rcmove |= CM_40_RANDOM;
            make_move(monptr, mm, rcmove);
        } else if ((r_ptr->cmove & CM_20_RANDOM) && (randint(100) < 20)) {
            // 20% random movement
            mm[0] = randint(9);
            mm[1] = randint(9);
            mm[2] = randint(9);
            mm[3] = randint(9);
            mm[4] = randint(9);
            *rcmove |= CM_20_RANDOM;
            make_move(monptr, mm, rcmove);
        } else if (r_ptr->cmove & CM_MOVE_NORMAL) {
            // Normal movement
            if (randint(200) == 1) {
                mm[0] = randint(9);
                mm[1] = randint(9);
                mm[2] = randint(9);
                mm[3] = randint(9);
                mm[4] = randint(9);
            } else {
                get_moves(monptr, mm);
            }
            *rcmove |= CM_MOVE_NORMAL;
            make_move(monptr, mm, rcmove);
        } else if (r_ptr->cmove & CM_ATTACK_ONLY) {
            // Attack, but don't move
            if (m_ptr->cdis < 2) {
                get_moves(monptr, mm);
                make_move(monptr, mm, rcmove);
            } else {
                // Learn that the monster does does not move when
                // it should have moved, but didn't.
                *rcmove |= CM_ATTACK_ONLY;
            }
        } else if ((r_ptr->cmove & CM_ONLY_MAGIC) && (m_ptr->cdis < 2)) {
            // A little hack for Quylthulgs, so that one will eventually
            // notice that they have no physical attacks.
            if (recall_get(m_ptr->creature)->r_attacks[0] < MAX_UCHAR) {
                const uint8_t count = ++recall_get(m_ptr->creature)->r_attacks[0];
                // Another little hack for Quylthulgs, so that one can
                // eventually learn their speed.
                if (count > 20) {
                    recall_update_move(m_ptr->creature, CM_ONLY_MAGIC);
                }
            }
        }
    }
}

// Creatures movement and attacking are done from here -RAK-
void creatures(int attack) {
    int k;
    int notice;
    uint32_t rcmove;
    bool wake, ignore;
    monster_type *m_ptr;
    recall_type *r_ptr;
    vtype cdesc;

    // Process the monsters
    for (int i = monster_list_used() - 1; i >= MIN_MONIX && !player_is_dead(); i--) {
        m_ptr = monster_list_at(i);
        // Get rid of an eaten/breathed on monster.  Note: Be sure not to
        // process this monster. This is necessary because we can't delete
        // monsters while scanning the monster list here.
        if (m_ptr->hp < 0) {
            fix2_delete_monster(i);
            continue;
        }

        m_ptr->cdis = distance(player_row(), player_col(), (int)m_ptr->fy, (int)m_ptr->fx);

        // Attack is argument passed to CREATURE
        if (attack) {
            k = moves_this_turn(m_ptr->cspeed);
            if (k <= 0) {
                update_mon(i);
            } else {
                while (k > 0) {
                    k--;
                    wake = false;
                    ignore = false;
                    rcmove = 0;

                    // Monsters trapped in rock must be given a turn also,
                    // so that they will die/dig out immediately.
                    if (m_ptr->ml || (m_ptr->cdis <= monster_get_creature(m_ptr->creature)->aaf) || ((!(monster_get_creature(m_ptr->creature)->cmove & CM_PHASE)) && square_at(m_ptr->fy, m_ptr->fx)->fval >= MIN_CAVE_WALL)) {
                        if (m_ptr->csleep > 0) {
                            if (player_aggravates_monsters()) {
                                m_ptr->csleep = 0;
                            } else if ((!player_resting() && !player_timed_in_force(PLAYER_TIMED_PARALYSIS)) || (randint(50) == 1)) {
                                notice = randint(1024);
                                // Stealth ranges from -1 to 18, giving 20 steps. Each point halves
                                // the right-hand side. At stl = -1, the threshold 1L << 30 exactly
                                // equals the maximum of randint(1024) cubed, so the monster always
                                // wakes.
                                if (notice * notice * notice <= (1L << (29 - player_stealth()))) {
                                    m_ptr->csleep -= (100 / m_ptr->cdis);
                                    if (m_ptr->csleep > 0) {
                                        ignore = true;
                                    } else {
                                        wake = true;

                                        // force it to be exactly zero
                                        m_ptr->csleep = 0;
                                    }
                                }
                            }
                        }

                        if (m_ptr->stunned != 0) {
                            // NOTE: Balrog = 100*100 = 10000, it always recovers instantly
                            if (randint(5000) < monster_get_creature(m_ptr->creature)->level * monster_get_creature(m_ptr->creature)->level) {
                                m_ptr->stunned = 0;
                            } else {
                                m_ptr->stunned--;
                            }
                            if (m_ptr->stunned == 0) {
                                if (m_ptr->ml) {
                                    (void)sprintf(cdesc, "The %s ", monster_get_creature(m_ptr->creature)->name);
                                    msg_print(strcat(cdesc, "recovers and glares at you."));
                                }
                            }
                        }
                        if ((m_ptr->csleep == 0) && (m_ptr->stunned == 0)) {
                            mon_move(i, &rcmove);
                        }
                    }

                    update_mon(i);
                    if (m_ptr->ml) {
                        r_ptr = recall_get(m_ptr->creature);
                        if (wake) {
                            if (r_ptr->r_wake < MAX_UCHAR) {
                                r_ptr->r_wake++;
                            }
                        } else if (ignore) {
                            if (r_ptr->r_ignore < MAX_UCHAR) {
                                r_ptr->r_ignore++;
                            }
                        }
                        recall_update_move(m_ptr->creature, rcmove);
                    }
                }
            }
        } else {
            update_mon(i);
        }

        // Get rid of an eaten/breathed on monster. This is necessary because
        // we can't delete monsters while scanning the monster list here.
        // This monster may have been killed during mon_move().
        if (m_ptr->hp < 0) {
            fix2_delete_monster(i);
            continue;
        }
    }
    // End processing monsters
}
