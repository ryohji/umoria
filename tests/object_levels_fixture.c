// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* object_levels_fixture.c -- src/item/object_levels.c と src/dungeon/object_alloc.c を試すテストの足場
 * （object_levels_test）
 *
 * fixture_reset() は、リンクされる窓口の状態を 0 に戻す。
 * 代役は、対象とその呼び先が呼ぶ名前のうち、ライブラリに無いもの
 * （fixture.h の窓口など）と、本物を引くと画面（ncurses）などが
 * 芋づるで付いてくるもの。
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "config.h"
#include "constant.h"
#include "types.h"

#include "fixture.h"
#include "inventory.h"
#include "item_ident.h"
#include "player_bio.h"
#include "player_class.h"
#include "player_level.h"
#include "player_race.h"
#include "player_speed.h"
#include "player_spells_to_learn.h"
#include "player_status_flags.h"
#include "player_timed_effects.h"

bool display_counts;
bool free_turn_flag;

/* msg_print は渡された文字列を記録する。表示しか結果を残さない
 * 関数を観測するため。 */
#define FIXTURE_MSG_MAX 16
#define FIXTURE_MSG_LEN 160
static char fixture_messages[FIXTURE_MSG_MAX][FIXTURE_MSG_LEN + 1];
static int fixture_msg_count;

void msg_print(const char *str)
{
    if (str == NULL || fixture_msg_count >= FIXTURE_MSG_MAX) {
        return;
    }
    strncpy(fixture_messages[fixture_msg_count], str, FIXTURE_MSG_LEN);
    fixture_messages[fixture_msg_count][FIXTURE_MSG_LEN] = '\0';
    fixture_msg_count++;
}

/* index 番目に msg_print へ渡された文字列を返す。まだ無ければ空文字列。 */
const char *fixture_message_text(int index)
{
    if (index < 0 || index >= fixture_msg_count) {
        return "";
    }
    return fixture_messages[index];
}

/* msg_print が呼ばれた回数。 */
int fixture_message_count(void) { return fixture_msg_count; }

void prt(const char *s, int r, int c) { (void)s; (void)r; (void)c; }
void prt_map(void) {}

/* put_buffer は書かれた文字を画面を模した配列に記録する。画面に書くだけの
 * 関数を観測するため。 */
#define FIXTURE_SCREEN_ROWS 24
#define FIXTURE_SCREEN_COLS 80
static char fixture_screen[FIXTURE_SCREEN_ROWS][FIXTURE_SCREEN_COLS + 1];

void put_buffer(const char *s, int r, int c)
{
    if (s == NULL || r < 0 || r >= FIXTURE_SCREEN_ROWS || c < 0 ||
        c >= FIXTURE_SCREEN_COLS) {
        return;
    }
    for (int i = 0; s[i] != '\0' && c + i < FIXTURE_SCREEN_COLS; i++) {
        fixture_screen[r][c + i] = s[i];
    }
}

/* 指定位置から、put_buffer が書いた分だけの文字列を返す。 */
const char *fixture_screen_text(int row, int col)
{
    if (row < 0 || row >= FIXTURE_SCREEN_ROWS || col < 0 ||
        col >= FIXTURE_SCREEN_COLS) {
        return "";
    }
    return &fixture_screen[row][col];
}

void clear_screen(void) {}
void erase_line(int row, int col) { (void)row; (void)col; }
void save_screen(void) {}
void restore_screen(void) {}
void bell(void) {}

/* get_com はテストが並べたキーを 1 つずつ返し、使いきったら
 * 0（押されなかった）を返す。 */
#define FIXTURE_KEYS_MAX 16
static char fixture_keys[FIXTURE_KEYS_MAX + 1];
static int fixture_keys_next;

void fixture_set_get_com_keys(const char *keys) {
    strncpy(fixture_keys, keys == NULL ? "" : keys, FIXTURE_KEYS_MAX);
    fixture_keys[FIXTURE_KEYS_MAX] = '\0';
    fixture_keys_next = 0;
}

int get_com(const char *p, char *c) {
    (void)p;
    if (fixture_keys[fixture_keys_next] == '\0') {
        return 0;
    }
    if (c != NULL) {
        *c = fixture_keys[fixture_keys_next];
    }
    fixture_keys_next++;
    return 1;
}

bool get_check(const char *p) { (void)p; return false; }
int popt(void) { return 0; }
bool in_bounds(int y, int x) { (void)y; (void)x; return true; }
void magic_treasure(int x, int level) { (void)x; (void)level; }
bool set_large(treasure_type *t) { (void)t; return false; }
bool no_light(void) { return false; }

/* randint はテストが決めた値を返し、渡された上限と呼ばれた回数を記録する。
 * 返す値は fixture_reset() では戻らない。 */
static int fixture_randint_value = 1;
static int fixture_randint_last_max = 0;
static int fixture_randint_calls = 0;

int randint(int maxval)
{
    fixture_randint_last_max = maxval;
    fixture_randint_calls++;
    return fixture_randint_value;
}

void fixture_set_randint(int value) { fixture_randint_value = value; }

/* 直前の randint() に渡された上限。 */
int fixture_randint_last_maxval(void) { return fixture_randint_last_max; }

/* randint() が呼ばれた回数。 */
int fixture_randint_call_count(void) { return fixture_randint_calls; }

void set_seed(uint32_t seed) { (void)seed; }
void reset_seed(void) {}
void add_inscribe(inven_type *i, uint8_t flag) { (void)i; (void)flag; }

bool general_store(int t) { (void)t; return false; }
bool armory(int t) { (void)t; return false; }
bool weaponsmith(int t) { (void)t; return false; }
bool temple(int t) { (void)t; return false; }
bool alchemist(int t) { (void)t; return false; }
bool magic_shop(int t) { (void)t; return false; }

/* 各テストの前に呼ぶ。窓口の状態と代役の記録を 0 に戻す。 */
void fixture_reset(void)
{
    extern player_type py;

    memset(item_kind_record_bytes(), 0, (size_t)item_kind_record_count());
    memset(inventory_and_equipment_at(0), 0,
           sizeof(inven_type) * (size_t)inventory_and_equipment_slot_count());
    memset(&py, 0, sizeof py);
    player_set_level(0);
    player_set_experience(0);
    player_set_max_experience(0);
    player_set_experience_fraction(0);
    player_set_experience_factor(0);
    player_set_status_word(0);
    for (int effect = 0; effect < PLAYER_TIMED_COUNT; effect++) {
        player_timed_clear((player_timed_effect)effect);
    }
    player_speed_set(0);
    player_spells_to_learn_set(0);
    player_race_set(0);
    player_name_set("");
    player_set_male(false);
    player_age_set(0);
    player_height_set(0);
    player_social_class_set(0);
    player_history_clear();
    player_class_set(0);
    inventory_set_count(0);
    inventory_set_weight(0);
    memset(fixture_messages, 0, sizeof fixture_messages);
    fixture_msg_count = 0;
    memset(fixture_screen, 0, sizeof fixture_screen);
    fixture_keys[0] = '\0';
    fixture_keys_next = 0;
    fixture_randint_last_max = 0;
    fixture_randint_calls = 0;
}
