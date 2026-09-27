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
#include "inventory.h"
#include "item_ident.h"
#include "player_base_to_hit.h"
#include "player_disarm.h"
#include "player_infra_range.h"
#include "player_level.h"
#include "player_saving_throw.h"
#include "player_speed.h"
#include "player_spells_to_learn.h"
#include "player_status_flags.h"
#include "player_timed_effects.h"

/* --- グローバル状態 --- */
cave_type cave[MAX_HEIGHT][MAX_WIDTH];
int16_t cur_height;
int16_t cur_width;
int16_t dun_level;
/* noscore はここに無い。#19B2 で misc3.c が score_disqualifications() 越しに
 * 読み書きするようになったので、実体は src/score_death.c の static である。 */
/* 打っているコマンドの覚え 3 個（command_count・default_dir・last_command）は
 * ここに無い。#18-11-7B で misc3.c の prt_state() が src/command_state.h の
 * 窓口越しに読むようになり、#18-11-7C で実体が src/command_state.c の static に
 * なった（ここで定義しても窓口には届かない別の器になるだけ）。 */
/* pack_heavy と weapon_heavy はここに無い。#18-7-4B で misc3.c が
 * pack_speed_penalty() / weapon_is_too_heavy() 越しに読み書きするように
 * なり、#18-7-4C1 で実体が src/burden.c の static になった
 * （ここで定義しても窓口には届かない別の器になるだけ）。 */
/* character_generated もここに無い。#19B3 で misc3.c が
 * character_is_generated() 越しに読むようになったので、実体は
 * src/save_state.c の static である。 */
bool display_counts;
bool free_turn_flag;
/* teleport_flag もここに無い。#18-11-4B で misc3.c の teleport() が出口で
 * teleport_done() を呼ぶようになり、#18-11-4C で実体が
 * src/pending_teleport.c の static になった
 * （ここで定義しても窓口には届かない別の器になるだけ）。 */
/* total_winner と max_score はここに無い。#18-7-1B で misc3.c が
 * player_has_won() 越しに読むようになり、#18-7-1C1 で実体が
 * src/score_death.c の static になった。 */
/* wizard はここに無い。#19B で misc3.c が progress_wizard_mode() 越しに
 * 読み書きするようになったので、実体は src/progress.c の static である
 * （ここで定義しても窓口には届かない別の器になるだけ）。 */

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

/* msg_print が呼ばれた回数。表示の有無そのものを固定したいときに使う。 */
int fixture_message_count(void) { return fixture_msg_count; }

void prt(const char *s, int r, int c) { (void)s; (void)r; (void)c; }
void prt_map(void) {}

/* put_buffer は捨てるだけでなく、書かれた内容を記録する。
 * put_misc3() のように「計算結果を画面に書くだけ」の関数は、書いた文字を
 * 読みとらなければふるまいを観測できない。実装は変えずに済ませるため、
 * 代役側で画面を模した二次元配列に写しとる（リンクシーム）。
 * 記録は fixture_reset() で消えるので、テスト間で漏れない。 */
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

/* get_com は「押されなかった」を返すだけでは足りない。gain_spells()
 * （misc3.c:1374）の MAGE の側は get_com が真を返すあいだ「どの呪文を学ぶ？」
 * を繰りかえすので、キーを返さないと**繰りかえしに 1 度も入らない**。
 * テストから並びを渡せるようにして、使いきったら 0（押されなかった）に戻す
 * （msg_print・change_speed と同じリンクシーム）。並びは fixture_reset() で
 * 空になるので、テスト間で漏れない。 */
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

/* --- ダンジョン・モンスター --- */
int popt(void) { return 0; }
int delete_object(int y, int x) { (void)y; (void)x; return 0; }
bool in_bounds(int y, int x) { (void)y; (void)x; return true; }
void magic_treasure(int x, int level) { (void)x; (void)level; }
bool set_large(treasure_type *t) { (void)t; return false; }
void move_rec(int y1, int x1, int y2, int x2) {
    (void)y1; (void)x1; (void)y2; (void)x2;
}
/* distance は代役にしてはいけない。misc3.c:2103 の teleport() が
 * `while (distance(...) > dis)` でループするので、常に 0 を返す代役では
 * ループの意味が変わる（テスト対象外の経路だが、将来テストが及んだときに
 * 誤った結果を「正しい」と固定してしまう）。
 *
 * 純粋関数なので misc1.c:210 の実装をそのまま写す。misc1.c 全体を
 * リンクするとダンジョン生成への依存が芋づるで付くため写しにしている。
 * tests/distance_test.c が本物のふるまいを 10 件で固定しているので、
 * 写しと本物が乖離すればそちらで気づける。 */
int distance(int y1, int x1, int y2, int x2) {
    int dy = y1 - y2;
    if (dy < 0) {
        dy = -dy;
    }

    int dx = x1 - x2;
    if (dx < 0) {
        dx = -dx;
    }

    return ((((dy + dx) << 1) - (dy > dx ? dx : dy)) >> 1);
}
creature_type *monster_get_creature(creature_handle h) { (void)h; return NULL; }
void recall_update_characteristics(creature_handle h, int defence) {
    (void)h; (void)defence;
}

/* --- プレイヤー状態の更新 --- */

/* change_speed と calc_bonuses は捨てるだけでなく、渡された値と呼ばれた
 * 回数を記録する。check_strength()（misc3.c:975）の重さの判定は、結果を
 * 画面（msg_print）と速度（change_speed）に流すだけで戻り値が無いので、
 * 呼ばれかたを写しとらなければふるまいを観測できない（msg_print と同じ
 * リンクシーム）。
 *
 * change_speed に渡るのは**段数の差**（新しい段数 − 覚えた段数）で、符号が
 * 向きを表す。正なら遅くなる・負なら速くなる。合計ではなく最後の値と回数を
 * 覚えるのは、1 回の check_strength が高々 1 回しか呼ばないため。
 *
 * 記録は fixture_reset() で消えるので、テスト間で漏れない。 */
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

/* change_speed が呼ばれた回数。差が 0 のときに「呼ばない」ことを見るために
 * 要る（最後の値だけでは、呼ばれていないのと 0 を渡されたのが区別できない）。 */
int fixture_speed_change_count(void) { return fixture_speed_change_calls; }

/* calc_bonuses が呼ばれた回数。武器の旗が変わったときの再計算を見る。 */
int fixture_calc_bonuses_count(void) { return fixture_bonuses_calls; }

void check_view(void) {}
void takeoff(int item, int posn) { (void)item; (void)posn; }
bool no_light(void) { return false; }

/* --- ファイル出力 --- */
bool file_character(char *f) { (void)f; return false; }
void user_name(char *b) { (void)b; }

/* --- モンスターの行動。misc3.c の移動処理が呼ぶ --- */
void creatures(int attack) { (void)attack; }

/* --- 乱数。テストから制御できるように固定値を返す ---
 *
 * 戻り値を固定するだけでは、呼びだし側が渡した上限（maxval）が正しいかを
 * 検証できない。配列の要素数を取りちがえても、返る値が同じなら気づけない。
 * だから maxval を記録して読みとれるようにする。
 * 記録は fixture_reset() で消える。 */
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

/* 直前の randint() に渡された上限。配列の要素数と一致すべき。 */
int fixture_randint_last_maxval(void) { return fixture_randint_last_max; }

/* randint() が呼ばれた回数。抽出の前後で変わってはいけない
 * （回数が変わると乱数列がずれ、ゲーム全体のふるまいが変わる）。 */
int fixture_randint_call_count(void) { return fixture_randint_calls; }

/* 種も同じ理由でここに無い。実体は src/progress.c の static（desc.c が窓口
 * 越しに読む）。 */
void set_seed(uint32_t seed) { (void)seed; }
void reset_seed(void) {}

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
 * py は player.c の本物なのでそのまま消去する。持ち物は #18-5C で
 * src/inventory.c、品目ごとの覚えは #18-9-C で src/item_ident.c が static で
 * 持つようになったので、どちらも窓口越しに消す。 --- */
void fixture_reset(void)
{
    extern player_type py;

    /* 品目ごとの覚えは #18-9-C で src/item_ident.c が static で持つように
     * なったので、セーブファイル用の生の窓口越しに消す。 */
    memset(item_kind_record_bytes(), 0, (size_t)item_kind_record_count());
    /* 持ち物と装備は 1 本の配列なので、跨ぎの窓口で全域を消す。 */
    memset(inventory_and_equipment_at(0), 0,
           sizeof(inven_type) * (size_t)inventory_and_equipment_slot_count());
    memset(&py, 0, sizeof py);
    /* 階級と経験値の 5 つは #18-12-6C で src/player_level.c が static で
     * 持つようになったので、py を消しても届かない。窓口越しに 0 へ戻す。 */
    player_set_level(0);
    player_set_experience(0);
    player_set_max_experience(0);
    player_set_experience_fraction(0);
    player_set_experience_factor(0);
    /* 状態の旗の 1 語も #18-12-7C で src/player_status_flags.c の static へ
     * 移ったので、同じように窓口越しに消す。**旗が 1 つも立っていない状態が
     * 人物の走りだし** —— 30 bit を 1 回で降ろせる窓口はセーブファイル用の
     * これだけ。 */
    player_set_status_word(0);
    /* 一時的な状態の残り時間 18 個も #18-12-9C で src/player_timed_effects.c の
     * static へ移ったので、py を消しても届かない。**どれも効いていないのが
     * 人物の走りだし**で、1 件が置いた長さが次の 1 件に残らないように 18 個とも
     * 0 へ戻す（印の 1 語と同じ理由）。 */
    for (int effect = 0; effect < PLAYER_TIMED_COUNT; effect++) {
        player_timed_clear((player_timed_effect)effect);
    }
    /* いまの速さも #18-12-11C で src/player_speed.c の static へ移ったので、
     * py を消しても届かない。**ふつうの速さ（0 段）が人物の走りだし**で、
     * 1 件が置いた段数が次の 1 件に残らないように窓口越しに戻す。**どのテストも
     * 速さを読まないので、外してもレッドにはならない** —— それでも足すのは、
     * 上の約束（各件は 0 から始まる）を黙って嘘にしないため。 */
    player_speed_set(0);
    /* 赤外視の距離も #18-12-12C で src/player_infra_range.c の static へ移った。
     * put_misc3() は毎回 xinfra を書くが、**それを読むのは xinfra の 1 件だけで、
     * その件は自分で 3 ます を置いてから読む**。だから外してもレッドにはならない
     * （実際に外して確かめた）。それでも足すのは、上の約束（各件は 0 から
     * 始まる）を黙って嘘にしないため —— 速さの 1 行と同じ理由。 */
    /* あと何個呪文を覚えられるかも #18-12-14C で
     * src/player_spells_to_learn.c の static へ移った。**この 1 行は外すと
     * 本当にレッドになる**（速さと赤外視の 2 行とは違う。実際に外して
     * 2 件が落ちるのを確かめた）—— calc_spells() が「学べるようになった」と
     * 告げるのは**前に置いた数が 0 だったとき**だけなので、前の件が置いた数が
     * 残っていると message が出ず、それを読む 2 件が落ちる。
     * **0 が人物の走りだし**（戦士はずっと 0）。 */
    player_spells_to_learn_set(0);
    /* 素の命中力の 2 本も #18-12-19C で src/player_base_to_hit.c の static へ
     * 移ったので、py を消しても届かない。**外してもレッドにはならない** ——
     * この数を読む 3 件（xbth の 2 つと xbthb）はどれも自分で 24 を置いてから
     * 読むので、前の件が残した数を見る件が無い（実際に外して確かめた。
     * 速さと赤外視の 2 行と同じ側で、呪文の 1 行とは違う）。それでも足すのは、
     * 上の約束（各件は 0 から始まる）を黙って嘘にしないため。
     * **2 本まとめて 0 へ**（どちらも種族を選ぶまで 0）。 */
    player_base_to_hit_set(0, 0);
    /* 罠と鍵をはずす腕も #18-12-20C で src/player_disarm.c の static へ移った。
     * **この 1 行は外すと本当にレッドになる**（呪文の 1 行と同じ側で、素の
     * 命中力・速さ・赤外視の 3 行とは違う）—— class_level_adj の列を見る
     * xdis_uses_the_disarm_column_of_class_level_adj は自分で腕を置かず、
     * 階級の項だけを見たいので 0 から始まることに頼っている。前の 2 件が
     * 置いた 40 と 32 が残ると "Fair" のはずが "Excellent" になる
     * （C の段で実際にこの 1 件が落ちて気づいた）。
     * **0 が人物の走りだし**（Human は種族の土台も創成時の下駄も 0）。 */
    player_disarm_set(0);
    /* 抵抗も #18-12-21C で src/player_saving_throw.c の static へ移ったので、
     * py を消しても届かない。**外してもレッドにはならない —— ただし今の並びの
     * おかげでしかない**（下調べでは「外すと落ちる」と読んでいたが外れた）。
     * 0 から始まることに頼っている件が 3 つある
     * （xdev_uses_the_device_column_of_class_level_adj・
     * xsave_uses_the_save_column_of_class_level_adj・
     * xsave_class_bonus_divides_evenly_when_product_is_multiple_of_three）が、
     * その前に数を置く 5 件の**最後が置くのがたまたま 0**（xdev_is_based_on_
     * save_not_disarm）なので今は通る。そこを 30 に変えると 3 件とも落ちるのを
     * 確かめた —— **並びが変わったら落ちる側**で、この 1 行がそれを止める。
     * **0 が人物の走りだし**（Human は種族の土台が 0 で、焼きこむ下駄も無い）。 */
    player_saving_throw_set(0);
    inventory_set_count(0);
    inventory_set_weight(0);
    memset(fixture_screen, 0, sizeof fixture_screen);
    fixture_randint_last_max = 0;
    fixture_randint_calls = 0;
    memset(fixture_messages, 0, sizeof fixture_messages);
    fixture_msg_count = 0;
    fixture_speed_change_last = 0;
    fixture_speed_change_calls = 0;
    fixture_bonuses_calls = 0;
    fixture_keys[0] = '\0';
    fixture_keys_next = 0;
}
