// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Monsters attacking the player in melee

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"

#include "equipment.h"
#include "inventory.h"
#include "monster_list.h"
#include "player_abilities.h"
#include "player_armour_class.h"
#include "player_glowing_hands.h"
#include "player_gold.h"
#include "player_level.h"
#include "player_timed_effects.h"
#include "score_death.h"

// Make an attack on the player (chuckle.) -RAK-
void make_attack(int monptr) {
    // don't beat a dead body!
    if (player_is_dead()) {
        return;
    }

    monster_type *m_ptr = monster_list_at(monptr);
    creature_type *r_ptr = monster_get_creature(m_ptr->creature);

    const char *cdesc = monster_name((vtype){0}, m_ptr);

    // For "DIED_FROM" string
    vtype ddesc;
    monster_name_indefinite(ddesc, r_ptr);

    int i, j, damage;
    int32_t gold;
    inven_type *i_ptr;

    const attack_handle *iter = r_ptr->attack;
    for (; iter != END_OF(r_ptr->attack) && !monster_attack_is_null(*iter) && !player_is_dead(); iter += 1) {
        const int attackn = iter - r_ptr->attack;
        int attype = monster_attack_get_type(*iter);
        int adesc = monster_attack_get_desc(*iter);
        const int adice = monster_attack_get_dice(*iter);
        const int asides = monster_attack_get_sides(*iter);

        bool flag = false;
        if (player_timed_in_force(PLAYER_TIMED_PROTECTION_FROM_EVIL) && (r_ptr->cdefense & CD_EVIL) &&
            ((player_level() + 1) > r_ptr->level)) {
            if (m_ptr->ml) {
                recall_update_characteristics(m_ptr->creature, CD_EVIL);
            }
            attype = 99;
            adesc = 99;
        }

        switch (attype) {
        case 1: // Normal attack
            if (test_hit(60, (int)r_ptr->level, 0, player_armour_class(), CLA_MISC_HIT)) {
                flag = true;
            }
            break;
        case 2: // Lose Strength
            if (test_hit(-3, (int)r_ptr->level, 0, player_armour_class(), CLA_MISC_HIT)) {
                flag = true;
            }
            break;
        case 3: // Confusion attack
            if (test_hit(10, (int)r_ptr->level, 0, player_armour_class(), CLA_MISC_HIT)) {
                flag = true;
            }
            break;
        case 4: // Fear attack
            if (test_hit(10, (int)r_ptr->level, 0, player_armour_class(), CLA_MISC_HIT)) {
                flag = true;
            }
            break;
        case 5: // Fire attack
            if (test_hit(10, (int)r_ptr->level, 0, player_armour_class(), CLA_MISC_HIT)) {
                flag = true;
            }
            break;
        case 6: // Acid attack
            if (test_hit(0, (int)r_ptr->level, 0, player_armour_class(), CLA_MISC_HIT)) {
                flag = true;
            }
            break;
        case 7: // Cold attack
            if (test_hit(10, (int)r_ptr->level, 0, player_armour_class(), CLA_MISC_HIT)) {
                flag = true;
            }
            break;
        case 8: // Lightning attack
            if (test_hit(10, (int)r_ptr->level, 0, player_armour_class(), CLA_MISC_HIT)) {
                flag = true;
            }
            break;
        case 9: // Corrosion attack
            if (test_hit(0, (int)r_ptr->level, 0, player_armour_class(), CLA_MISC_HIT)) {
                flag = true;
            }
            break;
        case 10: // Blindness attack
            if (test_hit(2, (int)r_ptr->level, 0, player_armour_class(), CLA_MISC_HIT)) {
                flag = true;
            }
            break;
        case 11: // Paralysis attack
            if (test_hit(2, (int)r_ptr->level, 0, player_armour_class(), CLA_MISC_HIT)) {
                flag = true;
            }
            break;
        case 12: // Steal Money
            if ((test_hit(5, (int)r_ptr->level, 0, (int)player_level(), CLA_MISC_HIT)) && (player_gold() > 0)) {
                flag = true;
            }
            break;
        case 13: // Steal Object
            if ((test_hit(2, (int)r_ptr->level, 0, (int)player_level(), CLA_MISC_HIT)) && (inventory_count() > 0)) {
                flag = true;
            }
            break;
        case 14: // Poison
            if (test_hit(5, (int)r_ptr->level, 0, player_armour_class(), CLA_MISC_HIT)) {
                flag = true;
            }
            break;
        case 15: // Lose dexterity
            if (test_hit(0, (int)r_ptr->level, 0, player_armour_class(), CLA_MISC_HIT)) {
                flag = true;
            }
            break;
        case 16: // Lose constitution
            if (test_hit(0, (int)r_ptr->level, 0, player_armour_class(), CLA_MISC_HIT)) {
                flag = true;
            }
            break;
        case 17: // Lose intelligence
            if (test_hit(2, (int)r_ptr->level, 0, player_armour_class(), CLA_MISC_HIT)) {
                flag = true;
            }
            break;
        case 18: // Lose wisdom
            if (test_hit(2, (int)r_ptr->level, 0, player_armour_class(), CLA_MISC_HIT)) {
                flag = true;
            }
            break;
        case 19: // Lose experience
            if (test_hit(5, (int)r_ptr->level, 0, player_armour_class(), CLA_MISC_HIT)) {
                flag = true;
            }
            break;
        case 20: // Aggravate monsters
            flag = true;
            break;
        case 21: // Disenchant
            if (test_hit(20, (int)r_ptr->level, 0, player_armour_class(), CLA_MISC_HIT)) {
                flag = true;
            }
            break;
        case 22: // Eat food
            if (test_hit(5, (int)r_ptr->level, 0, player_armour_class(), CLA_MISC_HIT)) {
                flag = true;
            }
            break;
        case 23: // Eat light
            if (test_hit(5, (int)r_ptr->level, 0, player_armour_class(), CLA_MISC_HIT)) {
                flag = true;
            }
            break;
        case 24: // Eat charges
            // check to make sure an object exists
            if ((test_hit(15, (int)r_ptr->level, 0, player_armour_class(), CLA_MISC_HIT)) &&
                (inventory_count() > 0)) {
                flag = true;
            }
            break;
        case 99:
            flag = true;
            break;
        default:
            break;
        }

        if (flag) {
            // can not strcat to cdesc because the creature may have multiple attacks.
            disturb(1, 0);
            switch (adesc) {
            case 1:
                msg_print(CONCAT(cdesc, " hits you."));
                break;
            case 2:
                msg_print(CONCAT(cdesc, " bites you."));
                break;
            case 3:
                msg_print(CONCAT(cdesc, " claws you."));
                break;
            case 4:
                msg_print(CONCAT(cdesc, " stings you."));
                break;
            case 5:
                msg_print(CONCAT(cdesc, " touches you."));
                break;
#if 0
            case 6:
                msg_print(CONCAT(cdesc, " kicks you."));
                break;
#endif
            case 7:
                msg_print(CONCAT(cdesc, " gazes at you."));
                break;
            case 8:
                msg_print(CONCAT(cdesc, " breathes on you."));
                break;
            case 9:
                msg_print(CONCAT(cdesc, " spits on you."));
                break;
            case 10:
                msg_print(CONCAT(cdesc, " makes a horrible wail."));
                break;
#if 0
            case 11:
                msg_print(CONCAT(cdesc, " embraces you."));
                break;
#endif
            case 12:
                msg_print(CONCAT(cdesc, " crawls on you."));
                break;
            case 13:
                msg_print(CONCAT(cdesc, " releases a cloud of spores."));
                break;
            case 14:
                msg_print(CONCAT(cdesc, " begs you for money."));
                break;
            case 15:
                msg_print("You've been slimed!");
                break;
            case 16:
                msg_print(CONCAT(cdesc, " crushes you."));
                break;
            case 17:
                msg_print(CONCAT(cdesc, " tramples you."));
                break;
            case 18:
                msg_print(CONCAT(cdesc, " drools on you."));
                break;
            case 19:
                switch (randint(9)) {
                case 1:
                    msg_print(CONCAT(cdesc, " insults you!"));
                    break;
                case 2:
                    msg_print(CONCAT(cdesc, " insults your mother!"));
                    break;
                case 3:
                    msg_print(CONCAT(cdesc, " gives you the finger!"));
                    break;
                case 4:
                    msg_print(CONCAT(cdesc, " humiliates you!"));
                    break;
                case 5:
                    msg_print(CONCAT(cdesc, " wets on your leg!"));
                    break;
                case 6:
                    msg_print(CONCAT(cdesc, " defiles you!"));
                    break;
                case 7:
                    msg_print(CONCAT(cdesc, " dances around you!"));
                    break;
                case 8:
                    msg_print(CONCAT(cdesc, " makes obscene gestures!"));
                    break;
                case 9:
                    msg_print(CONCAT(cdesc, " moons you!!!"));
                    break;
                }
                break;
            case 99:
                msg_print(CONCAT(cdesc, " is repelled."));
                break;
            default:
                break;
            }

            bool notice = true;
            bool visible = true;

            // always fail to notice attack if creature invisible, set notice
            // and visible here since creature may be visible when attacking
            // and then teleport afterwards (becoming effectively invisible)
            if (!m_ptr->ml) {
                visible = false;
                notice = false;
            } else {
                visible = true;
            }

            damage = damroll(adice, asides);

            switch (attype) {
            case 1: // Normal attack
                // round half-way case down
                damage -= (player_armour_class() * damage) / 200;
                take_hit(damage, ddesc);
                break;
            case 2: // Lose Strength
                take_hit(damage, ddesc);
                if (player_stat_sustained(A_STR)) {
                    msg_print("You feel weaker for a moment, but it passes.");
                } else if (randint(2) == 1) {
                    msg_print("You feel weaker.");
                    (void)dec_stat(A_STR);
                } else {
                    notice = false;
                }
                break;
            case 3: // Confusion attack
                take_hit(damage, ddesc);
                if (randint(2) == 1) {
                    if (!player_timed_in_force(PLAYER_TIMED_CONFUSION)) {
                        msg_print("You feel confused.");
                        player_timed_add(PLAYER_TIMED_CONFUSION, randint((int)r_ptr->level));
                    } else {
                        notice = false;
                    }
                    player_timed_add(PLAYER_TIMED_CONFUSION, 3);
                } else {
                    notice = false;
                }
                break;
            case 4: // Fear attack
                take_hit(damage, ddesc);
                if (player_saves()) {
                    msg_print("You resist the effects!");
                } else if (!player_timed_in_force(PLAYER_TIMED_FEAR)) {
                    msg_print("You are suddenly afraid!");
                    player_timed_add(PLAYER_TIMED_FEAR, 3 + randint((int)r_ptr->level));
                } else {
                    player_timed_add(PLAYER_TIMED_FEAR, 3);
                    notice = false;
                }
                break;
            case 5: // Fire attack
                msg_print("You are enveloped in flames!");
                fire_dam(damage, ddesc);
                break;
            case 6: // Acid attack
                msg_print("You are covered in acid!");
                acid_dam(damage, ddesc);
                break;
            case 7: // Cold attack
                msg_print("You are covered with frost!");
                cold_dam(damage, ddesc);
                break;
            case 8: // Lightning attack
                msg_print("Lightning strikes you!");
                light_dam(damage, ddesc);
                break;
            case 9: // Corrosion attack
                msg_print("A stinging red gas swirls about you.");
                corrode_gas(ddesc);
                take_hit(damage, ddesc);
                break;
            case 10: // Blindness attack
                take_hit(damage, ddesc);
                if (!player_timed_in_force(PLAYER_TIMED_BLINDNESS)) {
                    player_timed_add(PLAYER_TIMED_BLINDNESS, 10 + randint((int)r_ptr->level));
                    msg_print("Your eyes begin to sting.");
                } else {
                    player_timed_add(PLAYER_TIMED_BLINDNESS, 5);
                    notice = false;
                }
                break;
            case 11: // Paralysis attack
                take_hit(damage, ddesc);
                if (player_saves()) {
                    msg_print("You resist the effects!");
                } else if (!player_timed_in_force(PLAYER_TIMED_PARALYSIS)) {
                    if (player_never_paralyzed()) {
                        msg_print("You are unaffected.");
                    } else {
                        player_timed_set(PLAYER_TIMED_PARALYSIS, randint((int)r_ptr->level) + 3);
                        msg_print("You are paralyzed.");
                    }
                } else {
                    notice = false;
                }
                break;
            case 12: // Steal Money
                if (!player_timed_in_force(PLAYER_TIMED_PARALYSIS) &&
                    (randint(124) < py.stats.use_stat[A_DEX])) {
                    msg_print("You quickly protect your money pouch!");
                } else {
                    gold = (player_gold() / 10) + randint(25);
                    if (gold > player_gold()) {
                        player_set_gold(0);
                    } else {
                        player_pay_gold(gold);
                    }
                    msg_print("Your purse feels lighter.");
                    prt_gold();
                }
                if (randint(2) == 1) {
                    msg_print("There is a puff of smoke!");
                    teleport_away(monptr, MAX_SIGHT);
                }
                break;
            case 13: // Steal Object
                if (!player_timed_in_force(PLAYER_TIMED_PARALYSIS) &&
                    (randint(124) < py.stats.use_stat[A_DEX])) {
                    msg_print("You grab hold of your backpack!");
                } else {
                    i = randint(inventory_count()) - 1;
                    inven_destroy(i);
                    msg_print("Your backpack feels lighter.");
                }
                if (randint(2) == 1) {
                    msg_print("There is a puff of smoke!");
                    teleport_away(monptr, MAX_SIGHT);
                }
                break;
            case 14: // Poison
                take_hit(damage, ddesc);
                msg_print("You feel very sick.");
                player_timed_add(PLAYER_TIMED_POISON, randint((int)r_ptr->level) + 5);
                break;
            case 15: // Lose dexterity
                take_hit(damage, ddesc);
                if (player_stat_sustained(A_DEX)) {
                    msg_print("You feel clumsy for a moment, but it passes.");
                } else {
                    msg_print("You feel more clumsy.");
                    (void)dec_stat(A_DEX);
                }
                break;
            case 16: // Lose constitution
                take_hit(damage, ddesc);
                if (player_stat_sustained(A_CON)) {
                    msg_print("Your body resists the effects of the disease.");
                } else {
                    msg_print("Your health is damaged!");
                    (void)dec_stat(A_CON);
                }
                break;
            case 17: // Lose intelligence
                take_hit(damage, ddesc);
                msg_print("You have trouble thinking clearly.");
                if (player_stat_sustained(A_INT)) {
                    msg_print("But your mind quickly clears.");
                } else {
                    (void)dec_stat(A_INT);
                }
                break;
            case 18: // Lose wisdom
                take_hit(damage, ddesc);
                if (player_stat_sustained(A_WIS)) {
                    msg_print("Your wisdom is sustained.");
                } else {
                    msg_print("Your wisdom is drained.");
                    (void)dec_stat(A_WIS);
                }
                break;
            case 19: // Lose experience
                msg_print("You feel your life draining away!");
                lose_exp(damage + (player_experience() / 100) * MON_DRAIN_LIFE);
                break;
            case 20: // Aggravate monster
                (void)aggravate_monster(20);
                break;
            case 21: // Disenchant
                flag = false;
                switch (randint(7)) {
                case 1:
                    i = INVEN_WIELD;
                    break;
                case 2:
                    i = INVEN_BODY;
                    break;
                case 3:
                    i = INVEN_ARM;
                    break;
                case 4:
                    i = INVEN_OUTER;
                    break;
                case 5:
                    i = INVEN_HANDS;
                    break;
                case 6:
                    i = INVEN_HEAD;
                    break;
                case 7:
                    i = INVEN_FEET;
                    break;
                }
                i_ptr = equipment_at(i);

                if (i_ptr->tohit > 0) {
                    i_ptr->tohit -= randint(2);

                    // don't send it below zero
                    if (i_ptr->tohit < 0) {
                        i_ptr->tohit = 0;
                    }
                    flag = true;
                }
                if (i_ptr->todam > 0) {
                    i_ptr->todam -= randint(2);

                    // don't send it below zero
                    if (i_ptr->todam < 0) {
                        i_ptr->todam = 0;
                    }
                    flag = true;
                }
                if (i_ptr->toac > 0) {
                    i_ptr->toac -= randint(2);

                    // don't send it below zero
                    if (i_ptr->toac < 0) {
                        i_ptr->toac = 0;
                    }
                    flag = true;
                }
                if (flag) {
                    msg_print("There is a static feeling in the air.");
                    calc_bonuses();
                } else {
                    notice = false;
                }
                break;
            case 22: // Eat food
                if (find_range(TV_FOOD, TV_NEVER, &i, &j)) {
                    inven_destroy(i);
                    msg_print("It got at your rations!");
                } else {
                    notice = false;
                }
                break;
            case 23: // Eat light
                i_ptr = equipment_at(INVEN_LIGHT);
                if (i_ptr->p1 > 0) {
                    i_ptr->p1 -= (250 + randint(250));
                    if (i_ptr->p1 < 1) {
                        i_ptr->p1 = 1;
                    }
                    if (!player_timed_in_force(PLAYER_TIMED_BLINDNESS)) {
                        msg_print("Your light dims.");
                    } else {
                        notice = false;
                    }
                } else {
                    notice = false;
                }
                break;
            case 24: // Eat charges
                i = randint(inventory_count()) - 1;
                j = r_ptr->level;
                i_ptr = inventory_at(i);
                if (((i_ptr->tval == TV_STAFF) || (i_ptr->tval == TV_WAND)) &&
                    (i_ptr->p1 > 0)) {
                    m_ptr->hp += j * i_ptr->p1;
                    i_ptr->p1 = 0;
                    if (!known2_p(i_ptr)) {
                        add_inscribe(i_ptr, ID_EMPTY);
                    }
                    msg_print("Energy drains from your pack!");
                } else {
                    notice = false;
                }
                break;
            case 99:
                notice = false;
                break;
            default:
                notice = false;
                break;
            }

            // Moved here from mon_move, so that monster only confused if it
            // actually hits. A monster that has been repelled has not hit
            // the player, so it should not be confused.
            if (player_glowing_hands() && adesc != 99) {
                msg_print("Your hands stop glowing.");
                player_glowing_hands_spend();
                const char *verb;
                if ((randint(MAX_MONS_LEVEL) < r_ptr->level) || (CD_NO_SLEEP & r_ptr->cdefense)) {
                    verb = "is unaffected.";
                } else {
                    if (m_ptr->confused) {
                        m_ptr->confused += 3;
                    } else {
                        m_ptr->confused = 2 + randint(16);
                    }
                    verb = " appears confused.";
                }
                msg_print(CONCAT(cdesc, verb));
                if (visible && !player_is_dead() && randint(4) == 1) {
                    recall_update_characteristics(m_ptr->creature, CD_NO_SLEEP);
                }
            }

            // increase number of attacks if notice true, or if visible and
            // had previously noticed the attack (in which case all this does
            // is help player learn damage), note that in the second case do
            // not increase attacks if creature repelled (no damage done)
            if ((notice || (visible && recall_get(m_ptr->creature)->r_attacks[attackn] != 0 && attype != 99)) && recall_get(m_ptr->creature)->r_attacks[attackn] < MAX_UCHAR) {
                recall_get(m_ptr->creature)->r_attacks[attackn]++;
            }
            if (player_is_dead()) {
                recall_increment_death(m_ptr->creature);
            }
        } else {
            if ((adesc >= 1 && adesc <= 3) || (adesc == 6)) {
                disturb(1, 0);
                msg_print(CONCAT(cdesc, " misses you."));
            }
        }
    }
}
