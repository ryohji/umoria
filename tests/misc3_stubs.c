/* misc3.c をリンクするためのスタブ
 *
 * inven_check_num() の依存は inven_ctr / inventory / known1_p / INVEN_WIELD の
 * 4 つだけだが、C はファイル単位でリンクするので misc3.c 全体（2212 行・6 責務）
 * を持ちこむことになる。その結果、画面描画・ダンジョン・呪文・モンスターへの
 * 参照が芋づるで未解決になる。
 *
 * ここに置くのはその代役。inven_check_num() は 1 つも呼ばないので、
 * すべて「呼ばれたら何もしない／固定値を返す」で足りる。一覧はリンカに
 * 出させたもので、手で数えあげたわけではない:
 *   gcc -std=c17 -Isrc -c -o /tmp/misc3.o src/misc3.c
 *   gcc -o /tmp/t probe.c /tmp/misc3.o desc.o tables.o treasure.o player.o \
 *     2>&1 | grep 'undefined reference' | sed 's/.*to //' | sort -u
 *
 * 本物をリンクできるもの（tables.c の定数表、treasure.c のインベントリ、
 * player.c の py と各種テーブル、desc.c の known1_p）は本物を使う。
 * ここには本物と一緒にリンクできないものだけを書く。
 *
 * fixture.c と分けている理由: fixture.c は py・msg_print・insert_str・
 * prt_experience を自前で持つが、misc3.c と player.c は同じものを本物として
 * 持っている。両方をリンクすると multiple definition になるので、
 * misc3.c 系のテストは fixture.c をリンクせず、こちらを使う。
 * fixture.h の窓口（fixture_reset / fixture_set_randint）は同じものを提供する。
 */
#include <stddef.h>
#include <string.h>

#include "config.h"
#include "constant.h"
#include "types.h"

#include "fixture.h"

/* --- グローバル状態 --- */
cave_type cave[MAX_HEIGHT][MAX_WIDTH];
int16_t cur_height;
int16_t cur_width;
int16_t dun_level;
int16_t noscore;
int command_count;
int pack_heavy;
bool character_generated;
bool display_counts;
bool free_turn_flag;
bool teleport_flag;
bool total_winner;
bool weapon_heavy;
bool wizard;

/* --- 画面描画（misc3.c の表示系 4 割がこれを呼ぶ） --- */

/* msg_print も put_buffer と同じ理由で内容を記録する。値切り交渉の
 * コメント表示（store2.c の prt_comment2 / prt_comment3）は組み立てた
 * 文字列を msg_print に渡すだけなので、渡された文字列を読みとらなければ
 * ふるまいを観測できない。実装は変えずに代役側で写しとる（リンクシーム）。
 * 記録は fixture_reset() で消えるので、テスト間で漏れない。 */
#define FIXTURE_MSG_MAX 16
#define FIXTURE_MSG_LEN 160
static char fixture_messages[FIXTURE_MSG_MAX][FIXTURE_MSG_LEN + 1];
static int fixture_msg_count;

void msg_print(char *str)
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

/* msg_print が呼ばれた回数。表示の有無そのものを固定したいときに使う。 */
int fixture_message_count(void) { return fixture_msg_count; }

void prt(char *s, int r, int c) { (void)s; (void)r; (void)c; }
void prt_map(void) {}

/* put_buffer は捨てるだけでなく、書かれた内容を記録する。
 * put_misc3() のように「計算結果を画面に書くだけ」の関数は、書いた文字を
 * 読みとらなければふるまいを観測できない。実装は変えずに済ませるため、
 * 代役側で画面を模した二次元配列に写しとる（リンクシーム）。
 * 記録は fixture_reset() で消えるので、テスト間で漏れない。 */
#define FIXTURE_SCREEN_ROWS 24
#define FIXTURE_SCREEN_COLS 80
static char fixture_screen[FIXTURE_SCREEN_ROWS][FIXTURE_SCREEN_COLS + 1];

void put_buffer(char *s, int r, int c)
{
    if (s == NULL || r < 0 || r >= FIXTURE_SCREEN_ROWS || c < 0 ||
        c >= FIXTURE_SCREEN_COLS) {
        return;
    }
    for (int i = 0; s[i] != '\0' && c + i < FIXTURE_SCREEN_COLS; i++) {
        fixture_screen[r][c + i] = s[i];
    }
}

/* 指定位置から始まる記録済みの文字列を返す。空白で終端されている扱いに
 * するのではなく、put_buffer が書いた分だけを返したいので、記録用の
 * 配列は fixture_reset() で '\0' 埋めしてある。 */
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
void lite_spot(int y, int x) { (void)y; (void)x; }
void bell(void) {}

/* --- 入力 --- */
char inkey(void) { return ' '; }
int get_com(char *p, char *c) { (void)p; (void)c; return 0; }
bool get_check(char *p) { (void)p; return false; }
bool get_string(char *s, int r, int c, int l) {
    (void)s; (void)r; (void)c; (void)l;
    return false;
}

/* --- ダンジョン・モンスター --- */
int popt(void) { return 0; }
int delete_object(int y, int x) { (void)y; (void)x; return 0; }
bool in_bounds(int y, int x) { (void)y; (void)x; return true; }
void magic_treasure(int x, int level) { (void)x; (void)level; }
bool set_large(treasure_type *t) { (void)t; return false; }
void move_rec(int y1, int x1, int y2, int x2) {
    (void)y1; (void)x1; (void)y2; (void)x2;
}
int distance(int y1, int x1, int y2, int x2) {
    (void)y1; (void)x1; (void)y2; (void)x2;
    return 0;
}
creature_type *monster_get_creature(creature_handle h) { (void)h; return NULL; }
void recall_update_characteristics(creature_handle h, int defence) {
    (void)h; (void)defence;
}

/* --- プレイヤー状態の更新 --- */
void calc_bonuses(void) {}
void change_speed(int num) { (void)num; }
void check_view(void) {}
void takeoff(int item, int posn) { (void)item; (void)posn; }
bool no_light(void) { return false; }

/* --- ファイル出力 --- */
bool file_character(char *f) { (void)f; return false; }
void user_name(char *b) { (void)b; }

/* --- モンスターの行動。misc3.c の移動処理が呼ぶ --- */
void creatures(int attack) { (void)attack; }

/* --- 乱数。テストから制御できるように固定値を返す --- */
static int fixture_randint_value = 1;
int randint(int maxval) { (void)maxval; return fixture_randint_value; }
void fixture_set_randint(int value) { fixture_randint_value = value; }

void set_seed(uint32_t seed) { (void)seed; }
void reset_seed(void) {}
uint32_t randes_seed = 0;

/* --- desc.c が参照する刻印の追加。今回の対象は呼ばない --- */
void add_inscribe(inven_type *i, uint8_t flag) { (void)i; (void)flag; }

/* --- desc.c が参照する店舗の商品判定 --- */
bool general_store(int t) { (void)t; return false; }
bool armory(int t) { (void)t; return false; }
bool weaponsmith(int t) { (void)t; return false; }
bool temple(int t) { (void)t; return false; }
bool alchemist(int t) { (void)t; return false; }
bool magic_shop(int t) { (void)t; return false; }

/* --- テスト専用の初期化。本体（src/）には存在しない。
 * MU_SETUP から呼ぶことで、先行テストの影響を受けない条件を作る。
 * py は player.c の本物、inventory / object_ident / inven_ctr は
 * treasure.c の本物を指しているので、そのまま消去する。 --- */
void fixture_reset(void)
{
    extern uint8_t object_ident[];
    extern inven_type inventory[];
    extern player_type py;
    extern int16_t inven_ctr;
    extern int16_t inven_weight;

    memset(object_ident, 0, OBJECT_IDENT_SIZE);
    memset(inventory, 0, sizeof(inven_type) * INVEN_ARRAY_SIZE);
    memset(&py, 0, sizeof py);
    inven_ctr = 0;
    inven_weight = 0;
    memset(fixture_screen, 0, sizeof fixture_screen);
    memset(fixture_messages, 0, sizeof fixture_messages);
    fixture_msg_count = 0;
}
