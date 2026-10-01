// src/save/save.c: save and restore games and monster memory info
//
// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke,
//                         David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

#include "headers.h"

#include "config.h"
#include "constant.h"
#include "types.h"

#include "burden.h"
#include "externs.h"
#include "floor_items.h"
#include "dungeon_level.h"
#include "dungeon_map.h"
#include "dungeon_size.h"
#include "equipment.h"
#include "hp_table.h"
#include "input_ended.h"
#include "inventory.h"
#include "item_ident.h"
#include "missile_serial.h"
#include "monster_breeding.h"
#include "monster_list.h"
#include "panel.h"
#include "messages.h"
#include "player_abilities.h"
#include "player_armour_class.h"
#include "player_attack_bonuses.h"
#include "player_base_to_hit.h"
#include "player_bio.h"
#include "player_body_weight.h"
#include "player_class.h"
#include "player_disarm.h"
#include "player_display_numbers.h"
#include "player_food.h"
#include "player_glowing_hands.h"
#include "player_gold.h"
#include "player_hit_die.h"
#include "player_hp.h"
#include "player_infra_range.h"
#include "player_level.h"
#include "player_mana.h"
#include "player_max_depth.h"
#include "player_pos.h"
#include "player_race.h"
#include "player_resting.h"
#include "player_saving_throw.h"
#include "player_search_skill.h"
#include "player_speed.h"
#include "player_spells_to_learn.h"
#include "player_status_flags.h"
#include "player_stealth.h"
#include "player_timed_effects.h"
#include "options.h"
#include "progress.h"
#include "save_state.h"
#include "score_death.h"
#include "spells_known.h"
#include "stores.h"

// For debugging the savefile code on systems with broken compilers.
#define SAVE_LOG(x)

// Semicolon inside the macro argument: SAVE_LOG(x) expands to nothing, so a
// semicolon outside would leave a bare ';' at file scope (forbidden in ISO C).
SAVE_LOG(static FILE *logfile;)

static bool sv_write(void);
static void wr_byte(uint8_t);
static void wr_short(uint16_t);
static void wr_long(uint32_t);
static void wr_bytes(uint8_t *, int);
static void wr_string(const char *);
static void wr_shorts(uint16_t *, int);
static void wr_item(inven_type *);
static void wr_store(store_type *);
static void wr_monster(monster_type *);
static void rd_byte(uint8_t *);
static void rd_short(uint16_t *);
static void rd_bool(bool *);
static void rd_timed(player_timed_effect);
static void rd_long(uint32_t *);
static void rd_bytes(uint8_t *, int);
static void rd_string(char *);
static void rd_shorts(uint16_t *, int);
static void rd_item(inven_type *);
static bool rd_store(store_type *);
static void rd_monster(monster_type *);

// these are used for the save file, to avoid having to pass them to every procedure
static FILE *fileptr;
static uint8_t xor_byte;
static int from_savefile;   // can overwrite old savefile when save
static uint32_t start_time; // time that play started

// Dead 2-byte hole in the savefile. The game never uses this value: only the
// two lines below touch it. We preserve what we read so that load-then-save
// doesn't corrupt files written elsewhere. This is part of the file format,
// just like the 23 other shorts around it. Not worth a dedicated module: no
// one asks for it, and keeping it here is smallest.
static int16_t dead_protection_bytes;

// Whether a savefile written by 5.<version_min>.<patch_level> predates
// 5.<min>.<patch>. The major version has already been checked by then.
static bool savefile_before(uint8_t version_min, uint8_t patch_level, uint8_t min, uint8_t patch) {
    return version_min < min || (version_min == min && patch_level < patch);
}

// This save package was brought to by                -JWT-
// and                                                -RAK-
// and has been completely rewritten for UNIX by      -JEW-
// and has been completely rewritten again by         -CJS-
// and completely rewritten again! for portability by -JEW-

static bool sv_write(void) {
    // clear the death flag when creating a HANGUP save file,
    // so that player can see tombstone when restart
    if (input_has_ended()) {
        set_player_dead(false);
    }

    // The low eleven bits are the player's options; which option owns which
    // bit is stated once, in options.c.
    uint32_t l = game_options_pack();

    if (player_is_dead()) {
        // Sign bit
        l |= 0x80000000L;
    }
    if (player_has_won()) {
        l |= 0x40000000L;
    }

    for (int i = 0; i < MAX_CREATURES; i++) {
        recall_type *r_ptr = recall_get(monster_make_creature_handle(i));

        if (r_ptr->r_cmove || r_ptr->r_cdefense || r_ptr->r_kills ||
            r_ptr->r_spells || r_ptr->r_deaths || r_ptr->r_attacks[0] ||
            r_ptr->r_attacks[1] || r_ptr->r_attacks[2] || r_ptr->r_attacks[3]) {
            wr_short((uint16_t)i);
            wr_long(r_ptr->r_cmove);
            wr_long(r_ptr->r_spells);
            wr_short(r_ptr->r_kills);
            wr_short(r_ptr->r_deaths);
            wr_short(r_ptr->r_cdefense);
            wr_byte(r_ptr->r_wake);
            wr_byte(r_ptr->r_ignore);
            wr_bytes(r_ptr->r_attacks, MAX_MON_NATTACK);
        }
    }

    // sentinel to indicate no more monster info
    wr_short((uint16_t)0xFFFF);

    wr_long(l);

    // Player bio fields. Their order in the file cannot move: name and sex here,
    // age and height below, depth further down, history lines at the end.
    wr_string(player_name());
    // Write 1 or 0, not the raw byte. A hand-edited 2 will round-trip as 1.
    wr_byte((uint8_t)(player_is_male() ? 1 : 0));
    wr_long((uint32_t)player_gold());
    wr_long((uint32_t)player_max_experience());
    wr_long((uint32_t)player_experience());
    wr_short(player_experience_fraction());
    // Age first, then height. Order is part of the file format.
    wr_short((uint16_t)player_age());
    wr_short((uint16_t)player_height());
    wr_short((uint16_t)player_body_weight());
    wr_short(player_level());
    wr_short((uint16_t)player_max_depth());
    // Search: chance first, then frequency. Order cannot move.
    wr_short((uint16_t)player_search_chance());
    wr_short((uint16_t)player_search_frequency());
    // Base to-hit: melee first, then bows. Order cannot move.
    wr_short((uint16_t)player_base_to_hit());
    wr_short((uint16_t)player_base_to_hit_with_bows());
    wr_short((uint16_t)player_max_mana());
    wr_short((uint16_t)player_max_hp());
    // Attack bonuses: to-hit first, then to-dam. Order cannot move.
    wr_short((uint16_t)player_to_hit_bonus());
    wr_short((uint16_t)player_to_damage_bonus());
    // Armor class: two parts in the file. Armor first, then magical. Order
    // cannot move.
    wr_short((uint16_t)player_armour_class_armour());
    wr_short((uint16_t)player_armour_class_magical());
    // The four numbers the sheet shows. Their place in the file cannot move.
    wr_short((uint16_t)player_display_to_hit());
    wr_short((uint16_t)player_display_to_dam());
    wr_short((uint16_t)player_display_ac());
    wr_short((uint16_t)player_display_to_ac());
    wr_short((uint16_t)player_disarm());
    wr_short((uint16_t)player_saving_throw());
    wr_short((uint16_t)player_social_class());
    // Stealth sits next to social class at the same width, so swapping the two
    // fails no test (findings.md 46). The character screen shows it: Stealth
    // turns Superb and Social Class drops to one digit.
    wr_short((uint16_t)player_stealth());
    // Four bytes in a row: class, race, hit die, experience factor. Swapping any
    // pair is undetectable by width; the character sheet catches it.
    wr_byte((uint8_t)player_class());
    wr_byte((uint8_t)player_race());
    wr_byte((uint8_t)player_hit_die());
    wr_byte(player_experience_factor());
    wr_short((uint16_t)player_mana());
    wr_short(player_mana_fraction());
    wr_short((uint16_t)player_hp());
    wr_short(player_hp_fraction());
    for (int i = 0; i < PLAYER_HISTORY_LINES; i++) {
        wr_string(player_history_line(i));
    }

    struct player_stat *s_ptr = &py.stats;
    wr_bytes(s_ptr->max_stat, 6);
    wr_bytes(s_ptr->cur_stat, 6);
    wr_shorts((uint16_t *)s_ptr->mod_stat, 6);
    wr_bytes(s_ptr->use_stat, 6);

    wr_long(player_status_word());
    // Timed effects and related fields: twenty-four values with rest, food, speed,
    // and infra_range interspersed. Write each by name; order is part of the file
    // format.
    wr_short((uint16_t)player_rest_turns());
    wr_short((uint16_t)player_timed_turns(PLAYER_TIMED_BLINDNESS));
    wr_short((uint16_t)player_timed_turns(PLAYER_TIMED_PARALYSIS));
    wr_short((uint16_t)player_timed_turns(PLAYER_TIMED_CONFUSION));
    wr_short((uint16_t)player_food());
    wr_short((uint16_t)player_digestion());
    // Dead 2-byte hole: write back what we read (see dead_protection_bytes above).
    wr_short((uint16_t)dead_protection_bytes);
    wr_short((uint16_t)player_speed());
    wr_short((uint16_t)player_timed_turns(PLAYER_TIMED_HASTE));
    wr_short((uint16_t)player_timed_turns(PLAYER_TIMED_SLOWNESS));
    wr_short((uint16_t)player_timed_turns(PLAYER_TIMED_FEAR));
    wr_short((uint16_t)player_timed_turns(PLAYER_TIMED_POISON));
    wr_short((uint16_t)player_timed_turns(PLAYER_TIMED_HALLUCINATION));
    wr_short((uint16_t)player_timed_turns(PLAYER_TIMED_PROTECTION_FROM_EVIL));
    wr_short((uint16_t)player_timed_turns(PLAYER_TIMED_INVULNERABILITY));
    wr_short((uint16_t)player_timed_turns(PLAYER_TIMED_HEROISM));
    wr_short((uint16_t)player_timed_turns(PLAYER_TIMED_SUPER_HEROISM));
    wr_short((uint16_t)player_timed_turns(PLAYER_TIMED_BLESSING));
    wr_short((uint16_t)player_timed_turns(PLAYER_TIMED_HEAT_RESISTANCE));
    wr_short((uint16_t)player_timed_turns(PLAYER_TIMED_COLD_RESISTANCE));
    wr_short((uint16_t)player_timed_turns(PLAYER_TIMED_SEEING_INVISIBLE));
    wr_short((uint16_t)player_timed_turns(PLAYER_TIMED_WORD_OF_RECALL));
    wr_short((uint16_t)player_infra_range());
    wr_short((uint16_t)player_timed_turns(PLAYER_TIMED_INFRA_VISION));
    // Seventeen ability bytes: their order is part of the file format, so we write
    // them by position rather than one-by-one.
    for (int i = 0; i < PLAYER_ABILITIES_SAVED_BYTES; i++) {
        wr_byte(player_abilities_saved_byte(i));
    }
    wr_byte((uint8_t)player_glowing_hands());
    wr_byte((uint8_t)player_spells_to_learn());

    wr_short((uint16_t)missile_serial_value());
    wr_long((uint32_t)progress_turn());
    wr_short((uint16_t)inventory_count());
    for (int i = 0; i < inventory_count(); i++) {
        wr_item(inventory_at(i));
    }
    for (int i = equipment_first_slot(); i < equipment_end_slot(); i++) {
        wr_item(equipment_at(i));
    }
    wr_short((uint16_t)inventory_weight());
    wr_short((uint16_t)equipment_count());
    wr_long(spells_learned_bits());
    wr_long(spells_worked_bits());
    wr_long(spells_forgotten_bits());
    wr_bytes(spell_order_bytes(), 32);
    wr_bytes(item_kind_record_bytes(), item_kind_record_count());
    wr_long(progress_color_seed());
    wr_long(progress_town_seed());
    // Message history: newest index, then all slots in storage order.
    wr_short((uint16_t)msg_history_newest_slot());
    for (int i = 0; i < msg_history_slot_count(); i++) {
        wr_string(msg_history_slot(i));
    }

    // this indicates 'cheating' if it is a one
    wr_short((uint16_t)is_panic_save());
    wr_short((uint16_t)player_has_won());
    wr_short((uint16_t)score_disqualifications());
    wr_shorts(hp_table_slots(), MAX_PLAYER_LEVEL);

    for (int i = 0; i < store_count(); i++) {
        wr_store(store_at(i));
    }

    // save the current time in the savefile
    l = (uint32_t)time((time_t *)0);

    if (l < start_time) {
        // someone is messing with the clock!,
        // assume that we have been playing for 1 day
        l = start_time + 86400L;
    }
    wr_long(l);

    // starting with 5.2, put died_from string in savefile
    wr_string(death_cause());

    // starting with 5.2.2, put the max_score in the savefile
    l = (uint32_t)(total_points());
    wr_long(l);

    // starting with 5.2.2, put the birth_date in the savefile
    wr_long((uint32_t)character_birth_date());

    // only level specific info follows, this allows characters to be
    // resurrected, the dungeon level info is not needed for a resurrection
    if (player_is_dead()) {
        if (ferror(fileptr) || fflush(fileptr) == EOF) {
            return false;
        }
        return true;
    }

    wr_short((uint16_t)dungeon_level());
    wr_short((uint16_t)player_row());
    wr_short((uint16_t)player_col());
    wr_short((uint16_t)monster_breeding_count());
    wr_short((uint16_t)dungeon_height());
    wr_short((uint16_t)dungeon_width());
    wr_short((uint16_t)panel_max_row_index());
    wr_short((uint16_t)panel_max_col_index());

    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            cave_type *c_ptr = square_at(i, j);
            if (c_ptr->cptr != 0) {
                wr_byte((uint8_t)i);
                wr_byte((uint8_t)j);
                wr_byte(c_ptr->cptr);
            }
        }
    }

    // marks end of cptr info
    wr_byte((uint8_t)0xFF);

    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            cave_type *c_ptr = square_at(i, j);
            if (c_ptr->tptr != 0) {
                wr_byte((uint8_t)i);
                wr_byte((uint8_t)j);
                wr_byte(c_ptr->tptr);
            }
        }
    }

    // marks end of tptr info
    wr_byte((uint8_t)0xFF);

    // must set counter to zero, note that code may write out two bytes unnecessarily
    int count = 0;
    uint8_t prev_char = 0;

    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            cave_type *c_ptr = square_at(i, j);

            uint8_t char_tmp = c_ptr->fval | (c_ptr->lr << 4) | (c_ptr->fm << 5) | (c_ptr->pl << 6) | (c_ptr->tl << 7);

            if (char_tmp != prev_char || count == MAX_UCHAR) {
                wr_byte((uint8_t)count);
                wr_byte(prev_char);
                prev_char = char_tmp;
                count = 1;
            } else {
                count++;
            }
        }
    }

    // save last entry
    wr_byte((uint8_t)count);
    wr_byte(prev_char);

    wr_short((uint16_t)floor_items_used());
    for (int i = MIN_TRIX; i < floor_items_used(); i++) {
        wr_item(floor_item_at(i));
    }
    wr_short((uint16_t)monster_list_used());
    for (int i = MIN_MONIX; i < monster_list_used(); i++) {
        wr_monster(monster_list_at(i));
    }

    if (ferror(fileptr) || (fflush(fileptr) == EOF)) {
        return false;
    }
    return true;
}

// Set up prior to actual save, do the save, then clean up
bool save_char(void) {
    while (!_save_char(save_file_path())) {
        msgtype temp;

        (void)snprintf(temp, sizeof(temp), "Savefile '%s' fails.", save_file_path());
        msg_print(temp);

        int i = 0;
        if (access(save_file_path(), 0) < 0 ||
            get_check("File exists. Delete old savefile?") == 0 ||
            (i = unlink(save_file_path())) < 0) {
            if (i < 0) {
                (void)snprintf(temp, sizeof(temp), "Can't delete '%s'", save_file_path());
                msg_print(temp);
            }
            prt("New Savefile [ESC to give up]:", 0, 0);
            if (!get_string(temp, 0, 31, 45)) {
                return false;
            }
            if (temp[0]) {
                (void)strcpy(save_file_path(), temp);
            }
        }
        (void)snprintf(temp, sizeof(temp), "Saving with %s...", save_file_path());
        prt(temp, 0, 0);
    }

    return true;
}

bool _save_char(char *fnam) {
    if (character_is_saved()) {
        return true; // Nothing to save.
    }

    nosignals();
    put_qio();
    disturb(1, 0);             // Turn off resting and searching.
    change_speed(-pack_speed_penalty()); // Fix the speed
    set_pack_speed_penalty(0);
    bool ok = false;

    fileptr = NULL; // Do not assume it has been init'ed

    int fd = open(fnam, O_RDWR | O_CREAT | O_EXCL, 0600);

    if (fd < 0 && access(fnam, 0) >= 0 && (from_savefile || (progress_wizard_mode() && get_check("Can't make new savefile. Overwrite old?")))) {
        (void)chmod(fnam, 0600);
        fd = open(fnam, O_RDWR | O_TRUNC, 0600);
    }

    if (fd >= 0) {
        (void)close(fd);
        fileptr = fopen(save_file_path(), "wb");
    }

    SAVE_LOG(logfile = fopen("IO_LOG", "a"));
    SAVE_LOG(fprintf(logfile, "Saving data to %s\n", save_file_path()));

    if (fileptr != NULL) {
        xor_byte = 0;
        wr_byte((uint8_t)CUR_VERSION_MAJ);
        xor_byte = 0;
        wr_byte((uint8_t)CUR_VERSION_MIN);
        xor_byte = 0;
        wr_byte((uint8_t)PATCH_LEVEL);
        xor_byte = 0;

        uint8_t char_tmp = randint(256) - 1;
        wr_byte(char_tmp);
        // Note that xor_byte is now equal to char_tmp

        ok = sv_write();

        SAVE_LOG(fclose(logfile));

        if (fclose(fileptr) == EOF) {
            ok = false;
        }
    }

    if (!ok) {
        if (fd >= 0) {
            (void)unlink(fnam);
        }
        signals();
        vtype temp;
        if (fd >= 0) {
            (void)sprintf(temp, "Error writing to file %s", fnam);
        } else {
            (void)sprintf(temp, "Can't create new file %s", fnam);
        }
        msg_print(temp);

        return false;
    } else {
        set_character_saved(true);
    }

    progress_set_turn(-1);
    signals();

    return true;
}

// Certain checks are ommitted for the wizard. -CJS-
bool get_char(bool *generate) {
    uint32_t time_saved;

    nosignals();
    *generate = true;
    int fd = -1;

    // Not required for Mac, because the file name is obtained through a dialog.
    // There is no way for a non existnat file to be specified. -BS-
    if (access(save_file_path(), 0) != 0) {
        signals();
        msg_print("Savefile does not exist.");
        return false; // Don't bother with messages here. File absent.
    }

    clear_screen();

    msgtype temp;
    (void)snprintf(temp, sizeof(temp), "Savefile %s present. Attempting restore.", save_file_path());
    put_buffer(temp, 23, 0);

    // FIXME: check this if/else logic! -- MRC
    if (save_state_character_is_in_play()) {
        msg_print("IMPOSSIBLE! Attempt to restore while still alive!");
    } else if ((fd = open(save_file_path(), O_RDONLY, 0)) < 0 && (chmod(save_file_path(), 0400) < 0 || (fd = open(save_file_path(), O_RDONLY, 0)) < 0)) {
        // Allow restoring a file belonging to someone else, if we can delete it.
        // Hence first try to read without doing a chmod.

        msg_print("Can't open file for reading.");
    } else {
        progress_set_turn(-1);
        bool ok = true;

        (void)close(fd);
        fd = -1; // Make sure it isn't closed again
        fileptr = fopen(save_file_path(), "rb");

        if (fileptr == NULL) {
            goto error;
        }

        prt("Restoring Memory...", 0, 0);
        put_qio();

        SAVE_LOG(logfile = fopen("IO_LOG", "a"));
        SAVE_LOG(fprintf(logfile, "Reading data from %s\n", save_file_path()));

        uint8_t version_maj, version_min, patch_level;

        xor_byte = 0;
        rd_byte(&version_maj);
        xor_byte = 0;
        rd_byte(&version_min);
        xor_byte = 0;
        rd_byte(&patch_level);
        xor_byte = 0;
        rd_byte(&xor_byte);

        // COMPAT support savefiles from 5.0.14 to 5.0.17.
        // Support savefiles from 5.1.0 to present.
        // As of version 5.4, accept savefiles even if they have higher version numbers.
        // The savefile format was frozen as of version 5.2.2.
        if ((version_maj != CUR_VERSION_MAJ) || savefile_before(version_min, patch_level, 0, 14)) {
            prt("Sorry. This savefile is from a different version of umoria.", 2, 0);
            goto error;
        }

        uint16_t uint16_t_tmp;
        rd_short(&uint16_t_tmp);
        while (uint16_t_tmp != 0xFFFF) {
            if (uint16_t_tmp >= MAX_CREATURES) {
                goto error;
            }
            recall_type *r_ptr = recall_get(monster_make_creature_handle(uint16_t_tmp));
            rd_long(&r_ptr->r_cmove);
            rd_long(&r_ptr->r_spells);
            rd_short(&r_ptr->r_kills);
            rd_short(&r_ptr->r_deaths);
            rd_short(&r_ptr->r_cdefense);
            rd_byte(&r_ptr->r_wake);
            rd_byte(&r_ptr->r_ignore);
            rd_bytes(r_ptr->r_attacks, MAX_MON_NATTACK);
            rd_short(&uint16_t_tmp);
        }

        // for save files before 5.2.2, read and ignore log_index (sic)
        if (savefile_before(version_min, patch_level, 2, 2)) {
            rd_short(&uint16_t_tmp);
        }

        uint32_t l;
        rd_long(&l);

        // The same eleven bits sv_write() packed; the bit assignment lives in
        // options.c, so this side cannot drift from that one.
        game_options_unpack(l);

        // save files before 5.2.2 have no bit for sound_beep_flag nor for
        // display_counts, so the bits just read are meaningless. Set them on
        // for compatibility.
        if (savefile_before(version_min, patch_level, 2, 2)) {
            sound_beep_flag = true;
            display_counts = true;
        }

        // Don't allow resurrection of characters that have won.  It causes
        // problems because the character level is out of the allowed range.
        if (progress_wizard_requested() && (l & 0x40000000L)) {
            msg_print("Sorry, this character is retired from moria.");
            msg_print("You can not resurrect a retired character.");
        } else if (progress_wizard_requested() && (l & 0x80000000L) && get_check("Resurrect a dead character?")) {
            l &= ~0x80000000L;
        }

        if ((l & 0x80000000L) == 0) {
            // Player bio fields. Read into locals, then set through accessors. Order
            // is part of the file format.
            char name[PLAYER_NAME_SIZE];
            rd_string(name);
            player_name_set(name);
            // Male flag: nonzero means male. A hand-edited 2 still works.
            uint8_t male;
            rd_byte(&male);
            player_set_male(male != 0);
            uint32_t gold;
            rd_long(&gold);
            player_set_gold((int32_t)gold);
            uint32_t max_exp;
            rd_long(&max_exp);
            player_set_max_experience((int32_t)max_exp);
            uint32_t exp;
            rd_long(&exp);
            player_set_experience((int32_t)exp);
            uint16_t exp_frac;
            rd_short(&exp_frac);
            player_set_experience_fraction(exp_frac);
            // Age first, then height. Order cannot move.
            uint16_t age;
            rd_short(&age);
            player_age_set(age);
            uint16_t height;
            rd_short(&height);
            player_height_set(height);
            uint16_t body_weight;
            rd_short(&body_weight);
            player_body_weight_set(body_weight);
            uint16_t lev;
            rd_short(&lev);
            player_set_level(lev);
            // Max depth: _set replaces the value, doesn't take the deeper of the two.
            uint16_t max_depth;
            rd_short(&max_depth);
            player_max_depth_set(max_depth);
            // Search: two setters (wizard.c only replaces chance, not both).
            uint16_t search_chance;
            uint16_t search_frequency;
            rd_short(&search_chance);
            rd_short(&search_frequency);
            player_search_chance_set((int16_t)search_chance);
            player_search_frequency_set((int16_t)search_frequency);
            // Base to-hit: one setter takes both values.
            uint16_t base_to_hit;
            uint16_t base_to_hit_with_bows;
            rd_short(&base_to_hit);
            rd_short(&base_to_hit_with_bows);
            player_base_to_hit_set((int16_t)base_to_hit, (int16_t)base_to_hit_with_bows);
            // Mana: max here, current and fraction below (after class/race).
            uint16_t max_mana;
            rd_short(&max_mana);
            player_set_max_mana((int16_t)max_mana);
            // HP: max here, current and fraction below (after class/race).
            uint16_t max_hp;
            rd_short(&max_hp);
            player_set_max_hp((int16_t)max_hp);
            // Attack bonuses: to-hit first, then to-dam. One setter takes both.
            uint16_t to_hit_bonus;
            uint16_t to_damage_bonus;
            rd_short(&to_hit_bonus);
            rd_short(&to_damage_bonus);
            player_attack_bonuses_set((int16_t)to_hit_bonus, (int16_t)to_damage_bonus);
            // Armor class: _set_parts, not _reset. The file's values already include
            // worn armor, so we can't zero out that part first.
            uint16_t armour_class;
            uint16_t magical_armour_class;
            rd_short(&armour_class);
            rd_short(&magical_armour_class);
            player_armour_class_set_parts((int16_t)armour_class, (int16_t)magical_armour_class);
            // Display values: four shorts, same order as written. AC total comes
            // before modifier.
            uint16_t dis_th;
            rd_short(&dis_th);
            player_display_set_to_hit((int16_t)dis_th);
            uint16_t dis_td;
            rd_short(&dis_td);
            player_display_set_to_dam((int16_t)dis_td);
            uint16_t dis_ac;
            rd_short(&dis_ac);
            player_display_set_ac((int16_t)dis_ac);
            uint16_t dis_tac;
            rd_short(&dis_tac);
            player_display_set_to_ac((int16_t)dis_tac);
            uint16_t disarm;
            rd_short(&disarm);
            player_disarm_set((int16_t)disarm);
            uint16_t saving_throw;
            rd_short(&saving_throw);
            player_saving_throw_set((int16_t)saving_throw);
            uint16_t social_class;
            rd_short(&social_class);
            player_social_class_set((int16_t)social_class);
            // Stealth: signed value round-trips through uint16_t (Half-Troll Warrior
            // can be -1).
            uint16_t stealth;
            rd_short(&stealth);
            player_stealth_set((int16_t)stealth);
            // Four bytes in a row: class, race, hit die, experience factor.
            uint8_t pclass;
            rd_byte(&pclass);
            player_class_set(pclass);
            uint8_t prace;
            rd_byte(&prace);
            player_race_set(prace);
            uint8_t hit_die;
            rd_byte(&hit_die);
            player_hit_die_set(hit_die);
            uint8_t expfact;
            rd_byte(&expfact);
            player_set_experience_factor(expfact);
            uint16_t cur_mana;
            rd_short(&cur_mana);
            player_set_mana((int16_t)cur_mana);
            uint16_t cur_mana_frac;
            rd_short(&cur_mana_frac);
            player_set_mana_fraction(cur_mana_frac);
            uint16_t cur_hp;
            rd_short(&cur_hp);
            player_set_hp((int16_t)cur_hp);
            uint16_t cur_hp_frac;
            rd_short(&cur_hp_frac);
            player_set_hp_fraction(cur_hp_frac);
            for (int i = 0; i < PLAYER_HISTORY_LINES; i++) {
                char line[PLAYER_HISTORY_LINE_SIZE];
                rd_string(line);
                player_history_line_set(i, line);
            }

            struct player_stat *s_ptr = &py.stats;
            rd_bytes(s_ptr->max_stat, 6);
            rd_bytes(s_ptr->cur_stat, 6);
            rd_shorts((uint16_t *)s_ptr->mod_stat, 6);
            rd_bytes(s_ptr->use_stat, 6);

            // Status word: bit positions are part of the file format, so we move the
            // whole word instead of thirty individual flags.
            uint32_t status;
            rd_long(&status);
            player_set_status_word(status);
            // Rest: signed value, so we preserve all sixteen bits.
            uint16_t rest_turns;
            rd_short(&rest_turns);
            player_rest_set((int16_t)rest_turns);
            // Eighteen timed effects: read each by name, same as when writing.
            rd_timed(PLAYER_TIMED_BLINDNESS);
            rd_timed(PLAYER_TIMED_PARALYSIS);
            rd_timed(PLAYER_TIMED_CONFUSION);
            uint16_t food;
            rd_short(&food);
            player_set_food((int16_t)food);
            uint16_t food_digested;
            rd_short(&food_digested);
            player_set_digestion((int16_t)food_digested);
            // Dead 2-byte hole: this one goes to a local static, so we can read
            // directly to its address (see dead_protection_bytes above).
            rd_short((uint16_t *)&dead_protection_bytes);
            uint16_t speed;
            rd_short(&speed);
            player_speed_set((int16_t)speed);
            rd_timed(PLAYER_TIMED_HASTE);
            rd_timed(PLAYER_TIMED_SLOWNESS);
            rd_timed(PLAYER_TIMED_FEAR);
            rd_timed(PLAYER_TIMED_POISON);
            rd_timed(PLAYER_TIMED_HALLUCINATION);
            rd_timed(PLAYER_TIMED_PROTECTION_FROM_EVIL);
            rd_timed(PLAYER_TIMED_INVULNERABILITY);
            rd_timed(PLAYER_TIMED_HEROISM);
            rd_timed(PLAYER_TIMED_SUPER_HEROISM);
            rd_timed(PLAYER_TIMED_BLESSING);
            rd_timed(PLAYER_TIMED_HEAT_RESISTANCE);
            rd_timed(PLAYER_TIMED_COLD_RESISTANCE);
            rd_timed(PLAYER_TIMED_SEEING_INVISIBLE);
            rd_timed(PLAYER_TIMED_WORD_OF_RECALL);
            uint16_t infra_range;
            rd_short(&infra_range);
            player_infra_range_set((int16_t)infra_range);
            rd_timed(PLAYER_TIMED_INFRA_VISION);
            // Seventeen ability bytes: read by position, same as when writing.
            for (int i = 0; i < PLAYER_ABILITIES_SAVED_BYTES; i++) {
                uint8_t ability;
                rd_byte(&ability);
                player_abilities_restore_byte(i, ability);
            }
            uint8_t saved_glowing_hands;
            rd_byte(&saved_glowing_hands);
            player_glowing_hands_restore(saved_glowing_hands);
            uint8_t saved_spells_to_learn;
            rd_byte(&saved_spells_to_learn);
            player_spells_to_learn_set(saved_spells_to_learn);

            uint16_t saved_missile_serial;
            rd_short(&saved_missile_serial);
            set_missile_serial((int16_t)saved_missile_serial);
            uint32_t saved_turn;
            rd_long(&saved_turn);
            progress_set_turn((int32_t)saved_turn);
            uint16_t pack_count;
            rd_short(&pack_count);
            inventory_set_count((int16_t)pack_count);
            if (inventory_count() > inventory_slot_count()) {
                goto error;
            }
            for (int i = 0; i < inventory_count(); i++) {
                rd_item(inventory_at(i));
            }
            for (int i = equipment_first_slot(); i < equipment_end_slot(); i++) {
                rd_item(equipment_at(i));
            }
            uint16_t pack_weight;
            rd_short(&pack_weight);
            inventory_set_weight((int16_t)pack_weight);
            uint16_t equip_count;
            rd_short(&equip_count);
            equipment_set_count((int16_t)equip_count);
            uint32_t saved_spells_learned;
            rd_long(&saved_spells_learned);
            spells_set_learned_bits(saved_spells_learned);
            uint32_t saved_spells_worked;
            rd_long(&saved_spells_worked);
            spells_set_worked_bits(saved_spells_worked);
            uint32_t saved_spells_forgotten;
            rd_long(&saved_spells_forgotten);
            spells_set_forgotten_bits(saved_spells_forgotten);
            rd_bytes(spell_order_bytes(), 32);
            rd_bytes(item_kind_record_bytes(), item_kind_record_count());
            uint32_t saved_color_seed;
            rd_long(&saved_color_seed);
            progress_set_color_seed(saved_color_seed);
            uint32_t saved_town_seed;
            rd_long(&saved_town_seed);
            progress_set_town_seed(saved_town_seed);
            uint16_t newest_msg_slot;
            rd_short(&newest_msg_slot);
            msg_history_set_newest_slot(newest_msg_slot);
            for (int i = 0; i < msg_history_slot_count(); i++) {
                rd_string(msg_history_slot(i));
            }

            bool saved_panic;
            rd_bool(&saved_panic);
            set_panic_save(saved_panic);
            bool saved_has_won;
            rd_bool(&saved_has_won);
            set_player_has_won(saved_has_won);
            uint16_t saved_disqualifications;
            rd_short(&saved_disqualifications);
            set_score_disqualifications((int16_t)saved_disqualifications);
            rd_shorts(hp_table_slots(), MAX_PLAYER_LEVEL);

            if (!savefile_before(version_min, patch_level, 1, 3)) {
                for (int i = 0; i < store_count(); i++) {
                    if (!rd_store(store_at(i))) {
                        goto error;
                    }
                }
            }

            if (!savefile_before(version_min, patch_level, 1, 3)) {
                rd_long(&time_saved);
            }

            if (version_min >= 2) {
                rd_string(death_cause());
            }

            if (!savefile_before(version_min, patch_level, 2, 2)) {
                // Read into a local first: max_score is int32_t and rd_long
                // takes a uint32_t *, so the old cast lied about the pointer's
                // type (the same fix as panel and the player's position).
                uint32_t saved_best_score;
                rd_long(&saved_best_score);
                set_best_score_so_far((int32_t)saved_best_score);
            } else {
                set_best_score_so_far(0);
            }

            if (!savefile_before(version_min, patch_level, 2, 2)) {
                uint32_t saved_birth_date;
                rd_long(&saved_birth_date);
                set_character_birth_date((int32_t)saved_birth_date);
            } else {
                set_character_birth_date((int32_t)time((time_t *)0));
            }
        }

        int c = getc(fileptr);
        if (c == EOF || (l & 0x80000000L)) {
            if ((l & 0x80000000L) == 0) {
                if (!progress_wizard_requested() || !save_state_character_is_in_play()) {
                    goto error;
                }
                prt("Attempting a resurrection!", 0, 0);
                (void)player_resurrect_hp();

                // don't let him starve to death immediately
                if (player_food() < 0) {
                    player_set_food(0);
                }

                // don't let him die of poison again immediately
                player_timed_shorten_to(PLAYER_TIMED_POISON, 1);

                set_dungeon_level(0); // Resurrect on the town level.
                set_character_generated(true);

                // set noscore to indicate a resurrection, and don't enter
                // wizard mode
                progress_set_wizard_requested(false);
                set_score_disqualifications((int16_t)(score_disqualifications() | SCORE_DISQUALIFY_RESURRECTED));
            } else {
                // Make sure that this message is seen, since it is a bit
                // more interesting than the other messages.
                msg_print("Restoring Memory of a departed spirit...");
                progress_set_turn(-1);
            }
            put_qio();
            goto closefiles;
        }
        if (ungetc(c, fileptr) == EOF) {
            goto error;
        }

        prt("Restoring Character...", 0, 0);
        put_qio();

        // only level specific info should follow,
        // not present for dead characters

        // Read into a local uint16_t, then set through the window.
        uint16_t dungeon_level_read;
        rd_short(&dungeon_level_read);
        set_dungeon_level((int16_t)dungeon_level_read);
        uint16_t char_row_read, char_col_read;
        rd_short(&char_row_read);
        rd_short(&char_col_read);
        player_place((int16_t)char_row_read, (int16_t)char_col_read);
        uint16_t mon_tot_mult_read;
        rd_short(&mon_tot_mult_read);
        set_monster_breeding_count((int16_t)mon_tot_mult_read);
        // Dungeon size: one setter takes both height and width.
        uint16_t level_height_read, level_width_read;
        rd_short(&level_height_read);
        rd_short(&level_width_read);
        set_dungeon_size((int16_t)level_height_read, (int16_t)level_width_read);
        uint16_t max_panel_rows_read, max_panel_cols_read;
        rd_short(&max_panel_rows_read);
        rd_short(&max_panel_cols_read);
        panel_set_max_indexes((int16_t)max_panel_rows_read, (int16_t)max_panel_cols_read);

        uint8_t char_tmp, ychar, xchar, count;

        // read in the creature ptr info
        rd_byte(&char_tmp);
        while (char_tmp != 0xFF) {
            ychar = char_tmp;
            rd_byte(&xchar);
            rd_byte(&char_tmp);
            if (xchar > MAX_WIDTH || ychar > MAX_HEIGHT) {
                goto error;
            }
            square_at(ychar, xchar)->cptr = char_tmp;
            rd_byte(&char_tmp);
        }

        // read in the treasure ptr info
        rd_byte(&char_tmp);
        while (char_tmp != 0xFF) {
            ychar = char_tmp;
            rd_byte(&xchar);
            rd_byte(&char_tmp);
            if (xchar > MAX_WIDTH || ychar > MAX_HEIGHT) {
                goto error;
            }
            square_at(ychar, xchar)->tptr = char_tmp;
            rd_byte(&char_tmp);
        }

        // Run-length encoded fval and light bits. The file stores squares in
        // row-major order, so total_count directly indexes the grid: square n is
        // (n / MAX_WIDTH, n % MAX_WIDTH).
        int total_count = 0;
        while (total_count != MAX_HEIGHT * MAX_WIDTH) {
            rd_byte(&count);
            rd_byte(&char_tmp);
            for (int i = count; i > 0; i--) {
                if (total_count >= MAX_HEIGHT * MAX_WIDTH) {
                    goto error;
                }
                cave_type *c_ptr =
                    square_at(total_count / MAX_WIDTH, total_count % MAX_WIDTH);
                c_ptr->fval = char_tmp & 0xF;
                c_ptr->lr = (char_tmp >> 4) & 0x1;
                c_ptr->fm = (char_tmp >> 5) & 0x1;
                c_ptr->pl = (char_tmp >> 6) & 0x1;
                c_ptr->tl = (char_tmp >> 7) & 0x1;
                total_count++;
            }
        }

        // The two marks below are put back before they are checked, exactly
        // as the old `rd_short((uint16_t *)&tcptr)` and `&mfptr` did -- a file
        // claiming more rows than the table holds leaves the mark bogus and
        // then fails the load.
        uint16_t floor_used;
        rd_short(&floor_used);
        set_floor_items_used((int16_t)floor_used);
        if (floor_items_used() > MAX_TALLOC) {
            goto error;
        }
        for (int i = MIN_TRIX; i < floor_items_used(); i++) {
            rd_item(floor_item_at(i));
        }
        uint16_t monsters_used;
        rd_short(&monsters_used);
        set_monster_list_used((int16_t)monsters_used);
        if (monster_list_used() > MAX_MALLOC) {
            goto error;
        }
        for (int i = MIN_MONIX; i < monster_list_used(); i++) {
            rd_monster(monster_list_at(i));
        }

        *generate = false; // We have restored a cave - no need to generate.

        if (savefile_before(version_min, patch_level, 1, 3)) {
            for (int i = 0; i < store_count(); i++) {
                if (!rd_store(store_at(i))) {
                    goto error;
                }
            }
        }

        // read the time that the file was saved
        if (savefile_before(version_min, patch_level, 0, 16)) {
            time_saved = 0; // no time in file, clear to zero
        } else if (version_min == 1 && patch_level < 3) {
            rd_long(&time_saved);
        }

        if (ferror(fileptr)) {
            goto error;
        }

        if (!save_state_character_is_in_play()) {
        error:
            ok = false; // Assume bad data.
        } else {
            // don't overwrite the killed by string if character is dead
            if (!player_hp_marks_death()) {
                (void)strcpy(death_cause(), "(alive and well)");
            }
            set_character_generated(true);
        }

    closefiles:

        SAVE_LOG(fclose(logfile));

        if (fileptr != NULL) {
            if (fclose(fileptr) < 0) {
                ok = false;
            }
        }
        if (fd >= 0) {
            (void)close(fd);
        }

        if (!ok) {
            msg_print("Error during reading of file.");
        } else {
            // let the user overwrite the old savefile when save/quit
            from_savefile = 1;

            signals();

            if (is_panic_save()) {
                (void)sprintf(temp, "This game is from a panic save.  Score "
                                    "will not be added to scoreboard.");
                msg_print(temp);
            } else if (((!score_disqualifications()) & SCORE_DISQUALIFY_DUPLICATE) && duplicate_character()) {
                (void)sprintf(temp, "This character is already on the "
                                    "scoreboard; it will not be scored again.");
                msg_print(temp);
                set_score_disqualifications((int16_t)(score_disqualifications() | SCORE_DISQUALIFY_DUPLICATE));
            }

            if (save_state_character_is_in_play()) { // Only if a full restoration.
                set_weapon_too_heavy(false);
                set_pack_speed_penalty(0);
                check_strength();

                // rotate store inventory, depending on how old the save file
                // is foreach day old (rounded up), call store_maint
                // calculate age in seconds
                start_time = (uint32_t)time((time_t *)0);

                uint32_t age;

                // check for reasonable values of time here ...
                if (start_time < time_saved) {
                    age = 0;
                } else {
                    age = start_time - time_saved;
                }

                age = (age + 43200L) / 86400L; // age in days
                if (age > 10) {
                    age = 10; // in case savefile is very old
                }

                for (int i = 0; i < (int)age; i++) {
                    store_maint();
                }
            }

            if (score_disqualifications()) {
                msg_print("This save file cannot be used to get on the score board.");
            }

            if (version_maj != CUR_VERSION_MAJ || version_min != CUR_VERSION_MIN) {
                (void)sprintf(
                    temp, "Save file version %d.%d %s on game version %d.%d.",
                    version_maj, version_min,
                    version_min <= CUR_VERSION_MIN ? "accepted" : "risky",
                    CUR_VERSION_MAJ, CUR_VERSION_MIN);
                msg_print(temp);
            }

            if (save_state_character_is_in_play()) {
                return true;
            } else {
                return false; // Only restored options and monster memory.
            }
        }
    }
    progress_set_turn(-1);
    prt("Please try again without that savefile.", 1, 0);
    signals();

    exit_game();

    return false; // not reached
}

static void wr_byte(uint8_t c) {
    xor_byte ^= c;
    (void)putc((int)xor_byte, fileptr);
    SAVE_LOG(fprintf(logfile, "BYTE:  %02X = %d\n", (int)xor_byte, (int)c));
}

static void wr_short(uint16_t s) {
    xor_byte ^= (s & 0xFF);
    (void)putc((int)xor_byte, fileptr);
    SAVE_LOG(fprintf(logfile, "SHORT: %02X", (int)xor_byte));
    xor_byte ^= ((s >> 8) & 0xFF);
    (void)putc((int)xor_byte, fileptr);
    SAVE_LOG(fprintf(logfile, " %02X = %d\n", (int)xor_byte, (int)s));
}

static void wr_long(uint32_t l) {
    xor_byte ^= (l & 0xFF);
    (void)putc((int)xor_byte, fileptr);
    SAVE_LOG(fprintf(logfile, "LONG:  %02X", (int)xor_byte));
    xor_byte ^= ((l >> 8) & 0xFF);
    (void)putc((int)xor_byte, fileptr);
    SAVE_LOG(fprintf(logfile, " %02X", (int)xor_byte));
    xor_byte ^= ((l >> 16) & 0xFF);
    (void)putc((int)xor_byte, fileptr);
    SAVE_LOG(fprintf(logfile, " %02X", (int)xor_byte));
    xor_byte ^= ((l >> 24) & 0xFF);
    (void)putc((int)xor_byte, fileptr);
    SAVE_LOG(fprintf(logfile, " %02X = %ld\n", (int)xor_byte, (int32_t)l));
}

static void wr_bytes(uint8_t *c, int count) {
    uint8_t *ptr;

    SAVE_LOG(fprintf(logfile, "%d BYTES:", count));
    ptr = c;
    for (int i = 0; i < count; i++) {
        xor_byte ^= *ptr++;
        (void)putc((int)xor_byte, fileptr);
        SAVE_LOG(fprintf(logfile, "  %02X = %d", (int)xor_byte, (int)(ptr[-1])));
    }
    SAVE_LOG(fprintf(logfile, "\n"));
}

// Parameter is const because the function only reads. Player name and history
// come through accessors, so we never write through the pointer.
static void wr_string(const char *str) {
    SAVE_LOG(const char *s = str);
    SAVE_LOG(fprintf(logfile, "STRING:"));
    while (*str != '\0') {
        xor_byte ^= *str++;
        (void)putc((int)xor_byte, fileptr);
        SAVE_LOG(fprintf(logfile, " %02X", (int)xor_byte));
    }
    xor_byte ^= *str;
    (void)putc((int)xor_byte, fileptr);
    SAVE_LOG(fprintf(logfile, " %02X = \"%s\"\n", (int)xor_byte, s));
}

static void wr_shorts(uint16_t *s, int count) {
    SAVE_LOG(fprintf(logfile, "%d SHORTS:", count));

    uint16_t *sptr = s;

    for (int i = 0; i < count; i++) {
        xor_byte ^= (*sptr & 0xFF);
        (void)putc((int)xor_byte, fileptr);
        SAVE_LOG(fprintf(logfile, "  %02X", (int)xor_byte));
        xor_byte ^= ((*sptr++ >> 8) & 0xFF);
        (void)putc((int)xor_byte, fileptr);
        SAVE_LOG(fprintf(logfile, " %02X = %d", (int)xor_byte, (int)sptr[-1]));
    }
    SAVE_LOG(fprintf(logfile, "\n"));
}

static void wr_item(inven_type *item) {
    SAVE_LOG(fprintf(logfile, "ITEM:\n"));
    wr_short(item->index);
    wr_byte(item->name2);
    wr_string(item->inscrip);
    wr_long(item->flags);
    wr_byte(item->tval);
    wr_byte(item->tchar);
    wr_short((uint16_t)item->p1);
    wr_long((uint32_t)item->cost);
    wr_byte(item->subval);
    wr_byte(item->number);
    wr_short(item->weight);
    wr_short((uint16_t)item->tohit);
    wr_short((uint16_t)item->todam);
    wr_short((uint16_t)item->ac);
    wr_short((uint16_t)item->toac);
    wr_bytes(item->damage, 2);
    wr_byte(item->level);
    wr_byte(item->ident);
}

// One shop: the counters at the front, then only the shelves it actually has
// something on. Written once and read twice (the current file format and the
// pre-5.1.3 one both store shops this way), so all three places used to carry
// their own copy of this field list.
static void wr_store(store_type *store) {
    SAVE_LOG(fprintf(logfile, "STORE:\n"));
    wr_long((uint32_t)store->store_open);
    wr_short((uint16_t)store->insult_cur);
    wr_byte(store->owner);
    wr_byte(store->store_ctr);
    wr_short(store->good_buy);
    wr_short(store->bad_buy);
    for (int i = 0; i < store->store_ctr; i++) {
        wr_long((uint32_t)store->store_inven[i].scost);
        wr_item(&store->store_inven[i].sitem);
    }
}

static void wr_monster(monster_type *mon) {
    SAVE_LOG(fprintf(logfile, "MONSTER:\n"));
    wr_short((uint16_t)mon->hp);
    wr_short((uint16_t)mon->csleep);
    wr_short((uint16_t)mon->cspeed);
    wr_short(mon->creature.place);
    wr_byte(mon->fy);
    wr_byte(mon->fx);
    wr_byte(mon->cdis);
    wr_byte(mon->ml);
    wr_byte(mon->stunned);
    wr_byte(mon->confused);
}

static void rd_byte(uint8_t *ptr) {
    uint8_t c = getc(fileptr) & 0xFF;
    *ptr = c ^ xor_byte;
    xor_byte = c;
    SAVE_LOG(fprintf(logfile, "BYTE:  %02X = %d\n", (int)c, (int)*ptr));
}

static void rd_short(uint16_t *ptr) {
    uint8_t c = (getc(fileptr) & 0xFF);
    uint16_t s = c ^ xor_byte;

    xor_byte = (getc(fileptr) & 0xFF);
    s |= (uint16_t)(c ^ xor_byte) << 8;
    *ptr = s;
    SAVE_LOG(fprintf(logfile, "SHORT: %02X %02X = %d\n", (int)c, (int)xor_byte, (int)s));
}

// Bools are written as two bytes by wr_short(), so reading consumes two bytes.
// Since bool is one byte, reading directly would overwrite adjacent memory. Read
// into uint16_t, then pack into bool.
static void rd_bool(bool *ptr) {
    uint16_t value;
    rd_short(&value);
    *ptr = (value != 0);
}

// Timed effect helper: read into a local, then set through the accessor. Called
// eighteen times, so factored into one place.
static void rd_timed(player_timed_effect effect) {
    uint16_t turns;
    rd_short(&turns);
    player_timed_set(effect, (int16_t)turns);
}

static void rd_long(uint32_t *ptr) {
    uint8_t c = (getc(fileptr) & 0xFF);
    uint32_t l = c ^ xor_byte;

    xor_byte = (getc(fileptr) & 0xFF);
    l |= (uint32_t)(c ^ xor_byte) << 8;
    SAVE_LOG(fprintf(logfile, "LONG:  %02X %02X ", (int)c, (int)xor_byte));
    c = (getc(fileptr) & 0xFF);
    l |= (uint32_t)(c ^ xor_byte) << 16;
    xor_byte = (getc(fileptr) & 0xFF);
    l |= (uint32_t)(c ^ xor_byte) << 24;
    *ptr = l;
    SAVE_LOG(fprintf(logfile, "%02X %02X = %ld\n", (int)c, (int)xor_byte, (int32_t)l));
}

static void rd_bytes(uint8_t *ch_ptr, int count) {
    SAVE_LOG(fprintf(logfile, "%d BYTES:", count));
    uint8_t *ptr = ch_ptr;
    for (int i = 0; i < count; i++) {
        uint8_t c = (getc(fileptr) & 0xFF);
        *ptr++ = c ^ xor_byte;
        xor_byte = c;
        SAVE_LOG(fprintf(logfile, "  %02X = %d", (int)c, (int)ptr[-1]));
    }
    SAVE_LOG(fprintf(logfile, "\n"));
}

static void rd_string(char *str) {
    SAVE_LOG(char *s = str);
    SAVE_LOG(fprintf(logfile, "STRING: "));
    do {
        uint8_t c = (getc(fileptr) & 0xFF);
        *str = c ^ xor_byte;
        xor_byte = c;
        SAVE_LOG(fprintf(logfile, "%02X ", (int)c));
    } while (*str++ != '\0');
    SAVE_LOG(fprintf(logfile, "= \"%s\"\n", s));
}

static void rd_shorts(uint16_t *ptr, int count) {
    SAVE_LOG(fprintf(logfile, "%d SHORTS:", count));
    uint16_t *sptr = ptr;

    for (int i = 0; i < count; i++) {
        uint8_t c = (getc(fileptr) & 0xFF);
        uint16_t s = c ^ xor_byte;
        xor_byte = (getc(fileptr) & 0xFF);
        s |= (uint16_t)(c ^ xor_byte) << 8;
        *sptr++ = s;
        SAVE_LOG(fprintf(logfile, "  %02X %02X = %d", (int)c, (int)xor_byte, (int)s));
    }
    SAVE_LOG(fprintf(logfile, "\n"));
}

static void rd_item(inven_type *item) {
    SAVE_LOG(fprintf(logfile, "ITEM:\n"));
    rd_short(&item->index);
    rd_byte(&item->name2);
    rd_string(item->inscrip);
    rd_long(&item->flags);
    rd_byte(&item->tval);
    rd_byte(&item->tchar);
    rd_short((uint16_t *)&item->p1);
    rd_long((uint32_t *)&item->cost);
    rd_byte(&item->subval);
    rd_byte(&item->number);
    rd_short(&item->weight);
    rd_short((uint16_t *)&item->tohit);
    rd_short((uint16_t *)&item->todam);
    rd_short((uint16_t *)&item->ac);
    rd_short((uint16_t *)&item->toac);
    rd_bytes(item->damage, 2);
    rd_byte(&item->level);
    rd_byte(&item->ident);
}

// The counterpart of wr_store(). Returns false if the file claims a shop has
// more items than it can hold: the shelf count decides how much is read next,
// so an impossible one means this is not one of our save files. The caller
// abandons the load, which is what both read paths did with their own copy of
// this check.
static bool rd_store(store_type *store) {
    SAVE_LOG(fprintf(logfile, "STORE:\n"));
    rd_long((uint32_t *)&store->store_open);
    rd_short((uint16_t *)&store->insult_cur);
    rd_byte(&store->owner);
    rd_byte(&store->store_ctr);
    rd_short(&store->good_buy);
    rd_short(&store->bad_buy);

    if (store->store_ctr > STORE_INVEN_MAX) {
        return false;
    }

    for (int i = 0; i < store->store_ctr; i++) {
        rd_long((uint32_t *)&store->store_inven[i].scost);
        rd_item(&store->store_inven[i].sitem);
    }

    return true;
}

static void rd_monster(monster_type *mon) {
    SAVE_LOG(fprintf(logfile, "MONSTER:\n"));
    rd_short((uint16_t *)&mon->hp);
    rd_short((uint16_t *)&mon->csleep);
    rd_short((uint16_t *)&mon->cspeed);
    rd_short(&mon->creature.place);
    rd_byte(&mon->fy);
    rd_byte(&mon->fx);
    rd_byte(&mon->cdis);
    rd_byte(&mon->ml);
    rd_byte(&mon->stunned);
    rd_byte(&mon->confused);
}

// functions called from death.c to implement the score file

// set the local fileptr to the scorefile fileptr
void set_fileptr(FILE *file) {
    fileptr = file;
}

void wr_highscore(high_scores *score) {
    SAVE_LOG(logfile = fopen("IO_LOG", "a"));
    SAVE_LOG(fprintf(logfile, "Saving score:\n"));

    // Save the encryption byte for robustness.
    wr_byte(xor_byte);

    wr_long((uint32_t)score->points);
    wr_long((uint32_t)score->birth_date);
    wr_short((uint16_t)score->uid);
    wr_short((uint16_t)score->mhp);
    wr_short((uint16_t)score->chp);
    wr_byte(score->dun_level);
    wr_byte(score->lev);
    wr_byte(score->max_dlv);
    wr_byte(score->sex);
    wr_byte(score->race);
    wr_byte(score->class);
    wr_bytes((uint8_t *)score->name, PLAYER_NAME_SIZE);
    wr_bytes((uint8_t *)score->died_from, 25);
    SAVE_LOG(fclose(logfile));
}

void rd_highscore(high_scores *score) {
    SAVE_LOG(logfile = fopen("IO_LOG", "a"));
    SAVE_LOG(fprintf(logfile, "Reading score:\n"));

    // Read the encryption byte.
    rd_byte(&xor_byte);

    rd_long((uint32_t *)&score->points);
    rd_long((uint32_t *)&score->birth_date);
    rd_short((uint16_t *)&score->uid);
    rd_short((uint16_t *)&score->mhp);
    rd_short((uint16_t *)&score->chp);
    rd_byte(&score->dun_level);
    rd_byte(&score->lev);
    rd_byte(&score->max_dlv);
    rd_byte(&score->sex);
    rd_byte(&score->race);
    rd_byte(&score->class);
    rd_bytes((uint8_t *)score->name, PLAYER_NAME_SIZE);
    rd_bytes((uint8_t *)score->died_from, 25);
    SAVE_LOG(fclose(logfile));
}
