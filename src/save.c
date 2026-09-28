// src/save.c: save and restore games and monster memory info
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

// セミコロンをマクロ引数の内側に置く。SAVE_LOG(x) は空に展開されるので、
// 外側に書くとファイル直下に裸の ';' が残り、ISO C では認められない
// （関数の中なら空文になるので、下の SAVE_LOG(...) 群はそのままでよい）。
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

// セーブファイルの死んだ 2 バイト。#18-12-15 まで `py.flags.protection`
// （"Protection fr. evil"）だったが、**ゲームはこの数を一度も見ない** ——
// 読み手も書き手も下の 2 行だけで、悪からの守りそのものは
// PLAYER_TIMED_PROTECTION_FROM_EVIL（別の数）が持っている。
//
// それでも 0 を書き捨てにせず読んだ値を覚えるのは、**この 2 バイトの位置が
// ファイルの書式**だからで、他が書いたファイルを読んで書き戻すときに中身を
// 落としたくない（前後の 23 個の short と同じ理由 —— player_timed_effects.h）。
//
// **新しい module を作らないのは、これが「問い」ではないから。** 窓口の向こうに
// 置く値には必ず「誰が何を訊くか」があるが、これを訊く者はいない。ここにあるのは
// 書式の穴で、穴はそれを読む 1 ファイルの中に置くのがいちばん小さい。
static int16_t dead_protection_bytes;

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

    // 人物の身上書きも窓口へ（#18-12-26B）。**並びはファイルの形なので
    // 動かせない** —— 名前・性別がここ、年齢と身長が下、階層はさらに下、
    // 生い立ち 4 行がいちばんあと。
    wr_string(player_name());
    // 生のバイトではなく真偽から 1 か 0 を書く。手で 2 を書きこんだ
    // セーブファイルは 1 になって戻る（player_bio.h の性別の項）。
    wr_byte((uint8_t)(player_is_male() ? 1 : 0));
    wr_long((uint32_t)player_gold());
    wr_long((uint32_t)player_max_experience());
    wr_long((uint32_t)player_experience());
    wr_short(player_experience_fraction());
    // **年齢が先、身長があと** —— 並びは動かせない。
    wr_short((uint16_t)player_age());
    wr_short((uint16_t)player_height());
    wr_short((uint16_t)player_body_weight());
    wr_short(player_level());
    wr_short((uint16_t)player_max_depth());
    // 探索の腕と頻度も窓口へ（#18-12-25B）。**腕が先、頻度があと** ——
    // 並びは動かせない。
    wr_short((uint16_t)player_search_chance());
    wr_short((uint16_t)player_search_frequency());
    // 素の命中力も窓口へ（#18-12-19B）。**近接が先、弓があと** ——
    // 並びは動かせない。
    wr_short((uint16_t)player_base_to_hit());
    wr_short((uint16_t)player_base_to_hit_with_bows());
    wr_short((uint16_t)player_max_mana());
    wr_short((uint16_t)player_max_hp());
    // 命中と打撃の下駄も窓口へ（#18-12-24B）。**命中が先、打撃があと** ——
    // 並びは動かせない。読みは 2 本だが、置きなおす窓口は対で 1 本。
    wr_short((uint16_t)player_to_hit_bonus());
    wr_short((uint16_t)player_to_damage_bonus());
    // 守りの点数も窓口へ（#18-12-18B）。**半分が 2 つあるのでファイルにも
    // 2 本ある** —— 着ているものぶんが先、魔法ぶんがあと。並びは動かせない。
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
    // 足音の静かさも窓口へ（#18-12-27B）。**手前の階層と同じ幅で隣りあっている**
    // ので、2 本を入れちがえてもテストは 1 件も落ちない（所見 46）—— 網は人物
    // 画面のほうで、入れかわると Stealth が Superb に、Social Class が 1 桁になる。
    wr_short((uint16_t)player_stealth());
    // 階級も窓口へ（#18-12-28B）。**ここから byte が 4 本つづく** ——
    // 階級・種族・体力の骰子・経験の倍率で、どの 2 本を入れちがえても幅では
    // 気づけない（所見 46）。**4 本が 4 つの別の module にあるので、並びを
    // 固定する 1 件はここには書けない** —— 網は人物画面のほうで、階級と種族が
    // 入れかわると Class 行と Race 行が同時にずれる。
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
    // 一時的な状態の十八個も窓口へ。**この二十四個の並びがこのファイルの
    // 書式**なので、能力の 17 バイトのように位置で訊くことはできない（十八個
    // の間に rest・腹の具合の二つ・protection・speed・see_infra が挟まって
    // いる）。だから一つずつ名前で書く。並びは元のまま。
    wr_short((uint16_t)player_rest_turns());
    wr_short((uint16_t)player_timed_turns(PLAYER_TIMED_BLINDNESS));
    wr_short((uint16_t)player_timed_turns(PLAYER_TIMED_PARALYSIS));
    wr_short((uint16_t)player_timed_turns(PLAYER_TIMED_CONFUSION));
    wr_short((uint16_t)player_food());
    wr_short((uint16_t)player_digestion());
    // 死んだ 2 バイトはこのファイルの static から（読んだ値をそのまま返す。
    // 上の dead_protection_bytes に理由を書いた）。
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
    // 装備で決まる耐性・能力 17 個も窓口へ。**この 17 バイトの並びがこの
    // ファイルの書式**なので、一つずつ名前で書くのをやめて、モジュールが
    // 持っている並び順に位置で 17 回訊く（旗の 1 語と同じ考え方）。
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
    // The file format is the raw ring: the index of the newest message, then
    // every slot in storage order.
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

    wr_short((uint16_t)dun_level);
    wr_short((uint16_t)player_row());
    wr_short((uint16_t)player_col());
    wr_short((uint16_t)monster_breeding_count());
    wr_short((uint16_t)dungeon_height());
    wr_short((uint16_t)dungeon_width());
    wr_short((uint16_t)panel_max_row_index());
    wr_short((uint16_t)panel_max_col_index());

    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            cave_type *c_ptr = &cave[i][j];
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
            cave_type *c_ptr = &cave[i][j];
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
            cave_type *c_ptr = &cave[i][j];

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

    wr_short((uint16_t)tcptr);
    for (int i = MIN_TRIX; i < tcptr; i++) {
        wr_item(&t_list[i]);
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
        if ((version_maj != CUR_VERSION_MAJ) || (version_min == 0 && patch_level < 14)) {
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
        if ((version_min < 2) || (version_min == 2 && patch_level < 2)) {
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
        if ((version_min < 2) || (version_min == 2 && patch_level < 2)) {
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
            // 人物の身上書きも窓口へ（#18-12-26B）。読みは器の番地を要る
            // ので、名前は局所の器に受けてから置く。**並びはファイルの形
            // なので動かせない**。
            char name[PLAYER_NAME_SIZE];
            rd_string(name);
            player_name_set(name);
            // 生のバイトを受けて「0 でなければ男」に落とす。2 は真だった
            // ので遊びの上のふるまいは変わらない（player_bio.h の性別の項）。
            uint8_t male;
            rd_byte(&male);
            player_set_male(male != 0);
            // 金は py.misc.au ではなく窓口へ入れる。読みは器の番地を要る
            // ので、いったん受けてから置く（幅と符号の扱いは元のまま）。
            uint32_t gold;
            rd_long(&gold);
            player_set_gold((int32_t)gold);
            // 階級と経験値の 5 つも窓口へ入れる。読みは器の番地を要るので、
            // いったん受けてから置く（幅と符号の扱いは元のまま。並び順は
            // ファイルの形なので動かせない）。
            uint32_t max_exp;
            rd_long(&max_exp);
            player_set_max_experience((int32_t)max_exp);
            uint32_t exp;
            rd_long(&exp);
            player_set_experience((int32_t)exp);
            uint16_t exp_frac;
            rd_short(&exp_frac);
            player_set_experience_fraction(exp_frac);
            // **年齢が先、身長があと** —— 並びは動かせない。
            uint16_t age;
            rd_short(&age);
            player_age_set(age);
            uint16_t height;
            rd_short(&height);
            player_height_set(height);
            // 体の重さも窓口へ（#18-12-23B）。番地に読んでいたので局所の
            // short に受けてから置く。**置きなおす窓口は創成と同じ 1 本**。
            uint16_t body_weight;
            rd_short(&body_weight);
            player_body_weight_set(body_weight);
            uint16_t lev;
            rd_short(&lev);
            player_set_level(lev);
            // どこまで潜ったかも窓口へ（#18-12-16B）。読みもどしは置きなおし
            // なので player_max_depth_set() —— 深いほうを残す窓口ではない。
            uint16_t max_depth;
            rd_short(&max_depth);
            player_max_depth_set(max_depth);
            // 読みもどしも置く窓口が 2 本（#18-12-25B）——
            // wizard.c が腕だけを置きかえるので対にはできない。
            uint16_t search_chance;
            uint16_t search_frequency;
            rd_short(&search_chance);
            rd_short(&search_frequency);
            player_search_chance_set((int16_t)search_chance);
            player_search_frequency_set((int16_t)search_frequency);
            // 読みもどしは種族の土台と同じ文なので、置きなおしの窓口が
            // 1 本で足りる（#18-12-19B。守りの点数だけが 2 本要った）。
            uint16_t base_to_hit;
            uint16_t base_to_hit_with_bows;
            rd_short(&base_to_hit);
            rd_short(&base_to_hit_with_bows);
            player_base_to_hit_set((int16_t)base_to_hit, (int16_t)base_to_hit_with_bows);
            // 魔力の 3 つも窓口へ（#18-12-4B）。並びは動かせないので位置は
            // そのまま —— 上限はここ、残りと端数は階級・種族のあと。
            uint16_t max_mana;
            rd_short(&max_mana);
            player_set_max_mana((int16_t)max_mana);
            // 体力も窓口へ（#18-12-5B）。並びは動かせないので位置はそのまま
            // —— 上限はここ、残りと端数は階級・種族のあと。
            uint16_t max_hp;
            rd_short(&max_hp);
            player_set_max_hp((int16_t)max_hp);
            // 下駄の 2 本。器の番地に読んでいたので局所に受けてから対で置く
            // （#18-12-24B）。**ファイルの並びが命中・打撃なので窓口の引数の
            // 並びもこれ** —— 15 つめが立てた問いへの 8 度めの答えで、
            // 遊びのときと同じ窓口を使う側。
            uint16_t to_hit_bonus;
            uint16_t to_damage_bonus;
            rd_short(&to_hit_bonus);
            rd_short(&to_damage_bonus);
            player_attack_bonuses_set((int16_t)to_hit_bonus, (int16_t)to_damage_bonus);
            // 守りの点数の 2 本。読みは器の番地を要るのでいったん受けてから
            // 窓口へ渡す。**_reset() ではなく _set_parts()** —— ファイルの数は
            // もう着ているものを含んでいるので、着ているものぶんを 0 に
            // する規則には従えない（player_armour_class.h）。
            uint16_t armour_class;
            uint16_t magical_armour_class;
            rd_short(&armour_class);
            rd_short(&magical_armour_class);
            player_armour_class_set_parts((int16_t)armour_class, (int16_t)magical_armour_class);
            // 画面に出す 4 つも窓口へ入れる。読みは器の番地を要るので、
            // いったん受けてから 1 つずつ置く（セーブデータの並びは動かせない
            // のでこの位置のまま。**書きだしと同じ順**で、AC の合計が修正より
            // 先に来る）。
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
            /* 罠と鍵をはずす腕。読みは器の番地を要るのでいったん受けて
             * から窓口へ渡す（置きなおしは種族の土台と同じ 1 本 ——
             * どちらもただの置きかえ。player_disarm.h）。 */
            uint16_t disarm;
            rd_short(&disarm);
            player_disarm_set((int16_t)disarm);
            /* 抵抗も同じ形（#18-12-21B）。読みは器の番地を要るので受けて
             * から窓口へ渡す。置きなおしは種族の土台と同じ 1 本
             * （player_saving_throw.h）。 */
            uint16_t saving_throw;
            rd_short(&saving_throw);
            player_saving_throw_set((int16_t)saving_throw);
            uint16_t social_class;
            rd_short(&social_class);
            player_social_class_set((int16_t)social_class);
            /* 番地を渡していた 1 か所。読んでから窓口へ渡す —— **符号なしで
             * 読んで符号つきに戻すので、負の静かさもそのまま往復する**
             * （Half-Troll の Warrior は -1）。 */
            uint16_t stealth;
            rd_short(&stealth);
            player_stealth_set((int16_t)stealth);
            /* 番地を渡していた 1 か所。読んでから窓口へ渡す —— **置きなおしは
             * 階級のメニューと同じ窓口**（種族・体力の骰子・素の命中力・
             * 罠と鍵をはずす腕・抵抗・足音の静かさと同じで、守りの点数だけが
             * ちがう。player_class.h）。**ここから byte が 4 本つづく**（所見 46）。 */
            uint8_t pclass;
            rd_byte(&pclass);
            player_class_set(pclass);
            /* 番地を渡していた 1 か所。読んでから窓口へ渡す
             * （置きなおしは種族のメニューと同じ窓口 —— どちらもただの
             * 置きかえ。player_race.h）。 */
            uint8_t prace;
            rd_byte(&prace);
            player_race_set(prace);
            /* 番地を渡していた 1 か所。読んでから窓口へ渡す
             * （置きなおしは種族の土台と同じ窓口 —— どちらもただの
             * 置きかえ。player_hit_die.h）。 */
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

            // 旗の 1 語も窓口へ。**ビットの番号はこのファイルの書式**なので、
            // 30 の旗を 1 つずつではなく 1 語まるごと運ぶ窓口を使う。読みは
            // 器の番地を要るので、いったん受けてから置く（腹の具合と同じ形。
            // 並びは動かせないのでこの位置のまま）。
            uint32_t status;
            rd_long(&status);
            player_set_status_word(status);
            // 休息の残りも窓口の向こうなので器の番地を渡せない。いったん
            // 受けてから置く（十八個の rd_timed() と同じ形）。**符号のある数**
            // なので、元の rd_short((uint16_t *)&f_ptr->rest) と同じく
            // 16 ビットをそのまま移す。
            uint16_t rest_turns;
            rd_short(&rest_turns);
            player_rest_set((int16_t)rest_turns);
            // 一時的な状態の十八個も窓口へ。書くほうと同じ理由で位置ではなく
            // 名前で、rd_timed() が受けてから置く（並びはこのまま）。
            rd_timed(PLAYER_TIMED_BLINDNESS);
            rd_timed(PLAYER_TIMED_PARALYSIS);
            rd_timed(PLAYER_TIMED_CONFUSION);
            // 腹の具合は（#18-12-2C まで f_ptr 越しだったが）窓口へ入れる。
            // 読みは器の番地を要るので、いったん受けてから置く（幅と符号の
            // 扱いは元のまま。並びは動かせないのでこの位置のまま）。
            uint16_t food;
            rd_short(&food);
            player_set_food((int16_t)food);
            uint16_t food_digested;
            rd_short(&food_digested);
            player_set_digestion((int16_t)food_digested);
            // 死んだ 2 バイトはこのファイルの static へ。**ここだけは窓口の
            // 向こうではないので器の番地をそのまま渡せる**（上の
            // dead_protection_bytes に、module を作らない理由を書いた）。
            rd_short((uint16_t *)&dead_protection_bytes);
            // 速さも器の番地が要るのでいったん受けてから置く（腹の具合と同じ。
            // 並びは動かせないのでこの位置のまま）。
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
            // 赤外視の距離も器の番地が要るのでいったん受けてから置く
            // （速さ・腹の具合と同じ。並びは動かせないのでこの位置のまま）。
            uint16_t infra_range;
            rd_short(&infra_range);
            player_infra_range_set((int16_t)infra_range);
            rd_timed(PLAYER_TIMED_INFRA_VISION);
            // 17 個も窓口へ。読みは器の番地を要るので、いったん受けてから
            // 位置で置く（腹の具合と旗の 1 語と同じ形。並びは動かせないので
            // この位置のまま）。
            for (int i = 0; i < PLAYER_ABILITIES_SAVED_BYTES; i++) {
                uint8_t ability;
                rd_byte(&ability);
                player_abilities_restore_byte(i, ability);
            }
            // 光る手も（#18-12-13C まで f_ptr 越しだったが）窓口へ。器の番地を要るので
            // いったん局所で受けて、そのまま窓口に渡す（0/1 に丸めない ——
            // src/player_glowing_hands.h）。
            uint8_t saved_glowing_hands;
            rd_byte(&saved_glowing_hands);
            player_glowing_hands_restore(saved_glowing_hands);
            // あと何個覚えられるかも窓口へ。光る手と同じ形 —— 読みは器の番地を
            // 要るのでいったん局所で受け、そのまま渡す（0 も 255 もそのまま。
            // 留めはもとから無い —— src/player_spells_to_learn.h）。
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

            if ((version_min >= 2) || (version_min == 1 && patch_level >= 3)) {
                for (int i = 0; i < store_count(); i++) {
                    if (!rd_store(store_at(i))) {
                        goto error;
                    }
                }
            }

            if ((version_min >= 2) || (version_min == 1 && patch_level >= 3)) {
                rd_long(&time_saved);
            }

            if (version_min >= 2) {
                rd_string(death_cause());
            }

            if ((version_min >= 3) || (version_min == 2 && patch_level >= 2)) {
                // Read into a local first: max_score is int32_t and rd_long
                // takes a uint32_t *, so the old cast lied about the pointer's
                // type (the same fix as panel and the player's position).
                uint32_t saved_best_score;
                rd_long(&saved_best_score);
                set_best_score_so_far((int32_t)saved_best_score);
            } else {
                set_best_score_so_far(0);
            }

            if ((version_min >= 3) || (version_min == 2 && patch_level >= 2)) {
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

                dun_level = 0; // Resurrect on the town level.
                set_character_generated(true);

                // set noscore to indicate a resurrection, and don't enter
                // wizard mode
                progress_set_wizard_requested(false);
                set_score_disqualifications((int16_t)(score_disqualifications() | 0x1));
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

        rd_short((uint16_t *)&dun_level);
        uint16_t char_row_read, char_col_read;
        rd_short(&char_row_read);
        rd_short(&char_col_read);
        // 変更前は int16_t のグローバルへポインタ型を偽って直に読んでいた。
        // 同じ値を渡すために int16_t を通す（panel の 2 つと同じ形）。
        player_place((int16_t)char_row_read, (int16_t)char_col_read);
        // 変更前は int16_t のグローバルへポインタ型を偽って直に読んでいた
        // （この global の別名はこの 1 か所だけ）。上の 2 つと同じ形。
        uint16_t mon_tot_mult_read;
        rd_short(&mon_tot_mult_read);
        set_monster_breeding_count((int16_t)mon_tot_mult_read);
        // 変更前は int16_t のグローバル 2 つへポインタ型を偽って直に読んでいた
        // （この対の別名はこの 2 か所だけ）。上の 2 つと同じ形。**窓口は
        // 両方を取る 1 本**なので、半分だけ置きなおす道がそもそも無い。
        uint16_t level_height_read, level_width_read;
        rd_short(&level_height_read);
        rd_short(&level_width_read);
        set_dungeon_size((int16_t)level_height_read, (int16_t)level_width_read);
        uint16_t max_panel_rows_read, max_panel_cols_read;
        rd_short(&max_panel_rows_read);
        rd_short(&max_panel_cols_read);
        // 変更前は int16_t のグローバルへポインタ型を偽って直に読んでいた。
        // 同じ値を渡すために int16_t を通す（正しいセーブファイルなら 0〜4）。
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
            cave[ychar][xchar].cptr = char_tmp;
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
            cave[ychar][xchar].tptr = char_tmp;
            rd_byte(&char_tmp);
        }

        // read in the rest of the cave info
        cave_type *c_ptr = &cave[0][0];

        // 番地としては &cave[MAX_HEIGHT][0] と同じだが、そう書くと存在しない
        // 行 MAX_HEIGHT の添字を書くことになる（-Warray-bounds）。cave は
        // 行の配列なので、末尾のひとつ先は行の側で数える。
        const cave_type *const cave_end = (const cave_type *)END_OF(cave);

        int total_count = 0;
        while (total_count != MAX_HEIGHT * MAX_WIDTH) {
            rd_byte(&count);
            rd_byte(&char_tmp);
            for (int i = count; i > 0; i--) {
                if (c_ptr >= cave_end) {
                    goto error;
                }
                c_ptr->fval = char_tmp & 0xF;
                c_ptr->lr = (char_tmp >> 4) & 0x1;
                c_ptr->fm = (char_tmp >> 5) & 0x1;
                c_ptr->pl = (char_tmp >> 6) & 0x1;
                c_ptr->tl = (char_tmp >> 7) & 0x1;
                c_ptr++;
            }
            total_count += count;
        }

        rd_short((uint16_t *)&tcptr);
        if (tcptr > MAX_TALLOC) {
            goto error;
        }
        for (int i = MIN_TRIX; i < tcptr; i++) {
            rd_item(&t_list[i]);
        }
        // The mark is put back before it is checked, exactly as the old
        // `rd_short((uint16_t *)&mfptr)` did -- a file claiming more rows than
        // the table holds leaves the mark bogus and then fails the load.
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

        if ((version_min == 1 && patch_level < 3) || (version_min == 0)) {
            for (int i = 0; i < store_count(); i++) {
                if (!rd_store(store_at(i))) {
                    goto error;
                }
            }
        }

        // read the time that the file was saved
        if (version_min == 0 && patch_level < 16) {
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
            } else if (((!score_disqualifications()) & 0x04) && duplicate_character()) {
                (void)sprintf(temp, "This character is already on the "
                                    "scoreboard; it will not be scored again.");
                msg_print(temp);
                set_score_disqualifications((int16_t)(score_disqualifications() | 0x4));
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

// 人物の名前と生い立ちが窓口ごしに来るので const になった（#18-12-26B。
// player_bio.h の名前の項 —— 窓口が書ける番地を渡さないのが要点で、
// この関数は読むだけなのだから元から const でよかった）。
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

// 真偽値は wr_short() で 2 バイトとして書かれている（save.c:247）ので、
// 読むほうも 2 バイト消費する。ただし bool は 1 バイトなので、そこへ直接
// 読ませると隣まで書きつぶす。いったん uint16_t で受けてから詰める。
static void rd_bool(bool *ptr) {
    uint16_t value;
    rd_short(&value);
    *ptr = (value != 0);
}

// 一時的な状態の残り時間も窓口の向こうにあるので、器の番地を渡せない。
// 十八回くり返すことになるので、腹の具合と同じ形（いったん受けてから置く）
// をここに一つだけ書いておく。幅と符号の扱いは元の
// rd_short((uint16_t *)&f_ptr->blind) と同じ、16 ビットをそのまま移すだけ。
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
