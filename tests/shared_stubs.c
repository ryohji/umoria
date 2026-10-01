// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* shared_stubs.c -- 対象ごとの足場（*_fixture.c）が共有する代役
 *
 * 対象が呼ぶ名前のうち、ライブラリに無いもの（takeoff など）と、本物を引くと
 * 画面（ncurses）などが芋づるで付いてくるもの（io.o・rnd.o など）。
 * 画面・メッセージ・乱数・速さの代役は、呼ばれかたを記録する。
 * 記録は shared_stubs_reset() で消える（各足場の fixture_reset() が呼ぶ）。
 *
 * どの名前を代役で埋めているかは scripts/link_units.py --shadows で出る。
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "config.h"
#include "constant.h"
#include "types.h"

#include "fixture.h"
#include "shared_stubs.h"

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
void clear_from(int row) { (void)row; }
void erase_line(int row, int col) { (void)row; (void)col; }
void save_screen(void) {}
void restore_screen(void) {}
void bell(void) {}
char inkey(void) { return ' '; }

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

bool get_string(char *s, int r, int c, int l) {
    (void)s; (void)r; (void)c; (void)l;
    return false;
}

int popt(void) { return 0; }
int delete_object(int y, int x) { (void)y; (void)x; return 0; }
bool in_bounds(int y, int x) { (void)y; (void)x; return true; }
void magic_treasure(int x, int level) { (void)x; (void)level; }

/* set_large は訊かれた品物と回数を記録し、並べた答え（'y' で大きい）を順に
 * 返す。答えを使いきったら「大きくない」。 */
static const char *fixture_large_answers = "";
static const treasure_type *fixture_large_last;
static int fixture_large_calls;

bool set_large(treasure_type *t) {
    fixture_large_last = t;
    fixture_large_calls++;
    if (*fixture_large_answers == '\0') {
        return false;
    }
    return *fixture_large_answers++ == 'y';
}

void fixture_set_large_answers(const char *answers) {
    fixture_large_answers = answers == NULL ? "" : answers;
}

int fixture_set_large_call_count(void) { return fixture_large_calls; }
const treasure_type *fixture_set_large_last_item(void) { return fixture_large_last; }

/* change_speed と calc_bonuses は呼ばれかたを記録する。change_speed に渡るのは
 * 段数の差（正なら遅くなる）。 */
static int fixture_speed_change_last;
static int fixture_speed_change_calls;
static int fixture_bonuses_calls;

void calc_bonuses(void) { fixture_bonuses_calls++; }

void change_speed(int num) {
    fixture_speed_change_last = num;
    fixture_speed_change_calls++;
}

/* 最後に change_speed へ渡された段数の差。まだ呼ばれていなければ 0。 */
int fixture_speed_change_last_steps(void) { return fixture_speed_change_last; }

/* change_speed が呼ばれた回数。呼ばれないことと 0 を渡されたことを
 * 区別する。 */
int fixture_speed_change_count(void) { return fixture_speed_change_calls; }

/* calc_bonuses が呼ばれた回数。 */
int fixture_calc_bonuses_count(void) { return fixture_bonuses_calls; }

void takeoff(int item, int posn) { (void)item; (void)posn; }
bool no_light(void) { return false; }
bool file_character(char *f) { (void)f; return false; }
void user_name(char *b) { (void)b; }

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

void shared_stubs_reset(void)
{
    memset(fixture_messages, 0, sizeof fixture_messages);
    fixture_msg_count = 0;
    memset(fixture_screen, 0, sizeof fixture_screen);
    fixture_keys[0] = '\0';
    fixture_keys_next = 0;
    fixture_speed_change_last = 0;
    fixture_speed_change_calls = 0;
    fixture_bonuses_calls = 0;
    fixture_randint_last_max = 0;
    fixture_randint_calls = 0;
    fixture_large_answers = "";
    fixture_large_last = NULL;
    fixture_large_calls = 0;
}
