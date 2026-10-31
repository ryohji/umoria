// Copyright (c) 1989-94 James E. Wilson, Robert A. Koeneke
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Handle object descriptions, mostly string handling code

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"
#include "game_status.h"
#include "world_item.h"
#include "world_internal.h"
#include "rnd.h"
#include "tables.h"
#include "treasure.h"
#include "ui_screen.h"
#include "item_kinds.h"

#include "inventory.h"
#include "item_knowledge.h"
#include "str_insert.h"

// Descriptive constants
#define MAX_TITLES               45   // Used with scrolls

static void unsample(inven_type *);

static char titles[MAX_TITLES][10];

// Object descriptor routines

// Initialize all Potions, wands, staves, scrolls, etc.
void magic_init(void) {
    int j;
    const char *tmp;

    set_seed(progress_color_seed());

    // The first 3 entries for colors are fixed, (slime & apple juice, water)
    for (int i = 3; i < MAX_COLORS; i++) {
        j = randint(MAX_COLORS - 3) + 2;
        tmp = colors[i];
        colors[i] = colors[j];
        colors[j] = tmp;
    }
    for (int i = 0; i < MAX_WOODS; i++) {
        j = randint(MAX_WOODS) - 1;
        tmp = woods[i];
        woods[i] = woods[j];
        woods[j] = tmp;
    }
    for (int i = 0; i < MAX_METALS; i++) {
        j = randint(MAX_METALS) - 1;
        tmp = metals[i];
        metals[i] = metals[j];
        metals[j] = tmp;
    }
    for (int i = 0; i < MAX_ROCKS; i++) {
        j = randint(MAX_ROCKS) - 1;
        tmp = rocks[i];
        rocks[i] = rocks[j];
        rocks[j] = tmp;
    }
    for (int i = 0; i < MAX_AMULETS; i++) {
        j = randint(MAX_AMULETS) - 1;
        tmp = amulets[i];
        amulets[i] = amulets[j];
        amulets[j] = tmp;
    }
    for (int i = 0; i < MAX_MUSH; i++) {
        j = randint(MAX_MUSH) - 1;
        tmp = mushrooms[i];
        mushrooms[i] = mushrooms[j];
        mushrooms[j] = tmp;
    }

    int k;
    vtype string;

    for (int h = 0; h < MAX_TITLES; h++) {
        string[0] = '\0';
        k = randint(2) + 1;
        for (int i = 0; i < k; i++) {
            for (int s = randint(2); s > 0; s--) {
                (void)strcat(string, syllables[randint(MAX_SYLLABLES) - 1]);
            }
            if (i < k - 1) {
                (void)strcat(string, " ");
            }
        }
        if (string[8] == ' ') {
            string[8] = '\0';
        } else {
            string[9] = '\0';
        }
        (void)strcpy(titles[h], string);
    }
    reset_seed();
}

// Remove "Secret" symbol for identity of object
void known1(inven_type *i_ptr) {
    item_kind_mark_known(i_ptr);
}

bool known1_p(inven_type *i_ptr) {
    // Items which don't have a 'color' are always known1,
    // so that they can be carried in order in the inventory.
    if (!item_kind_has_record(i_ptr)) {
        return true;
    }
    if (store_bought_p(i_ptr)) {
        return true;
    }

    return item_kind_is_known(i_ptr);
}

// Remove "Secret" symbol for identity of plusses
void known2(inven_type *i_ptr) {
    unsample(i_ptr);
    i_ptr->ident |= ID_KNOWN2;
}

bool known2_p(inven_type *i_ptr) {
    return (i_ptr->ident & ID_KNOWN2) != 0;
}

void clear_known2(inven_type *i_ptr) {
    i_ptr->ident &= ~ID_KNOWN2;
}

void clear_empty(inven_type *i_ptr) {
    i_ptr->ident &= ~ID_EMPTY;
}

void store_bought(inven_type *i_ptr) {
    i_ptr->ident |= ID_STOREBOUGHT;
    known2(i_ptr);
}

bool store_bought_p(inven_type *i_ptr) {
    return (i_ptr->ident & ID_STOREBOUGHT) != 0;
}

// Remove an automatically generated inscription. -CJS-
static void unsample(inven_type *i_ptr) {
    // used to clear ID_DAMD flag, but I think it should remain set
    i_ptr->ident &= ~(ID_MAGIK | ID_EMPTY);

    item_kind_clear_tried(i_ptr);
}

// Somethings been sampled -CJS-
void sample(inven_type *i_ptr) {
    item_kind_mark_tried(i_ptr);
}

// Somethings been identified.
// Extra complexity by CJS so that it can merge store/dungeon objects when appropriate.
void identify(int *item) {
    // The index crosses both windows: ident_spell() lets the player pick from
    // the whole index space, so *item can be a pack slot or an equipment slot.
    // The merging below only ever touches pack slots.
    inven_type *i_ptr = inventory_and_equipment_at(*item);

    if (i_ptr->flags & TR_CURSED) {
        add_inscribe(i_ptr, ID_DAMD);
    }

    if (!known1_p(i_ptr)) {
        known1(i_ptr);
        int x1 = i_ptr->tval;
        int x2 = i_ptr->subval;
        if (x2 < ITEM_SINGLE_STACK_MIN || x2 >= ITEM_GROUP_MIN) {
            ; // no merging possible
        } else {
            int j;
            inven_type *t_ptr;

            for (int i = 0; i < inventory_count(); i++) {
                t_ptr = inventory_at(i);
                if (t_ptr->tval == x1 && t_ptr->subval == x2 && i != *item &&
                    ((int)t_ptr->number + (int)i_ptr->number < 256)) {
                    // make *item the smaller number
                    if (*item > i) {
                        j = *item;
                        *item = i;
                        i = j;
                    }
                    msg_print("You combine similar objects from the shop and dungeon.");

                    inventory_at(*item)->number += inventory_at(i)->number;
                    inventory_set_count(inventory_count() - 1);
                    for (j = i; j < inventory_count(); j++) {
                        *inventory_at(j) = *inventory_at(j + 1);
                    }
                    invcopy(inventory_at(j), OBJ_NOTHING);
                }
            }
        }
    }
}

// If an object has lost magical properties,
// remove the appropriate portion of the name. -CJS-
void unmagic_name(inven_type *i_ptr) {
    i_ptr->name2 = SN_NULL;
}

// defines for p1_use, determine how the p1 field is printed
#define IGNORED   0
#define CHARGES   1
#define PLUSSES   2
#define LIGHT     3
#define FLAGS     4
#define Z_PLUSSES 5

// Substitute the material/color string into the "%s" placeholder in the template.
// Written directly to avoid -Wformat-nonliteral (format string is not a literal).
// If no "%s", copies the template as-is (same behavior as sprintf).
static void fill_in_modstr(char *out_val, const char *basenm, const char *modstr) {
    const char *hole = strstr(basenm, "%s");

    if (hole == NULL) {
        (void)strcpy(out_val, basenm);
        return;
    }

    size_t prefix_len = (size_t)(hole - basenm);
    (void)memcpy(out_val, basenm, prefix_len);
    (void)strcpy(out_val + prefix_len, modstr);
    (void)strcat(out_val + prefix_len, hole + 2);
}

// Returns a description of item for inventory
// pref indicates that there should be an article added (prefix).
// Note that since out_val can easily exceed 80 characters, objdes
// must always be called with a bigvtype as the first paramter.
// The pieces of a name before it is put together: the base name (a format when
// modstr is set), the flavour or book name to put in it, the damage or bow
// multiplier, how p1 is shown, and whether " of <kind>" follows.
typedef struct {
    const char *basenm;
    const char *modstr;
    vtype damstr;
    int p1_use;
    bool append_name;
} name_parts;

// Unknown kinds show their flavour (format fmt with flavour); known kinds show
// the plain base name followed by " of <kind>".
static void flavoured(name_parts *parts, bool unknown, const char *fmt, const char *flavour, const char *plain) {
    if (unknown) {
        parts->basenm = fmt;
        parts->modstr = flavour;
    } else {
        parts->basenm = plain;
        parts->append_name = true;
    }
}

// Fills parts for the item's kind. Returns false when the kind has a fixed
// name, which is then already in out_val.
static bool base_name(name_parts *parts, char *out_val, inven_type *i_ptr) {
    int tmp;

    int indexx = i_ptr->subval & (ITEM_SINGLE_STACK_MIN - 1);

    parts->basenm = object_list[i_ptr->index].name;
    parts->modstr = CNIL;
    parts->damstr[0] = '\0';
    parts->p1_use = IGNORED;
    parts->append_name = false;
    bool modify = (known1_p(i_ptr) ? false : true);

    switch (i_ptr->tval) {
    case TV_MISC: case TV_CHEST:
        break;
    case TV_SLING_AMMO: case TV_BOLT: case TV_ARROW:
        (void)sprintf(parts->damstr, " (%dd%d)", i_ptr->damage[0], i_ptr->damage[1]);
        break;
    case TV_LIGHT:
        parts->p1_use = LIGHT;
        break;
    case TV_SPIKE:
        break;
    case TV_BOW:
        if (i_ptr->p1 == 1 || i_ptr->p1 == 2) {
            tmp = 2;
        } else if (i_ptr->p1 == 3 || i_ptr->p1 == 5) {
            tmp = 3;
        } else if (i_ptr->p1 == 4 || i_ptr->p1 == 6) {
            tmp = 4;
        } else {
            tmp = -1;
        }
        (void)sprintf(parts->damstr, " (x%d)", tmp);
        break;
    case TV_HAFTED: case TV_POLEARM: case TV_SWORD:
        (void)sprintf(parts->damstr, " (%dd%d)", i_ptr->damage[0], i_ptr->damage[1]);
        parts->p1_use = FLAGS;
        break;
    case TV_DIGGING:
        parts->p1_use = Z_PLUSSES;
        (void)sprintf(parts->damstr, " (%dd%d)", i_ptr->damage[0], i_ptr->damage[1]);
        break;
    case TV_BOOTS: case TV_GLOVES: case TV_CLOAK: case TV_HELM:
    case TV_SHIELD: case TV_HARD_ARMOR: case TV_SOFT_ARMOR:
        break;
    case TV_AMULET:
        flavoured(parts, modify, "& %s Amulet", amulets[indexx], "& Amulet");
        parts->p1_use = PLUSSES;
        break;
    case TV_RING:
        flavoured(parts, modify, "& %s Ring", rocks[indexx], "& Ring");
        parts->p1_use = PLUSSES;
        break;
    case TV_STAFF:
        flavoured(parts, modify, "& %s Staff", woods[indexx], "& Staff");
        parts->p1_use = CHARGES;
        break;
    case TV_WAND:
        flavoured(parts, modify, "& %s Wand", metals[indexx], "& Wand");
        parts->p1_use = CHARGES;
        break;
    case TV_SCROLL1: case TV_SCROLL2:
        flavoured(parts, modify, "& Scroll~ titled \"%s\"", titles[indexx], "& Scroll~");
        break;
    case TV_POTION1: case TV_POTION2:
        flavoured(parts, modify, "& %s Potion~", colors[indexx], "& Potion~");
        break;
    case TV_FLASK:
        break;
    case TV_FOOD:
        if (modify) {
            if (indexx <= 15) {
                parts->basenm = "& %s Mushroom~";
            } else if (indexx <= 20) {
                parts->basenm = "& Hairy %s Mold~";
            }
            if (indexx <= 20) {
                parts->modstr = mushrooms[indexx];
            }
        } else {
            parts->append_name = true;
            if (indexx <= 15) {
                parts->basenm = "& Mushroom~";
            } else if (indexx <= 20) {
                parts->basenm = "& Hairy Mold~";
            } else {
                // Ordinary food does not have a name appended.
                parts->append_name = false;
            }
        }
        break;
    case TV_MAGIC_BOOK:
        parts->modstr = parts->basenm;
        parts->basenm = "& Book~ of Magic Spells %s";
        break;
    case TV_PRAYER_BOOK:
        parts->modstr = parts->basenm;
        parts->basenm = "& Holy Book~ of Prayers %s";
        break;
    case TV_OPEN_DOOR: case TV_CLOSED_DOOR: case TV_SECRET_DOOR: case TV_RUBBLE:
        break;
    case TV_GOLD:
    case TV_INVIS_TRAP: case TV_VIS_TRAP:
    case TV_UP_STAIR: case TV_DOWN_STAIR:
        (void)strcpy(out_val, object_list[i_ptr->index].name);
        (void)strcat(out_val, ".");
        return false;
    case TV_STORE_DOOR:
        (void)sprintf(out_val, "the entrance to the %s.", object_list[i_ptr->index].name);
        return false;
    default:
        (void)strcpy(out_val, "Error in objdes()");
        return false;
    }
    return true;
}

// Puts the parts together: the base name with its flavour, " of <kind>", and
// the plural (~ marks where an s goes). tmp_val is a bigvtype.
static void compose_name(char *tmp_val, const name_parts *parts, const inven_type *i_ptr) {
    if (parts->modstr != CNIL) {
        fill_in_modstr(tmp_val, parts->basenm, parts->modstr);
    } else {
        (void)strcpy(tmp_val, parts->basenm);
    }
    if (parts->append_name) {
        (void)strcat(tmp_val, " of ");
        (void)strcat(tmp_val, object_list[i_ptr->index].name);
    }
    if (i_ptr->number != 1) {
        insert_str(tmp_val, sizeof(bigvtype), "ch~", "ches");
        insert_str(tmp_val, sizeof(bigvtype), "~", "s");
    } else {
        insert_str(tmp_val, sizeof(bigvtype), "~", CNIL);
    }
}

// The name without article or details: "some" and the leading "& " go.
static void bare_name(char *out_val, const char *tmp_val) {
    if (!strncmp("some", tmp_val, 4)) {
        (void)strcpy(out_val, &tmp_val[5]);
    } else if (tmp_val[0] == '&') {
        // eliminate the '& ' at the beginning
        (void)strcpy(out_val, &tmp_val[2]);
    } else {
        (void)strcpy(out_val, tmp_val);
    }
}

// Appends the special name, the damage, the to-hit and to-damage, and the
// armour class.
static void append_bonuses(char *tmp_val, inven_type *i_ptr, const char *damstr) {
    vtype tmp_str;

    if (i_ptr->name2 != SN_NULL && known2_p(i_ptr)) {
        (void)strcat(tmp_val, " ");
        (void)strcat(tmp_val, special_names[i_ptr->name2]);
    }
    if (damstr[0] != '\0') {
        (void)strcat(tmp_val, damstr);
    }
    if (known2_p(i_ptr)) {
        // originally used %+d, but several machines don't support it
        if (item_shows_hit_dam(i_ptr)) {
            (void)sprintf(tmp_str, " (%c%d,%c%d)",
                          (i_ptr->tohit < 0) ? '-' : '+', abs(i_ptr->tohit),
                          (i_ptr->todam < 0) ? '-' : '+', abs(i_ptr->todam));
        } else if (i_ptr->tohit != 0) {
            (void)sprintf(tmp_str, " (%c%d)", (i_ptr->tohit < 0) ? '-' : '+', abs(i_ptr->tohit));
        } else if (i_ptr->todam != 0) {
            (void)sprintf(tmp_str, " (%c%d)", (i_ptr->todam < 0) ? '-' : '+', abs(i_ptr->todam));
        } else {
            tmp_str[0] = '\0';
        }
        (void)strcat(tmp_val, tmp_str);
    }
    // Crowns have a zero base AC, so make a special test for them.
    if (i_ptr->ac != 0 || (i_ptr->tval == TV_HELM)) {
        (void)sprintf(tmp_str, " [%d", i_ptr->ac);
        (void)strcat(tmp_val, tmp_str);
        if (known2_p(i_ptr)) {
            // originally used %+d, but several machines don't support it
            (void)sprintf(tmp_str, ",%c%d", (i_ptr->toac < 0) ? '-' : '+', abs(i_ptr->toac));
            (void)strcat(tmp_val, tmp_str);
        }
        (void)strcat(tmp_val, "]");
    } else if ((i_ptr->toac != 0) && known2_p(i_ptr)) {
        // originally used %+d, but several machines don't support it
        (void)sprintf(tmp_str, " [%c%d]", (i_ptr->toac < 0) ? '-' : '+', abs(i_ptr->toac));
        (void)strcat(tmp_val, tmp_str);
    }
}

// Appends what p1 means for this item (light, charges, plusses, which stat).
static void append_p1(char *tmp_val, inven_type *i_ptr, int p1_use) {
    vtype tmp_str;

    // override defaults, check for p1 flags in the ident field
    if (item_hides_p1(i_ptr)) {
        p1_use = IGNORED;
    } else if (item_shows_p1(i_ptr)) {
        p1_use = Z_PLUSSES;
    }
    tmp_str[0] = '\0';
    if (p1_use == LIGHT) {
        (void)sprintf(tmp_str, " with %d turns of light", i_ptr->p1);
    } else if (p1_use == IGNORED) {
        ;
    } else if (known2_p(i_ptr)) {
        if (p1_use == Z_PLUSSES) {
            // originally used %+d, but several machines don't support it
            (void)sprintf(tmp_str, " (%c%d)", (i_ptr->p1 < 0) ? '-' : '+', abs(i_ptr->p1));
        } else if (p1_use == CHARGES) {
            (void)sprintf(tmp_str, " (%d charges)", i_ptr->p1);
        } else if (i_ptr->p1 != 0) {
            if (p1_use == PLUSSES) {
                (void)sprintf(tmp_str, " (%c%d)", (i_ptr->p1 < 0) ? '-' : '+', abs(i_ptr->p1));
            } else if (p1_use == FLAGS) {
                if (i_ptr->flags & TR_STR) {
                    (void)sprintf(tmp_str, " (%c%d to STR)", (i_ptr->p1 < 0) ? '-' : '+', abs(i_ptr->p1));
                } else if (i_ptr->flags & TR_STEALTH) {
                    (void)sprintf(tmp_str, " (%c%d to stealth)", (i_ptr->p1 < 0) ? '-' : '+', abs(i_ptr->p1));
                }
            }
        }
    }
    (void)strcat(tmp_val, tmp_str);
}

// The article or the count in front: a, an, 2, "no more".
static void with_article(char *out_val, const char *tmp_val, const inven_type *i_ptr) {
    // ampersand is always the first character
    if (tmp_val[0] == '&') {
        // use &tmp_val[1], so that & does not appear in output
        if (i_ptr->number > 1) {
            (void)sprintf(out_val, "%d%s", (int)i_ptr->number, &tmp_val[1]);
        } else if (i_ptr->number < 1) {
            (void)sprintf(out_val, "%s%s", "no more", &tmp_val[1]);
        } else if (is_a_vowel(tmp_val[2])) {
            (void)sprintf(out_val, "an%s", &tmp_val[1]);
        } else {
            (void)sprintf(out_val, "a%s", &tmp_val[1]);
        }
    } else if (i_ptr->number < 1) {
        // handle 'no more' case specially

        // check for "some" at start
        if (!strncmp("some", tmp_val, 4)) {
            (void)sprintf(out_val, "no more %s", &tmp_val[5]);
        } else {
            // here if no article
            (void)sprintf(out_val, "no more %s", tmp_val);
        }
    } else {
        (void)strcpy(out_val, tmp_val);
    }
}

// Appends {tried magik empty damned <inscription>} and the full stop.
static void append_notes(char *out_val, inven_type *i_ptr) {
    vtype tmp_str;
    bigvtype tmp_val;
    int indexx;

    tmp_str[0] = '\0';

    // don't print tried string for store bought items
    if (item_kind_was_tried(i_ptr) && !store_bought_p(i_ptr)) {
        (void)strcat(tmp_str, "tried ");
    }
    if (item_has_any_note(i_ptr)) {
        if (item_noted_magical(i_ptr)) {
            (void)strcat(tmp_str, "magik ");
        }
        if (item_noted_empty(i_ptr)) {
            (void)strcat(tmp_str, "empty ");
        }
        if (item_noted_damned(i_ptr)) {
            (void)strcat(tmp_str, "damned ");
        }
    }
    if (i_ptr->inscrip[0] != '\0') {
        (void)strcat(tmp_str, i_ptr->inscrip);
    } else if ((indexx = (int)strlen(tmp_str)) > 0) {
        // remove the extra blank at the end
        tmp_str[indexx - 1] = '\0';
    }
    if (tmp_str[0]) {
        (void)sprintf(tmp_val, " {%s}", tmp_str);
        (void)strcat(out_val, tmp_val);
    }
    (void)strcat(out_val, ".");
}

void objdes(char *out_val, inven_type *i_ptr, int pref) {
    name_parts parts;
    if (!base_name(&parts, out_val, i_ptr)) {
        return;
    }

    bigvtype tmp_val;
    compose_name(tmp_val, &parts, i_ptr);
    if (!pref) {
        bare_name(out_val, tmp_val);
        return;
    }
    append_bonuses(tmp_val, i_ptr, parts.damstr);
    append_p1(tmp_val, i_ptr, parts.p1_use);
    with_article(out_val, tmp_val, i_ptr);
    append_notes(out_val, i_ptr);
}

// Describe number of remaining charges. -RAK-
void desc_charges(int item_val) {
    int rem_num;
    vtype out_val;

    if (known2_p(inventory_at(item_val))) {
        rem_num = inventory_at(item_val)->p1;
        (void)sprintf(out_val, "You have %d charges remaining.", rem_num);
        msg_print(out_val);
    }
}

// Describe amount of item remaining. -RAK-
void desc_remain(int item_val) {
    msgtype out_val;
    bigvtype tmp_str;

    inven_type *i_ptr = inventory_at(item_val);
    i_ptr->number--;
    objdes(tmp_str, i_ptr, true);
    i_ptr->number++;
    // the string already has a dot at the end.
    (void)snprintf(out_val, sizeof(out_val), "You have %s", tmp_str);
    msg_print(out_val);
}
