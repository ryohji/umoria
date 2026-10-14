// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#include "config.h"
#include "constant.h"
#include "types.h"

#include "externs.h"
#include "equipment.h"
#include "armor_selection.h"

int pick_random_worn_armor(void) {
    int tmp[6];
    int k = 0;

    if (equipment_at(INVEN_BODY)->tval != TV_NOTHING) {
        tmp[k++] = INVEN_BODY;
    }
    if (equipment_at(INVEN_ARM)->tval != TV_NOTHING) {
        tmp[k++] = INVEN_ARM;
    }
    if (equipment_at(INVEN_OUTER)->tval != TV_NOTHING) {
        tmp[k++] = INVEN_OUTER;
    }
    if (equipment_at(INVEN_HANDS)->tval != TV_NOTHING) {
        tmp[k++] = INVEN_HANDS;
    }
    if (equipment_at(INVEN_HEAD)->tval != TV_NOTHING) {
        tmp[k++] = INVEN_HEAD;
    }
    if (equipment_at(INVEN_FEET)->tval != TV_NOTHING) {
        tmp[k++] = INVEN_FEET;
    }

    if (k > 0) {
        return tmp[randint(k) - 1];
    }
    return 0;
}

int pick_first_cursed_armor(void) {
    if (TR_CURSED & equipment_at(INVEN_BODY)->flags) {
        return INVEN_BODY;
    } else if (TR_CURSED & equipment_at(INVEN_ARM)->flags) {
        return INVEN_ARM;
    } else if (TR_CURSED & equipment_at(INVEN_OUTER)->flags) {
        return INVEN_OUTER;
    } else if (TR_CURSED & equipment_at(INVEN_HEAD)->flags) {
        return INVEN_HEAD;
    } else if (TR_CURSED & equipment_at(INVEN_HANDS)->flags) {
        return INVEN_HANDS;
    } else if (TR_CURSED & equipment_at(INVEN_FEET)->flags) {
        return INVEN_FEET;
    }
    return 0;
}
