// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Haggling with a store owner: the comments, the insults, and reading and
// answering offers

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"
#include "stores.h"
#include "store_haggle.h"
#include "player_race.h"
#include "progress.h"
#include "stats.h"
#include "str_insert.h"

static const char *comment2a[3] = {
    "%A2 is my final offer; take it or leave it.",
    "I'll give you no more than %A2.",
    "My patience grows thin.  %A2 is final.",
};

static const char *comment2b[16] = {
    "%A1 for such a fine item?  HA!  No less than %A2.",
    "%A1 is an insult!  Try %A2 gold pieces.",
    "%A1?!?  You would rob my poor starving children?",
    "Why, I'll take no less than %A2 gold pieces.",
    "Ha!  No less than %A2 gold pieces.",
    "Thou knave!  No less than %A2 gold pieces.",
    "%A1 is far too little, how about %A2?",
    "I paid more than %A1 for it myself, try %A2.",
    "%A1?  Are you mad?!?  How about %A2 gold pieces?",
    "As scrap this would bring %A1.  Try %A2 in gold.",
    "May the fleas of 1000 orcs molest you.  I want %A2.",
    "My mother you can get for %A1, this costs %A2.",
    "May your chickens grow lips.  I want %A2 in gold!",
    "Sell this for such a pittance?  Give me %A2 gold.",
    "May the Balrog find you tasty!  %A2 gold pieces?",
    "Your mother was a Troll!  %A2 or I'll tell.",
};

static const char *comment3a[3] = {
    "I'll pay no more than %A1; take it or leave it.",
    "You'll get no more than %A1 from me.",
    "%A1 and that's final.",
};

static const char *comment3b[15] = {
    "%A2 for that piece of junk?  No more than %A1.",
    "For %A2 I could own ten of those.  Try %A1.",
    "%A2?  NEVER!  %A1 is more like it.",
    "Let's be reasonable. How about %A1 gold pieces?",
    "%A1 gold for that junk, no more.",
    "%A1 gold pieces and be thankful for it!",
    "%A1 gold pieces and not a copper more.",
    "%A2 gold?  HA!  %A1 is more like it.",
    "Try about %A1 gold.",
    "I wouldn't pay %A2 for your children, try %A1.",
    "*CHOKE* For that!?  Let's say %A1.",
    "How about %A1?",
    "That looks war surplus!  Say %A1 gold.",
    "I'll buy it as scrap for %A1.",
    "%A2 is too much, let us say %A1 gold.",
};

static const char *comment4a[5] = {
    "ENOUGH!  You have abused me once too often!",
    "THAT DOES IT!  You shall waste my time no more!",
    "This is getting nowhere.  I'm going home!",
    "BAH!  No more shall you insult me!",
    "Begone!  I have had enough abuse for one day.",
};

static const char *comment4b[5] = {
    "Out of my place!", "out... Out... OUT!!!",
    "Come back tomorrow.", "Leave my place.  Begone!",
    "Come back when thou art richer.",
};

static const char *comment5[10] = {
    "You will have to do better than that!",
    "That's an insult!",
    "Do you wish to do business or not?",
    "Hah!  Try again.",
    "Ridiculous!",
    "You've got to be kidding!",
    "You'd better be kidding!",
    "You try my patience.",
    "I don't hear you.",
    "Hmmm, nice weather we're having.",
};

static const char *comment6[5] = {
    "I must have heard you wrong.", "What was that?",
    "I'm sorry, say that again.", "What did you say?",
    "Sorry, what was that again?",
};

// Comments vary. -RAK-
// Number of elements of a comment table. Keeps the count next to the
// array it belongs to, so the size is no longer written out twice.
#define comment_count(table) ((int)(sizeof(table) / sizeof((table)[0])))

// Pick one haggling comment and show it with the two numbers filled in.
// final > 0 selects the "final offer" table, otherwise the ordinary one.
// The meaning of the two numbers depends on the caller: the buyer and the
// seller swap their roles, so they are named after the placeholder they
// land in rather than after offer / asking.
static void prt_haggle_comment(const char **final_comments, int final_comment_count,
                               const char **normal_comments, int normal_comment_count,
                               int32_t a1, int32_t a2, int final) {
    vtype comment;

    if (final > 0) {
        (void)strcpy(comment, final_comments[randint(final_comment_count) - 1]);
    } else {
        (void)strcpy(comment, normal_comments[randint(normal_comment_count) - 1]);
    }

    insert_lnum(comment, "%A1", a1, false);
    insert_lnum(comment, "%A2", a2, false);
    msg_print(comment);
}

// %A1 is offer, %A2 is asking.
static void prt_comment2(int32_t offer, int32_t asking, int final) {
    prt_haggle_comment(comment2a, comment_count(comment2a),
                       comment2b, comment_count(comment2b),
                       offer, asking, final);
}

static void prt_comment3(int32_t offer, int32_t asking, int final) {
    prt_haggle_comment(comment3a, comment_count(comment3a),
                       comment3b, comment_count(comment3b),
                       offer, asking, final);
}

// Kick 'da bum out. -RAK-
static void prt_comment4(void) {
    int tmp = randint(5) - 1;
    msg_print(comment4a[tmp]);
    msg_print(comment4b[tmp]);
}

static void prt_comment5(void) {
    msg_print(comment5[randint(10) - 1]);
}

static void prt_comment6(void) {
    msg_print(comment6[randint(5) - 1]);
}

// Displays the set of commands -RAK-
static void haggle_commands(int typ) {
    if (typ == -1) {
        prt("Specify an asking-price in gold pieces.", 21, 0);
    } else {
        prt("Specify an offer in gold pieces.", 21, 0);
    }
    prt("ESC) Quit Haggling.", 22, 0);
    erase_line(23, 0); // clear last line
}

// Increase the insult counter and get angry if too many -RAK-
bool increase_insults(int store_num) {
    bool increase = false;

    store_type *s_ptr = store_at(store_num);
    s_ptr->insult_cur++;

    if (s_ptr->insult_cur > owners[s_ptr->owner].insult_max) {
        prt_comment4();
        s_ptr->insult_cur = 0;
        s_ptr->bad_buy++;
        s_ptr->store_open = progress_turn() + 2500 + randint(2500);
        increase = true;
    }

    return increase;
}

// Decrease insults -RAK-
void decrease_insults(int store_num) {
    store_type *s_ptr = store_at(store_num);

    if (s_ptr->insult_cur != 0) {
        s_ptr->insult_cur--;
    }
}

// Have insulted while haggling -RAK-
static bool haggle_insults(int store_num) {
    bool haggle = false;

    if (increase_insults(store_num)) {
        haggle = true;
    } else {
        prt_comment5();
        msg_print(CNIL); // keep insult separate from rest of haggle
    }

    return haggle;
}

// The last offer the player typed as an increment ("+50"), so that an empty
// line can repeat it. Only the haggling below ever looks at it.
static int16_t last_store_inc;

static bool get_haggle(const char *comment, int32_t *new_offer, int num_offer) {
    bool flag = true;
    bool increment = false;

    int clen = (int)strlen(comment);
    int orig_clen = clen;

    if (num_offer == 0) {
        last_store_inc = 0;
    }

    char *p;
    int32_t i = 0;
    vtype out_val, default_offer;

    do {
        prt(comment, 0, 0);
        if (num_offer && last_store_inc != 0) {
            (void)sprintf(default_offer, "[%c%d] ", (last_store_inc < 0) ? '-' : '+', abs(last_store_inc));
            prt(default_offer, 0, orig_clen);
            clen = orig_clen + (int)strlen(default_offer);
        }
        if (!get_string(out_val, 0, clen, 40)) {
            flag = false;
        }
        for (p = out_val; *p == ' '; p++) {
            ;
        }
        if (*p == '+' || *p == '-') {
            increment = true;
        }
        if (num_offer && increment) {
            i = (int32_t)atol(out_val);
            // Don't accept a zero here.  Turn off increment if it was zero
            // because a zero will not exit.  This can be zero if the user
            // did not type a number after the +/- sign.
            if (i == 0) {
                increment = false;
            } else {
                last_store_inc = i;
            }
        } else if (num_offer && *out_val == '\0') {
            i = last_store_inc;
            increment = true;
        } else {
            i = (int32_t)atol(out_val);
        }

        // don't allow incremental haggling, if player has not made an offer yet
        if (flag && num_offer == 0 && increment) {
            msg_print("You haven't even made your first offer yet!");
            i = 0;
            increment = false;
        }
    } while (flag && (i == 0));

    if (flag) {
        if (increment) {
            *new_offer += i;
        } else {
            *new_offer = i;
        }
    } else {
        erase_line(0, 0);
    }

    return flag;
}

static int receive_offer(int store_num, const char *comment, int32_t *new_offer, int32_t last_offer, int num_offer, int factor) {
    int receive = HAGGLE_AGREED;

    bool flag = false;
    do {
        if (get_haggle(comment, new_offer, num_offer)) {
            if (*new_offer * factor >= last_offer * factor) {
                flag = true;
            } else if (haggle_insults(store_num)) {
                receive = HAGGLE_INSULTED;
                flag = true;
            } else {
                // new_offer rejected, reset new_offer so that incremental
                // haggling works correctly
                *new_offer = last_offer;
            }
        } else {
            receive = HAGGLE_CANCELLED;
            flag = true;
        }
    } while (!flag);

    return receive;
}

// Haggling routine -RAK-
int purchase_haggle(int store_num, int32_t *price, inven_type *item) {
    bool flag = false;
    bool didnt_haggle = false;

    *price = 0;
    int purchase = HAGGLE_AGREED;
    int final_flag = 0;

    store_type *s_ptr = store_at(store_num);
    owner_type *o_ptr = &owners[s_ptr->owner];

    int32_t max_sell, min_sell;
    int32_t cost = sell_price(store_num, &max_sell, &min_sell, item);

    max_sell = max_sell * chr_adj() / 100;
    if (max_sell <= 0) {
        max_sell = 1;
    }

    min_sell = min_sell * chr_adj() / 100;
    if (min_sell <= 0) {
        min_sell = 1;
    }

    // cast max_inflate to signed so that subtraction works correctly
    int32_t max_buy = cost * (200 - (int)o_ptr->max_inflate) / 100;
    if (max_buy <= 0) {
        max_buy = 1;
    }

    int32_t min_per = o_ptr->haggle_per;
    int32_t max_per = min_per * 3;

    haggle_commands(1);

    int32_t cur_ask = max_sell;
    int32_t final_ask = min_sell;
    int32_t min_offer = max_buy;
    int32_t last_offer = min_offer;
    int32_t new_offer = 0;
    int num_offer = 0; // this prevents incremental haggling on first try
    const char *comment = "Asking";

    // go right to final price if player has bargained well
    if (noneedtobargain(store_num, final_ask)) {
        msg_print("After a long bargaining session, you agree upon the price.");
        cur_ask = min_sell;
        comment = "Final offer";
        didnt_haggle = true;

        // Set up automatic increment, so that a return will accept the final price.
        last_store_inc = min_sell;
        num_offer = 1;
    }

    vtype out_val;

    do {
        bool loop_flag;
        do {
            loop_flag = true;

            (void)sprintf(out_val, "%s :  %d", comment, cur_ask);
            put_buffer(out_val, 1, 0);

            purchase = receive_offer(store_num, "What do you offer? ", &new_offer, last_offer, num_offer, 1);
            if (purchase != HAGGLE_AGREED) {
                flag = true;
            } else {
                if (new_offer > cur_ask) {
                    prt_comment6();
                    // rejected, reset new_offer for incremental haggling
                    new_offer = last_offer;

                    // If the automatic increment is large enough to overflow,
                    // then the player must have made a mistake.  Clear it
                    // because it is useless.
                    if (last_offer + last_store_inc > cur_ask) {
                        last_store_inc = 0;
                    }
                } else if (new_offer == cur_ask) {
                    flag = true;
                    *price = new_offer;
                } else {
                    loop_flag = false;
                }
            }
        } while (!flag && loop_flag);

        if (!flag) {
            int32_t x1 = (new_offer - last_offer) * 100 / (cur_ask - last_offer);
            if (x1 < min_per) {
                flag = haggle_insults(store_num);
                if (flag) {
                    purchase = HAGGLE_INSULTED;
                }
            } else if (x1 > max_per) {
                x1 = x1 * 75 / 100;
                if (x1 < max_per) {
                    x1 = max_per;
                }
            }
            int32_t x2 = x1 + randint(5) - 3;
            int32_t x3 = ((cur_ask - new_offer) * x2 / 100) + 1;

            // don't let the price go up
            if (x3 < 0) {
                x3 = 0;
            }
            cur_ask -= x3;
            if (cur_ask < final_ask) {
                cur_ask = final_ask;
                comment = "Final Offer";

                // Set the automatic haggle increment so that RET will give
                // a new_offer equal to the final_ask price.
                last_store_inc = final_ask - new_offer;
                final_flag++;
                if (final_flag > HAGGLE_FINAL_OFFER_LIMIT) {
                    if (increase_insults(store_num)) {
                        purchase = HAGGLE_INSULTED;
                    } else {
                        purchase = HAGGLE_CANCELLED;
                    }
                    flag = true;
                }
            } else if (new_offer >= cur_ask) {
                flag = true;
                *price = new_offer;
            }
            if (!flag) {
                last_offer = new_offer;
                num_offer++; // enable incremental haggling
                erase_line(1, 0);
                (void)sprintf(out_val, "Your last offer : %d", last_offer);
                put_buffer(out_val, 1, 39);
                prt_comment2(last_offer, cur_ask, final_flag);

                // If the current increment would take you over the store's
                // price, then decrease it to an exact match.
                if (cur_ask - last_offer < last_store_inc) {
                    last_store_inc = cur_ask - last_offer;
                }
            }
        }
    } while (!flag);

    // update bargaining info
    if ((purchase == HAGGLE_AGREED) && (!didnt_haggle)) {
        updatebargain(store_num, *price, final_ask);
    }

    return purchase;
}

// Haggling routine -RAK-
int sell_haggle(int store_num, int32_t *price, inven_type *item) {
    int32_t max_gold = 0;
    int32_t min_per = 0;
    int32_t max_per = 0;
    int32_t max_sell = 0;
    int32_t min_buy = 0;
    int32_t max_buy = 0;

    bool flag = false;
    bool didnt_haggle = false;

    *price = 0;
    int sell = HAGGLE_AGREED;
    int final_flag = 0;

    store_type *s_ptr = store_at(store_num);

    int32_t cost = item_value(item);
    if (cost < 1) {
        sell = HAGGLE_WORTHLESS;
        flag = true;
    } else {
        owner_type *o_ptr = &owners[s_ptr->owner];

        cost = cost * (200 - chr_adj()) / 100;
        // Adjust by race pairing table (inverse direction from buy price).
        cost = cost * (200 - rgold_adj[o_ptr->owner_race][player_race()]) / 100;
        if (cost < 1) {
            cost = 1;
        }
        max_sell = cost * o_ptr->max_inflate / 100;

        // cast max_inflate to signed so that subtraction works correctly
        max_buy = cost * (200 - (int)o_ptr->max_inflate) / 100;
        min_buy = cost * (200 - o_ptr->min_inflate) / 100;
        if (min_buy < 1) {
            min_buy = 1;
        }
        if (max_buy < 1) {
            max_buy = 1;
        }
        if (min_buy < max_buy) {
            min_buy = max_buy;
        }
        min_per = o_ptr->haggle_per;
        max_per = min_per * 3;
        max_gold = o_ptr->max_cost;
    }

    int32_t cur_ask;
    int32_t final_ask = 0;
    const char *comment;

    if (!flag) {
        haggle_commands(-1);

        int num_offer = 0; // this prevents incremental haggling on first try

        if (max_buy > max_gold) {
            final_flag = 1;
            comment = "Final Offer";

            // Disable the automatic haggle increment on RET.
            last_store_inc = 0;
            cur_ask = max_gold;
            final_ask = max_gold;
            msg_print("I am sorry, but I have not the money to afford such a fine item.");
            didnt_haggle = true;
        } else {
            cur_ask = max_buy;
            final_ask = min_buy;
            if (final_ask > max_gold) {
                final_ask = max_gold;
            }
            comment = "Offer";

            // go right to final price if player has bargained well
            if (noneedtobargain(store_num, final_ask)) {
                msg_print("After a long bargaining session, you agree upon the price.");
                cur_ask = final_ask;
                comment = "Final offer";
                didnt_haggle = true;

                // Set up automatic increment, so that a return
                // will accept the final price.
                last_store_inc = final_ask;
                num_offer = 1;
            }
        }

        int32_t min_offer = max_sell;
        int32_t last_offer = min_offer;
        int32_t new_offer = 0;

        if (cur_ask < 1) {
            cur_ask = 1;
        }

        do {
            bool loop_flag;
            do {
                loop_flag = true;

                vtype out_val;
                (void)sprintf(out_val, "%s :  %d", comment, cur_ask);
                put_buffer(out_val, 1, 0);
                sell = receive_offer(store_num, "What price do you ask? ", &new_offer, last_offer, num_offer, -1);
                if (sell != HAGGLE_AGREED) {
                    flag = true;
                } else {
                    if (new_offer < cur_ask) {
                        prt_comment6();

                        // rejected, reset new_offer for incremental haggling
                        new_offer = last_offer;

                        // If the automatic increment is large enough to
                        // overflow, then the player must have made a mistake.
                        // Clear it because it is useless.
                        if (last_offer + last_store_inc < cur_ask) {
                            last_store_inc = 0;
                        }
                    } else if (new_offer == cur_ask) {
                        flag = true;
                        *price = new_offer;
                    } else {
                        loop_flag = false;
                    }
                }
            } while (!flag && loop_flag);

            if (!flag) {
                int32_t x1 = (last_offer - new_offer) * 100 / (last_offer - cur_ask);

                if (x1 < min_per) {
                    flag = haggle_insults(store_num);
                    if (flag) {
                        sell = HAGGLE_INSULTED;
                    }
                } else if (x1 > max_per) {
                    x1 = x1 * 75 / 100;
                    if (x1 < max_per) {
                        x1 = max_per;
                    }
                }
                int32_t x2 = x1 + randint(5) - 3;
                int32_t x3 = ((new_offer - cur_ask) * x2 / 100) + 1;

                // don't let the price go down
                if (x3 < 0) {
                    x3 = 0;
                }

                cur_ask += x3;
                if (cur_ask > final_ask) {
                    cur_ask = final_ask;
                    comment = "Final Offer";

                    // Set the automatic haggle increment so that RET will give
                    // a new_offer equal to the final_ask price.
                    last_store_inc = final_ask - new_offer;
                    final_flag++;
                    if (final_flag > HAGGLE_FINAL_OFFER_LIMIT) {
                        if (increase_insults(store_num)) {
                            sell = HAGGLE_INSULTED;
                        } else {
                            sell = HAGGLE_CANCELLED;
                        }
                        flag = true;
                    }
                } else if (new_offer <= cur_ask) {
                    flag = true;
                    *price = new_offer;
                }

                if (!flag) {
                    last_offer = new_offer;
                    num_offer++; // enable incremental haggling
                    erase_line(1, 0);

                    vtype out_val;
                    (void)sprintf(out_val, "Your last bid %d", last_offer);
                    put_buffer(out_val, 1, 39);
                    prt_comment3(cur_ask, last_offer, final_flag);

                    // If the current decrement would take you under the store's
                    // price, then increase it to an exact match.
                    if (cur_ask - last_offer > last_store_inc) {
                        last_store_inc = cur_ask - last_offer;
                    }
                }
            }
        } while (!flag);
    }

    // update bargaining info
    if ((sell == HAGGLE_AGREED) && (!didnt_haggle)) {
        updatebargain(store_num, *price, final_ask);
    }

    return sell;
}
