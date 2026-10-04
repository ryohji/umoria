// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* セーブファイルの項目の並びを守るテスト。
 *
 * 保存するすべての項目に、ほかと重ならない値を窓口から入れて sv_write() で書き、
 * できたバイト列を頭から「項目名・幅・値」の表で読んで突きあわせる。表はいまの
 * 並びを写したもので、書き手と読み手を同じ向きに入れかえても赤になる
 * （往復させるだけのテストでは通ってしまう）。
 *
 * 書き手は save.c の static 関数なので、このテストが src/save/save.c を #include する。
 * XOR の難読化は wr_byte() が 1 つ前に書いたバイトとの XOR を書く形なので、
 * xor_byte を 0 から始めれば、書いたバイト列の隣どうしの XOR で元の値に戻る。
 * 先頭の版の 3 バイトと種の 1 バイトは _save_char() が書くので、ここには無い。
 */
#include "save.c"

#include "save_stubs.h"

#include "minunit.h"

/* --- 書いたバイト列を読む側 ------------------------------------------------ */

static uint8_t plain[1 << 16];
static size_t plain_len;
static size_t cursor;
static int mismatches;
static char first_mismatch[160];

/* sv_write() を一時ファイルに書かせ、XOR を外したバイト列を plain[] に置く。 */
static bool write_and_decode(void)
{
    fileptr = tmpfile();
    xor_byte = 0;
    start_time = 0;
    bool ok = sv_write();
    long n = ftell(fileptr);
    rewind(fileptr);
    uint8_t prev = 0;
    plain_len = 0;
    for (long i = 0; i < n && plain_len < sizeof plain; i++) {
        uint8_t c = (uint8_t)getc(fileptr);
        plain[plain_len++] = c ^ prev;
        prev = c;
    }
    (void)fclose(fileptr);
    fileptr = NULL;
    cursor = 0;
    mismatches = 0;
    first_mismatch[0] = '\0';
    return ok;
}

static void mismatch(const char *name, size_t at, uint32_t want, uint32_t got)
{
    if (mismatches++ == 0) {
        (void)snprintf(first_mismatch, sizeof first_mismatch,
                       "offset %zu: %s want %lu got %lu", at, name,
                       (unsigned long)want, (unsigned long)got);
    }
}

/* 下位のバイトが先（wr_short・wr_long と同じ）。 */
static uint32_t take(int width)
{
    uint32_t v = 0;
    for (int i = 0; i < width; i++) {
        uint32_t b = cursor < plain_len ? plain[cursor] : 0;
        v |= b << (8 * i);
        cursor++;
    }
    return v;
}

static void expect_n(const char *name, int width, uint32_t want)
{
    size_t at = cursor;
    uint32_t got = take(width);
    if (got != want) {
        mismatch(name, at, want, got);
    }
}

static void expect_byte(const char *name, uint8_t want) { expect_n(name, 1, want); }
static void expect_short(const char *name, uint16_t want) { expect_n(name, 2, want); }
static void expect_long(const char *name, uint32_t want) { expect_n(name, 4, want); }

/* 文字列は終端の 0 まで書かれる。 */
static void expect_string(const char *name, const char *want)
{
    size_t at = cursor;
    size_t n = strlen(want) + 1;
    if (cursor + n > plain_len || memcmp(plain + cursor, want, n) != 0) {
        mismatch(name, at, 0, 0);
    }
    cursor = at + n;
}

/* 時刻は書いた瞬間の time()。前後で取った時刻の間にあることだけを見る。 */
static void expect_time(const char *name, uint32_t before, uint32_t after)
{
    size_t at = cursor;
    uint32_t got = take(4);
    if (got < before || got > after) {
        mismatch(name, at, before, got);
    }
}

static void expect_end(void)
{
    if (cursor != plain_len) {
        mismatch("end of file", cursor, (uint32_t)cursor, (uint32_t)plain_len);
    }
}

/* --- 書く側の状態 --------------------------------------------------------- */

/* 項目ごとに番号 i を振り、幅ごとにほかと重ならない値にする。入れる側と
 * 突きあわせる側が同じ番号を使う。 */
#define B(i) ((uint8_t)(0x40 + (i)))
#define W(i) ((uint16_t)(0x2000 + (i)))
#define L(i) ((uint32_t)(0x30000000u + (i)))

/* 品物は 19 項目。seed は 20 ずつ離して使う。 */
static void fill_item(inven_type *t, int seed)
{
    t->index = W(seed);
    t->name2 = B(seed + 1);
    (void)snprintf(t->inscrip, sizeof t->inscrip, "i%d", seed);
    t->flags = L(seed + 2);
    t->tval = B(seed + 3);
    t->tchar = B(seed + 4);
    t->p1 = (int16_t)W(seed + 5);
    t->cost = (int32_t)L(seed + 6);
    t->subval = B(seed + 7);
    t->number = B(seed + 8);
    t->weight = W(seed + 9);
    t->tohit = (int16_t)W(seed + 10);
    t->todam = (int16_t)W(seed + 11);
    t->ac = (int16_t)W(seed + 12);
    t->toac = (int16_t)W(seed + 13);
    t->damage[0] = B(seed + 14);
    t->damage[1] = B(seed + 15);
    t->level = B(seed + 16);
    t->ident = B(seed + 17);
}

static void expect_item(int seed)
{
    char inscrip[16];
    (void)snprintf(inscrip, sizeof inscrip, "i%d", seed);
    expect_short("item.index", W(seed));
    expect_byte("item.name2", B(seed + 1));
    expect_string("item.inscrip", inscrip);
    expect_long("item.flags", L(seed + 2));
    expect_byte("item.tval", B(seed + 3));
    expect_byte("item.tchar", B(seed + 4));
    expect_short("item.p1", W(seed + 5));
    expect_long("item.cost", L(seed + 6));
    expect_byte("item.subval", B(seed + 7));
    expect_byte("item.number", B(seed + 8));
    expect_short("item.weight", W(seed + 9));
    expect_short("item.tohit", W(seed + 10));
    expect_short("item.todam", W(seed + 11));
    expect_short("item.ac", W(seed + 12));
    expect_short("item.toac", W(seed + 13));
    expect_byte("item.damage[0]", B(seed + 14));
    expect_byte("item.damage[1]", B(seed + 15));
    expect_byte("item.level", B(seed + 16));
    expect_byte("item.ident", B(seed + 17));
}

/* 記憶を持つモンスターは 2 種だけ。ほかは全部 0 で、書かれない。 */
static const int remembered[2] = {3, 177};

static void fill_recall(void)
{
    for (int i = 0; i < MAX_CREATURES; i++) {
        memset(recall_get(monster_make_creature_handle((uint16_t)i)), 0, sizeof(recall_type));
    }
    for (int k = 0; k < 2; k++) {
        recall_type *r = recall_get(monster_make_creature_handle((uint16_t)remembered[k]));
        int i = 20 * k;
        r->r_cmove = L(i);
        r->r_spells = L(i + 1);
        r->r_kills = W(i + 2);
        r->r_deaths = W(i + 3);
        r->r_cdefense = W(i + 4);
        r->r_wake = B(i + 5);
        r->r_ignore = B(i + 6);
        for (int a = 0; a < MAX_MON_NATTACK; a++) {
            r->r_attacks[a] = B(i + 7 + a);
        }
    }
}

static void expect_recall(void)
{
    for (int k = 0; k < 2; k++) {
        int i = 20 * k;
        expect_short("recall index", (uint16_t)remembered[k]);
        expect_long("r_cmove", L(i));
        expect_long("r_spells", L(i + 1));
        expect_short("r_kills", W(i + 2));
        expect_short("r_deaths", W(i + 3));
        expect_short("r_cdefense", W(i + 4));
        expect_byte("r_wake", B(i + 5));
        expect_byte("r_ignore", B(i + 6));
        for (int a = 0; a < MAX_MON_NATTACK; a++) {
            expect_byte("r_attacks", B(i + 7 + a));
        }
    }
    expect_short("recall sentinel", 0xFFFF);
}

/* 人物の記録。番号は 100 から。バイトの項目は B() が 255 を超えないよう 0 から。 */
static const player_timed_effect timed_before_food[3] = {
    PLAYER_TIMED_BLINDNESS, PLAYER_TIMED_PARALYSIS, PLAYER_TIMED_CONFUSION,
};
static const player_timed_effect timed_after_speed[14] = {
    PLAYER_TIMED_HASTE, PLAYER_TIMED_SLOWNESS, PLAYER_TIMED_FEAR, PLAYER_TIMED_POISON,
    PLAYER_TIMED_HALLUCINATION, PLAYER_TIMED_PROTECTION_FROM_EVIL, PLAYER_TIMED_INVULNERABILITY,
    PLAYER_TIMED_HEROISM, PLAYER_TIMED_SUPER_HEROISM, PLAYER_TIMED_BLESSING,
    PLAYER_TIMED_HEAT_RESISTANCE, PLAYER_TIMED_COLD_RESISTANCE, PLAYER_TIMED_SEEING_INVISIBLE,
    PLAYER_TIMED_WORD_OF_RECALL,
};

static void fill_player(void)
{
    player_name_set("Tester");
    player_set_male(true);
    player_set_gold((int32_t)L(100));
    player_set_max_experience((int32_t)L(101));
    player_set_experience((int32_t)L(102));
    player_set_experience_fraction(W(103));
    player_age_set(W(104));
    player_height_set(W(105));
    player_body_weight_set(W(106));
    player_set_level(W(107));
    player_max_depth_set(W(108));
    player_search_chance_set(W(109));
    player_search_frequency_set(W(110));
    player_base_to_hit_set(W(111), W(112));
    player_set_max_mana((int16_t)W(113));
    player_set_max_hp((int16_t)W(114));
    player_attack_bonuses_set(W(115), W(116));
    player_armour_class_set_parts(W(117), W(118));
    player_display_set_to_hit((int16_t)W(119));
    player_display_set_to_dam((int16_t)W(120));
    player_display_set_ac((int16_t)W(121));
    player_display_set_to_ac((int16_t)W(122));
    player_disarm_set(W(123));
    player_saving_throw_set(W(124));
    player_social_class_set(W(125));
    player_stealth_set(W(126));
    player_class_set(B(0));
    player_race_set(B(1));
    player_hit_die_set(B(2));
    player_set_experience_factor(B(3));
    player_set_mana((int16_t)W(127));
    player_set_mana_fraction(W(128));
    player_set_hp((int16_t)W(129));
    player_set_hp_fraction(W(130));
    for (int i = 0; i < PLAYER_HISTORY_LINES; i++) {
        char line[16];
        (void)snprintf(line, sizeof line, "history %d", i);
        player_history_line_set(i, line);
    }
    for (int i = 0; i < 6; i++) {
        player_stat_set_max(i, B(10 + i));
        player_stat_set_cur(i, B(20 + i));
        player_stat_set_mod(i, (int16_t)W(131 + i));
        player_stat_set_use(i, B(30 + i));
    }
    player_rest_set(W(140));
    for (int i = 0; i < 3; i++) {
        player_timed_set(timed_before_food[i], W(141 + i));
    }
    player_set_food((int16_t)W(144));
    player_set_digestion((int16_t)W(145));
    dead_protection_bytes = (int16_t)W(146);
    player_speed_set(W(147));
    for (int i = 0; i < 14; i++) {
        player_timed_set(timed_after_speed[i], W(148 + i));
    }
    player_infra_range_set(W(162));
    player_timed_set(PLAYER_TIMED_INFRA_VISION, W(163));
    for (int i = 0; i < PLAYER_ABILITIES_SAVED_BYTES; i++) {
        player_abilities_restore_byte(i, B(40 + i));
    }
    player_glowing_hands_restore(B(60));
    player_spells_to_learn_set(B(61));
    /* 状態の語は時限の効果を入れたあとに決める（窓口がほかの値を動かさないように）。 */
    player_set_status_word(L(164));
}

static void expect_player(void)
{
    expect_string("name", "Tester");
    expect_byte("male", 1);
    expect_long("gold", L(100));
    expect_long("max_exp", L(101));
    expect_long("exp", L(102));
    expect_short("exp_frac", W(103));
    expect_short("age", W(104));
    expect_short("height", W(105));
    expect_short("body_weight", W(106));
    expect_short("level", W(107));
    expect_short("max_depth", W(108));
    expect_short("search_chance", W(109));
    expect_short("search_frequency", W(110));
    expect_short("base_to_hit", W(111));
    expect_short("base_to_hit_with_bows", W(112));
    expect_short("max_mana", W(113));
    expect_short("max_hp", W(114));
    expect_short("to_hit_bonus", W(115));
    expect_short("to_damage_bonus", W(116));
    expect_short("armour_class_armour", W(117));
    expect_short("armour_class_magical", W(118));
    expect_short("display_to_hit", W(119));
    expect_short("display_to_dam", W(120));
    expect_short("display_ac", W(121));
    expect_short("display_to_ac", W(122));
    expect_short("disarm", W(123));
    expect_short("saving_throw", W(124));
    expect_short("social_class", W(125));
    expect_short("stealth", W(126));
    expect_byte("class", B(0));
    expect_byte("race", B(1));
    expect_byte("hit_die", B(2));
    expect_byte("experience_factor", B(3));
    expect_short("mana", W(127));
    expect_short("mana_fraction", W(128));
    expect_short("hp", W(129));
    expect_short("hp_fraction", W(130));
    for (int i = 0; i < PLAYER_HISTORY_LINES; i++) {
        char line[16];
        (void)snprintf(line, sizeof line, "history %d", i);
        expect_string("history", line);
    }
    for (int i = 0; i < 6; i++) {
        expect_byte("stat max", B(10 + i));
    }
    for (int i = 0; i < 6; i++) {
        expect_byte("stat cur", B(20 + i));
    }
    for (int i = 0; i < 6; i++) {
        expect_short("stat mod", W(131 + i));
    }
    for (int i = 0; i < 6; i++) {
        expect_byte("stat use", B(30 + i));
    }
    expect_long("status word", L(164));
    expect_short("rest", W(140));
    for (int i = 0; i < 3; i++) {
        expect_short("timed (before food)", W(141 + i));
    }
    expect_short("food", W(144));
    expect_short("digestion", W(145));
    expect_short("dead 2-byte hole", W(146));
    expect_short("speed", W(147));
    for (int i = 0; i < 14; i++) {
        expect_short("timed (after speed)", W(148 + i));
    }
    expect_short("infra_range", W(162));
    expect_short("timed infra-vision", W(163));
    for (int i = 0; i < PLAYER_ABILITIES_SAVED_BYTES; i++) {
        expect_byte("ability byte", B(40 + i));
    }
    expect_byte("glowing_hands", B(60));
    expect_byte("spells_to_learn", B(61));
}

/* 持ち物・呪文・メッセージ・得点・店・時刻。番号は 200 から、品物の seed は 300 から。 */
#define INVENTORY_ITEMS 3
#define STORE_ITEMS(i) ((i) % 3)

static void fill_rest(void)
{
    set_missile_serial((int16_t)W(200));
    progress_set_turn((int32_t)L(201));
    inventory_set_count(INVENTORY_ITEMS);
    for (int i = 0; i < INVENTORY_ITEMS; i++) {
        fill_item(inventory_at(i), 300 + 20 * i);
    }
    for (int i = equipment_first_slot(); i < equipment_end_slot(); i++) {
        fill_item(equipment_at(i), 400 + 20 * (i - equipment_first_slot()));
    }
    inventory_set_weight(W(202));
    equipment_set_count(W(203));
    spells_set_learned_bits(L(204));
    spells_set_worked_bits(L(205));
    spells_set_forgotten_bits(L(206));
    for (int i = 0; i < 32; i++) {
        spell_order_bytes()[i] = (uint8_t)(100 + i);
    }
    for (int i = 0; i < item_kind_record_count(); i++) {
        item_kind_record_bytes()[i] = (uint8_t)(i * 7 + 1);
    }
    progress_set_color_seed(L(207));
    progress_set_town_seed(L(208));
    msg_history_set_newest_slot(W(209) & 0x0F);
    for (int i = 0; i < msg_history_slot_count(); i++) {
        (void)snprintf(msg_history_slot(i), 16, "message %d", i % 100);
    }
    set_panic_save(true);
    set_player_has_won(true);
    set_score_disqualifications((int16_t)W(210));
    for (int i = 0; i < MAX_PLAYER_LEVEL; i++) {
        hp_table_slots()[i] = W(220 + i);
    }
    for (int i = 0; i < store_count(); i++) {
        store_type *st = store_at(i);
        int k = 300 + 10 * i;
        st->store_open = (int32_t)L(k);
        st->insult_cur = (int16_t)W(k + 1);
        st->owner = B(k + 2);
        st->store_ctr = (uint8_t)STORE_ITEMS(i);
        st->good_buy = W(k + 3);
        st->bad_buy = W(k + 4);
        for (int j = 0; j < STORE_ITEMS(i); j++) {
            st->store_inven[j].scost = (int32_t)L(k + 5 + j);
            fill_item(&st->store_inven[j].sitem, 700 + 60 * i + 20 * j);
        }
    }
    (void)strcpy(death_cause(), "a layout test");
    save_stubs_set_total_points((int32_t)L(400));
    set_character_birth_date((int32_t)L(401));
}

static void expect_rest(uint32_t before, uint32_t after)
{
    expect_short("missile_serial", W(200));
    expect_long("turn", L(201));
    expect_short("inventory_count", INVENTORY_ITEMS);
    for (int i = 0; i < INVENTORY_ITEMS; i++) {
        expect_item(300 + 20 * i);
    }
    for (int i = equipment_first_slot(); i < equipment_end_slot(); i++) {
        expect_item(400 + 20 * (i - equipment_first_slot()));
    }
    expect_short("inventory_weight", W(202));
    expect_short("equipment_count", W(203));
    expect_long("spells learned", L(204));
    expect_long("spells worked", L(205));
    expect_long("spells forgotten", L(206));
    for (int i = 0; i < 32; i++) {
        expect_byte("spell order", (uint8_t)(100 + i));
    }
    for (int i = 0; i < item_kind_record_count(); i++) {
        expect_byte("item kind record", (uint8_t)(i * 7 + 1));
    }
    expect_long("color seed", L(207));
    expect_long("town seed", L(208));
    expect_short("newest message slot", W(209) & 0x0F);
    for (int i = 0; i < msg_history_slot_count(); i++) {
        char text[16];
        (void)snprintf(text, sizeof text, "message %d", i % 100);
        expect_string("message", text);
    }
    expect_short("panic save", 1);
    expect_short("has won", 1);
    expect_short("score disqualifications", W(210));
    for (int i = 0; i < MAX_PLAYER_LEVEL; i++) {
        expect_short("hp table", W(220 + i));
    }
    for (int i = 0; i < store_count(); i++) {
        int k = 300 + 10 * i;
        expect_long("store_open", L(k));
        expect_short("insult_cur", W(k + 1));
        expect_byte("owner", B(k + 2));
        expect_byte("store_ctr", (uint8_t)STORE_ITEMS(i));
        expect_short("good_buy", W(k + 3));
        expect_short("bad_buy", W(k + 4));
        for (int j = 0; j < STORE_ITEMS(i); j++) {
            expect_long("scost", L(k + 5 + j));
            expect_item(700 + 60 * i + 20 * j);
        }
    }
    expect_time("time saved", before, after);
    expect_string("death cause", "a layout test");
    expect_long("total points", L(400));
    expect_long("birth date", L(401));
}

/* 階。生きている人物のファイルだけにある。番号は 500 から、品物の seed は 1200 から。 */
#define FLOOR_ITEMS 2
#define MONSTERS 2

/* マスの値を決める。人物・品物の印は数マス、地形は 2 行だけ違う値。 */
static void fill_level(void)
{
    set_dungeon_level(W(500));
    player_place(10, 20);
    set_monster_breeding_count((int16_t)W(501));
    set_dungeon_size(W(502), W(503));
    panel_set_max_indexes(W(504), W(505));

    dungeon_map_reset();
    square_at(5, 6)->cptr = 2;
    square_at(7, 8)->cptr = 3;
    square_at(9, 10)->tptr = 1;
    square_at(11, 12)->tptr = 2;
    for (int x = 0; x < MAX_WIDTH; x++) {
        cave_type *c = square_at(1, x);
        c->fval = 1;
        c->lr = 1;
        c->pl = (x % 2) != 0;
        square_at(2, x)->fval = (uint8_t)(x % 5);
        square_at(2, x)->tl = 1;
    }

    set_floor_items_used(MIN_TRIX + FLOOR_ITEMS);
    for (int i = 0; i < FLOOR_ITEMS; i++) {
        fill_item(floor_item_at(MIN_TRIX + i), 1200 + 20 * i);
    }
    set_monster_list_used(MIN_MONIX + MONSTERS);
    for (int i = 0; i < MONSTERS; i++) {
        monster_type *m = monster_list_at(MIN_MONIX + i);
        int k = 510 + 10 * i;
        m->hp = (int16_t)W(k);
        m->csleep = (int16_t)W(k + 1);
        m->cspeed = (int16_t)W(k + 2);
        m->creature.place = W(k + 3);
        m->fy = B(k + 4);
        m->fx = B(k + 5);
        m->cdis = B(k + 6);
        m->ml = B(k + 7);
        m->stunned = B(k + 8);
        m->confused = B(k + 9);
    }
}

/* 地形の 1 バイト（fval と 4 つの旗）の並びを、変わり目ごとに (数, 値) の組で書く。
 * 数は 255 で打ちきる。最初の組は (0, 0) から始まりうる（本体の注釈のとおり）。 */
static void expect_terrain_runs(void)
{
    int count = 0;
    uint8_t prev = 0;
    for (int y = 0; y < MAX_HEIGHT; y++) {
        for (int x = 0; x < MAX_WIDTH; x++) {
            const cave_type *c = square_at(y, x);
            uint8_t v = (uint8_t)(c->fval | (c->lr << 4) | (c->fm << 5) | (c->pl << 6) | (c->tl << 7));
            if (v != prev || count == MAX_UCHAR) {
                expect_byte("terrain run count", (uint8_t)count);
                expect_byte("terrain run value", prev);
                prev = v;
                count = 1;
            } else {
                count++;
            }
        }
    }
    expect_byte("terrain run count", (uint8_t)count);
    expect_byte("terrain run value", prev);
}

static void expect_level(void)
{
    expect_short("dungeon_level", W(500));
    expect_short("player_row", 10);
    expect_short("player_col", 20);
    expect_short("breeding count", W(501));
    expect_short("dungeon_height", W(502));
    expect_short("dungeon_width", W(503));
    expect_short("panel max row", W(504));
    expect_short("panel max col", W(505));

    expect_byte("cptr y", 5);
    expect_byte("cptr x", 6);
    expect_byte("cptr", 2);
    expect_byte("cptr y", 7);
    expect_byte("cptr x", 8);
    expect_byte("cptr", 3);
    expect_byte("end of cptr", 0xFF);
    expect_byte("tptr y", 9);
    expect_byte("tptr x", 10);
    expect_byte("tptr", 1);
    expect_byte("tptr y", 11);
    expect_byte("tptr x", 12);
    expect_byte("tptr", 2);
    expect_byte("end of tptr", 0xFF);
    expect_terrain_runs();

    expect_short("floor items used", MIN_TRIX + FLOOR_ITEMS);
    for (int i = 0; i < FLOOR_ITEMS; i++) {
        expect_item(1200 + 20 * i);
    }
    expect_short("monsters used", MIN_MONIX + MONSTERS);
    for (int i = 0; i < MONSTERS; i++) {
        int k = 510 + 10 * i;
        expect_short("monster hp", W(k));
        expect_short("monster csleep", W(k + 1));
        expect_short("monster cspeed", W(k + 2));
        expect_short("monster creature", W(k + 3));
        expect_byte("monster fy", B(k + 4));
        expect_byte("monster fx", B(k + 5));
        expect_byte("monster cdis", B(k + 6));
        expect_byte("monster ml", B(k + 7));
        expect_byte("monster stunned", B(k + 8));
        expect_byte("monster confused", B(k + 9));
    }
}

/* --- テスト ---------------------------------------------------------------- */

/* 選択肢の 11 ビット。どの選択肢がどのビットかは options.c が決めるので、ここでは
 * 「読んだ値が書いた値に戻る」形で決める。 */
#define OPTION_BITS 0x2A5u

static void fill_all(bool dead)
{
    game_options_unpack(OPTION_BITS);
    fill_recall();
    fill_player();
    fill_rest();
    fill_level();
    set_player_dead(dead);
}

static void expect_head(bool dead)
{
    expect_recall();
    expect_long("flags", OPTION_BITS | (dead ? 0x80000000u : 0) | 0x40000000u);
    expect_player();
}

/* 死んだ人物のファイルは誕生日で終わる（階は書かない）。 */
TEST(dead_character_file_has_every_field_in_order) {
    fill_all(true);
    uint32_t before = (uint32_t)time(NULL);
    bool ok = write_and_decode();
    uint32_t after = (uint32_t)time(NULL);
    expect_head(true);
    expect_rest(before, after);
    expect_end();
    ASSERT_EQ_STR(ok ? first_mismatch : "sv_write() failed", "");
}

/* 生きている人物のファイルは、誕生日のあとに階が続く。 */
TEST(living_character_file_has_every_field_in_order) {
    fill_all(false);
    uint32_t before = (uint32_t)time(NULL);
    bool ok = write_and_decode();
    uint32_t after = (uint32_t)time(NULL);
    expect_head(false);
    expect_rest(before, after);
    expect_level();
    expect_end();
    ASSERT_EQ_STR(ok ? first_mismatch : "sv_write() failed", "");
}

int main(void)
{
    RUN_TEST(dead_character_file_has_every_field_in_order);
    RUN_TEST(living_character_file_has_every_field_in_order);
    return TEST_SUMMARY();
}
