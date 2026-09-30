// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 代役：本物をリンクするテストのために、その本物が呼ぶ先を埋めるスタブ
 *
 * ファイルの名前は misc3.c の代役だったときのまま。改名は代役の片づけ（#37）に
 * 残してある。
 *
 * これを使うのは makefile.test の足場の表で EXTRA_<name>_test = misc3_stubs.c と
 * 書いた 9 本（calc_hitpoints・calc_spells・check_strength・gain_spells・
 * haggle_comment・inven_stack・objdes・object_levels・put_misc3 の各 _test）。
 * どれも検証する本物（spellbook.o・level_ops.o・inven_ops.o・char_screen.o
 * など）をライブラリ（tests/build/libcore.a）から引く。ここに置くのは、その
 * 本物が呼ぶ先の代役で、2 種類ある。
 *
 *   1. ライブラリに無い名前。makefile.test の LIB_EXCLUDE（dungeon.c・
 *      inven_menu.c・signals.c）にしか無いもの（takeoff など）と、fixture.h
 *      の窓口（fixture_reset など）。
 *   2. 本物より先に埋める名前。画面と入力（io.o の msg_print・put_buffer・
 *      get_com など）、装備と速さ（player_bonuses.o の calc_bonuses・
 *      change_speed）、乱数（rnd.o の randint）、ダンジョンとモンスター
 *      （geometry.o の distance・in_bounds など）。本物を引くと ncurses の
 *      窓口などまで芋づるで付いてくるうえ、テストが呼ばれかたを読みとれない。
 *
 * 一覧は手で数えあげず、リンカに出させる。1 は、misc3_stubs.c を外して
 * ライブラリだけでリンクすると undefined reference に出る:
 *   make -f makefile.test libcore
 *   gcc -std=c17 -Isrc $(find src -mindepth 1 -maxdepth 1 -type d -printf '-I%p ') \
 *     -Itests -o /tmp/t tests/calc_spells_test.c tests/build/libcore.a 2>&1 |
 *     grep -o "undefined reference to \`[^']*'" | sed "s/.*\`//; s/'//" | sort -u
 * 出た名前のうち、ここに定義のあるものが 1 の代役。ほかの名前（get_item・
 * show_inven・ncurses の窓口など）は、代役を外したせいで io.o などの本物が
 * 引かれ、その先で要るようになったもので、代役があるあいだは出てこない。
 * 2 は scripts/link_units.py --shadows で出る（先に make -f makefile.test で
 * tests/build/ に .map を作っておく）。
 *
 * fixture.c と分けている理由: fixture.c は py・msg_print・insert_str・
 * prt_experience を自前で持つが、これらのテストがライブラリから引く本物
 * （player.o の py、str_insert.o の insert_str、level_ops.o の prt_experience。
 * fixture.c では画面の代役が足りず io.o の msg_print も引かれる）も同じ名前を
 * 持っている。両方をリンクすると multiple definition になるので、これらの
 * テストは fixture.c をリンクせず、こちらを使う。
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
#include "player_attack_bonuses.h"
#include "player_base_to_hit.h"
#include "player_bio.h"
#include "player_class.h"
#include "player_stealth.h"
#include "player_body_weight.h"
#include "player_disarm.h"
#include "player_infra_range.h"
#include "player_level.h"
#include "player_race.h"
#include "player_saving_throw.h"
#include "player_search_skill.h"
#include "player_speed.h"
#include "player_spells_to_learn.h"
#include "player_status_flags.h"
#include "player_timed_effects.h"

/* --- グローバル状態 --- */
/* マスの表（cave）はここに無い。#18-14-8C で置き場が src/dungeon/dungeon_map.c の
 * static に入ったので、ここで定義しても窓口には届かない別の表になるだけ。
 * recipe が src/dungeon_map.c をリンクしているのがその代わりで、misc3.c から
 * 分かれた先（object_alloc.c・inven_ops.c など）は square_at(y, x) で 1 マスを
 * 取る（下の cur_height・dun_level・t_list の註と同じ形。これでこの区分の
 * 11 個ぜんぶがこの形になった）。 */
/* この階の広さ（cur_height・cur_width）はここに無い。#18-14-5A で置き場が
 * src/dungeon/dungeon_size.c の static に入り、#18-14-5B で misc3.c が窓口越しに
 * 読むようになった（ここで定義しても窓口には届かない別の器になるだけ）。
 * recipe が src/dungeon_size.c をリンクしているのがその代わり。 */
/* いま何階か（dun_level）はここに無い。#18-14-6A で置き場が
 * src/dungeon/dungeon_level.c の static に入り、#18-14-6B で misc3.c が窓口越しに
 * 読むようになった（ここで定義しても窓口には届かない別の器になるだけ）。
 * recipe が src/dungeon_level.c をリンクしているのがその代わり。 */
/* noscore はここに無い。#19B2 で misc3.c が score_disqualifications() 越しに
 * 読み書きするようになったので、実体は src/save/score_death.c の static である。 */
/* 打っているコマンドの覚え 3 個（command_count・default_dir・last_command）は
 * ここに無い。#18-11-7B で misc3.c の prt_state() が src/ui/command_state.h の
 * 窓口越しに読むようになり、#18-11-7C で実体が src/ui/command_state.c の static に
 * なった（ここで定義しても窓口には届かない別の器になるだけ）。 */
/* pack_heavy と weapon_heavy はここに無い。#18-7-4B で misc3.c が
 * pack_speed_penalty() / weapon_is_too_heavy() 越しに読み書きするように
 * なり、#18-7-4C1 で実体が src/player/burden.c の static になった
 * （ここで定義しても窓口には届かない別の器になるだけ）。 */
/* character_generated もここに無い。#19B3 で misc3.c が
 * character_is_generated() 越しに読むようになったので、実体は
 * src/save/save_state.c の static である。 */
bool display_counts;
bool free_turn_flag;
/* teleport_flag もここに無い。#18-11-4B で misc3.c の teleport() が出口で
 * teleport_done() を呼ぶようになり、#18-11-4C で実体が
 * src/player/pending_teleport.c の static になった
 * （ここで定義しても窓口には届かない別の器になるだけ）。 */
/* total_winner と max_score はここに無い。#18-7-1B で misc3.c が
 * player_has_won() 越しに読むようになり、#18-7-1C1 で実体が
 * src/save/score_death.c の static になった。 */
/* wizard はここに無い。#19B で misc3.c が progress_wizard_mode() 越しに
 * 読み書きするようになったので、実体は src/data/progress.c の static である
 * （ここで定義しても窓口には届かない別の器になるだけ）。 */

/* --- 画面描画（もと misc3.c の 4 割だった表示系 —— いまの ui/screen_fields.c・
 * status_line.c・char_screen.c —— がこれを呼ぶ） --- */

/* msg_print も put_buffer と同じ理由で内容を記録する。値切り交渉の
 * コメント表示（store_haggle.c の prt_comment2 / prt_comment3）は組み立てた
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
 * （spellbook.c:330）の MAGE の側は get_com が真を返すあいだ「どの呪文を学ぶ？」
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
/* distance は代役にしてはいけない。player_move.c:32 の teleport() が
 * `while (distance(...) > dis)` でループするので、常に 0 を返す代役では
 * ループの意味が変わる（テスト対象外の経路だが、将来テストが及んだときに
 * 誤った結果を「正しい」と固定してしまう）。
 *
 * 純粋関数なので もと misc1.c:210（いまは dungeon/geometry.c）の実装をそのまま写す。misc1.c 全体を
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
 * 回数を記録する。check_strength()（inven_ops.c:174）の重さの判定は、結果を
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

/* --- モンスターの行動。player_move.c の teleport() が呼ぶ --- */
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

/* 種も同じ理由でここに無い。実体は src/data/progress.c の static（desc.c が窓口
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
 * src/item/inventory.c、品目ごとの覚えは #18-9-C で src/item/item_ident.c が static で
 * 持つようになったので、どちらも窓口越しに消す。 --- */
void fixture_reset(void)
{
    extern player_type py;

    /* 品目ごとの覚えは #18-9-C で src/item/item_ident.c が static で持つように
     * なったので、セーブファイル用の生の窓口越しに消す。 */
    memset(item_kind_record_bytes(), 0, (size_t)item_kind_record_count());
    /* 持ち物と装備は 1 本の配列なので、跨ぎの窓口で全域を消す。 */
    memset(inventory_and_equipment_at(0), 0,
           sizeof(inven_type) * (size_t)inventory_and_equipment_slot_count());
    memset(&py, 0, sizeof py);
    /* 階級と経験値の 5 つは #18-12-6C で src/player/player_level.c が static で
     * 持つようになったので、py を消しても届かない。窓口越しに 0 へ戻す。 */
    player_set_level(0);
    player_set_experience(0);
    player_set_max_experience(0);
    player_set_experience_fraction(0);
    player_set_experience_factor(0);
    /* 状態の旗の 1 語も #18-12-7C で src/player/player_status_flags.c の static へ
     * 移ったので、同じように窓口越しに消す。**旗が 1 つも立っていない状態が
     * 人物の走りだし** —— 30 bit を 1 回で降ろせる窓口はセーブファイル用の
     * これだけ。 */
    player_set_status_word(0);
    /* 一時的な状態の残り時間 18 個も #18-12-9C で src/player/player_timed_effects.c の
     * static へ移ったので、py を消しても届かない。**どれも効いていないのが
     * 人物の走りだし**で、1 件が置いた長さが次の 1 件に残らないように 18 個とも
     * 0 へ戻す（印の 1 語と同じ理由）。 */
    for (int effect = 0; effect < PLAYER_TIMED_COUNT; effect++) {
        player_timed_clear((player_timed_effect)effect);
    }
    /* いまの速さも #18-12-11C で src/player/player_speed.c の static へ移ったので、
     * py を消しても届かない。**ふつうの速さ（0 段）が人物の走りだし**で、
     * 1 件が置いた段数が次の 1 件に残らないように窓口越しに戻す。**どのテストも
     * 速さを読まないので、外してもレッドにはならない** —— それでも足すのは、
     * 上の約束（各件は 0 から始まる）を黙って嘘にしないため。 */
    player_speed_set(0);
    /* 赤外視の距離も #18-12-12C で src/player/player_infra_range.c の static へ移った。
     * put_misc3() は毎回 xinfra を書くが、**それを読むのは xinfra の 1 件だけで、
     * その件は自分で 3 ます を置いてから読む**。だから外してもレッドにはならない
     * （実際に外して確かめた）。それでも足すのは、上の約束（各件は 0 から
     * 始まる）を黙って嘘にしないため —— 速さの 1 行と同じ理由。 */
    /* あと何個呪文を覚えられるかも #18-12-14C で
     * src/player/player_spells_to_learn.c の static へ移った。**この 1 行は外すと
     * 本当にレッドになる**（速さと赤外視の 2 行とは違う。実際に外して
     * 2 件が落ちるのを確かめた）—— calc_spells() が「学べるようになった」と
     * 告げるのは**前に置いた数が 0 だったとき**だけなので、前の件が置いた数が
     * 残っていると message が出ず、それを読む 2 件が落ちる。
     * **0 が人物の走りだし**（戦士はずっと 0）。 */
    player_spells_to_learn_set(0);
    /* 素の命中力の 2 本も #18-12-19C で src/player/player_base_to_hit.c の static へ
     * 移ったので、py を消しても届かない。**外してもレッドにはならない** ——
     * この数を読む 3 件（xbth の 2 つと xbthb）はどれも自分で 24 を置いてから
     * 読むので、前の件が残した数を見る件が無い（実際に外して確かめた。
     * 速さと赤外視の 2 行と同じ側で、呪文の 1 行とは違う）。それでも足すのは、
     * 上の約束（各件は 0 から始まる）を黙って嘘にしないため。
     * **2 本まとめて 0 へ**（どちらも種族を選ぶまで 0）。 */
    player_base_to_hit_set(0, 0);
    /* 罠と鍵をはずす腕も #18-12-20C で src/player/player_disarm.c の static へ移った。
     * **この 1 行は外すと本当にレッドになる**（呪文の 1 行と同じ側で、素の
     * 命中力・速さ・赤外視の 3 行とは違う）—— class_level_adj の列を見る
     * xdis_uses_the_disarm_column_of_class_level_adj は自分で腕を置かず、
     * 階級の項だけを見たいので 0 から始まることに頼っている。前の 2 件が
     * 置いた 40 と 32 が残ると "Fair" のはずが "Excellent" になる
     * （C の段で実際にこの 1 件が落ちて気づいた）。
     * **0 が人物の走りだし**（Human は種族の土台も創成時の下駄も 0）。 */
    player_disarm_set(0);
    /* 抵抗も #18-12-21C で src/player/player_saving_throw.c の static へ移ったので、
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
    /* どの種族かも #18-12-22C で src/player/player_race.c の static へ移ったので、
     * py を消しても届かない。**外してもレッドにはならない —— しかも今回は
     * 並びではなく中身から言える**（21 つめは並びのおかげだった）。
     * この行番号を読む既存の件は 1 つも無く、変異を入れても 11 本のどれにも
     * 1 件も出ないのを確かめてある（読みに +1 で module の 14 件だけが落ちる）。
     * それでも足すのは、上の約束（各件は 0 から始まる）を黙って嘘にしないため。
     * **0 が人物の走りだし**で、race[] の 0 行めはちょうど Human。 */
    player_race_set(0);
    /* 体の重さも #18-12-23C で src/player/player_body_weight.c の static へ移った。
     * **この 1 行は外してもレッドにならないが、0 は既存の件の期待値に
     * 乗っている** —— check_strength_test の
     * the_limit_for_an_average_character_is_thirteen_hundred が読み返す 1300 は
     * 10 × PLAYER_WEIGHT_CAP ＋ **0** で、体重を置く件は 1 つも無い。
     * **「外しても緑」と「0 が当てにされている」は別のこと**で、21 つめ
     * （どちらも無し）とここで分かれる。だから必ず 0 に戻す。
     * **0 が人物の走りだし**（創成が重さを振るまで体重は無い）。 */
    player_body_weight_set(0);
    /* 命中と打撃の下駄も #18-12-24C で src/player/player_attack_bonuses.c の static 2 つへ
     * 移ったので、py を消しても届かない。**外してもレッドにはならない —— 21・
     * 22 つめと同じく並びのおかげで、しかも綱わたりが 1 本ある**。0 から始まる
     * ことに頼っている件が 3 つ（xbth_uses_the_bth_column_of_class_level_adj・
     * xbthb_uses_the_bthb_column_of_class_level_adj・
     * xbth_is_bth_itself_when_ptohit_is_zero）あり、下駄に 4 を置く 2 件
     * （xbth_adds_three_times_ptohit と xbthb_…）は **main() でその 3 つより
     * あとに並んでいる**から今は通る。順を入れかえたら落ちる側。
     * **対で 0 へ**（どちらも創成が能力値を振るまで 0 で、ふつうの能力値でも 0）。 */
    player_attack_bonuses_set(0, 0);
    /* 探索の腕と頻度も #18-12-25C で src/player/player_search_skill.c の static 2 つへ
     * 移った。**外してもレッドにならず、0 を当てにしている件も 1 つも無い** ——
     * 21 つめ（どちらも無し）と同じ側で、24 つめの綱わたりとは違う。この 2 つを
     * 読む既存の件は 5 つ（xfos の 4 件と xsrh の 1 件）だけで、**どれも自分が
     * 読む数を自分で置いてから put_misc3() を呼ぶ**ので、並びを変えても落ちない。
     * **確かめた** —— この 2 行を足す前にグリーン 1448 件。
     * **それでも足すのは、2 つの 0 の意味が違うことを黙って隠さないため。**
     * 腕の 0 は本当の答え（search() に 0 が渡る＝何も見つけられない）だが、
     * **頻度の 0 はふつうの答えではない** —— 本物の人物は種族の表から必ず値を
     * 持ち、randint(0) を訊く者は居ない（読み手が `<= 1` で先に囲う）。
     * 置く窓口が 2 本あるので 2 行になる（対で置く窓口は無い。wizard.c が
     * 腕だけを置くから —— player_search_skill.h）。 */
    player_search_chance_set(0);
    player_search_frequency_set(0);
    /* 人物の身上書きの 6 つも #18-12-26C で src/player/player_bio.c の static 6 つへ
     * 移ったので、py を消しても届かない。**外してもレッドにならず、0 を当てに
     * している既存の件も 1 つも無い** —— 21・25 つめと同じ側で、23 つめの
     * 「0 が期待値に乗っている」とも 24 つめの綱わたりとも違う。**この 6 つを
     * 読む既存の件は 1 つも無い**（put_character() と put_misc1() を呼ぶテストが
     * そもそも無く、6 つの窓口を名ざす既存の件も無い）。**確かめた** —— この
     * 6 行を足さずに全 1481 件がグリーンになるのを見た。
     * **それでも足すのは、6 つとも「0 が人物の走りだし」だから** ——
     * 名前も無く、男でもなく、年齢も身長も階層も 0 で、生い立ちは 4 行とも空。
     * py を memset していたころに書いてあった約束（各件は白紙の人物から
     * 始まる）を、置き場が移ったせいで黙って嘘にしないため。
     * 置く窓口が 5 本と消す窓口 1 本なので 6 行になる（`_adjust` は 1 本も
     * 無い —— 誰も年齢を足さない。player_bio.h）。 */
    player_name_set("");
    player_set_male(false);
    player_age_set(0);
    player_height_set(0);
    player_social_class_set(0);
    player_history_clear();
    /* 足音の静かさも #18-12-27C で src/player/player_stealth.c の static へ移ったので、
     * py を消しても届かない。**外してもレッドにならない** —— この数を読む既存の
     * 2 件（xstl の語）は **B で自分で置くように書きかえた**ので、走りだしの 0 に
     * よりかかっていない。**確かめた** —— この 1 行を足さずに全 1497 件が
     * グリーンになるのを見た。
     * **それでも足すのは、0 が「まだ種族を選んでいない」人物だから** ——
     * py を memset していたころの約束（各件は白紙の人物から始まる）を、置き場が
     * 移ったせいで黙って嘘にしないため。0 は Human の種族の土台でもあるが、
     * **どの階級も 1 以上足すので、振りおわった人物が 0 で居ることはない**
     * （player_stealth.h）。置く窓口 1 本なので 1 行（`_adjust` は足場の仕事では
     * ない）。 */
    player_stealth_set(0);
    /* どの階級かも #18-12-28C で src/player/player_class.c の static へ移ったので、
     * py を消しても届かない。**外してもレッドにならない** —— この行番号を読む
     * 既存の件（calc_spells の 17・gain_spells の 13・put_misc3 の 8）は
     * **どれも B で自分で階級を置くように書きかえた**ので、走りだしの 0 に
     * よりかかっていない。**確かめた** —— この 1 行を足さずに全 1517 件が
     * グリーンになるのを見た（21・25・26 つめと同じ側）。
     * **それでも足すのは、0 が「まだ階級を選んでいない」人物だから** ——
     * get_class() がメニューの前に置くのと同じ 0 で（たまたま class[] の
     * 0 行め Warrior でもある）、py を memset していたころの約束（各件は白紙の
     * 人物から始まる）を、置き場が移ったせいで黙って嘘にしないため。
     * 置く窓口 1 本なので 1 行（`_adjust` は無いし、これからも無い ——
     * 人物はローグに「なっていく」ものではない。player_class.h）。
     * **税は 0 本** —— この足場をリンクする 9 本は 9 本とも
     * すでに src/player/player_class.c を張っている（所見 34・48）。 */
    player_class_set(0);
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
