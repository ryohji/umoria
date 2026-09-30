// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* save_stubs.c -- src/save/save.c をリンクするための代役
 *
 * save.c は 1300 行あり、セーブ／ロードの本体（get_char / save_char）が
 * 画面表示・入力・シグナル・店の再入荷・モンスター配列にまで手を伸ばす。
 * 検証したいのは真偽値 1 つの読み書きだけなので、本物をリンクするのは
 * データ側（variable.c / tables.c / treasure.c / player.c）に留め、残りは
 * ここで埋める。一覧はリンカに列挙させたもので、手で数えあげてはいない。
 *
 * どれも呼ばれない。呼ばれたら代役ではなく本物が必要だという合図なので、
 * 黙って何もせず返すのではなく気づけるようにしたい -- が、戻り値の型が
 * まちまちで統一した通知手段を置けないため、ここでは最小の実装にしてある。
 *
 * 本体の宣言（externs.h）は include しない。save.c 側が include するので、
 * 型が合わなければリンクではなくコンパイルで落ちる。
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* 画面表示・入力（io.c） */
void clear_screen(void) {}
void put_buffer(const char *str, int row, int col) { (void)str; (void)row; (void)col; }
void put_qio(void) {}
void prt(const char *str, int row, int col) { (void)str; (void)row; (void)col; }
void msg_print(const char *str) { (void)str; }
bool get_check(const char *prompt) { (void)prompt; return false; }
bool get_string(char *in_str, int row, int col, int slen) { (void)in_str; (void)row; (void)col; (void)slen; return false; }

/* ファイル入出力の差しかえ層（io.c）。externs.h が fopen / open を
 * これらに置きかえている。テストは tmpfile() を使うので通らない。 */
FILE *tfopen(const char *file, const char *mode) { (void)file; (void)mode; return NULL; }
int topen(char *file, int flags, int mode) { (void)file; (void)flags; (void)mode; return -1; }

/* シグナル（signals.c） */
void nosignals(void) {}
void signals(void) {}

/* 終了処理（death.c） */
_Noreturn void exit_game(void) { (void)fflush(NULL); abort(); }
int32_t total_points(void) { return 0; }

/* プレイヤーの状態更新（inven_ops.c / player_bonuses.c / rest_command.c） */
void change_speed(int num) { (void)num; }
void check_strength(void) {}
void disturb(int stop_search, int flush_input) { (void)stop_search; (void)flush_input; }

/* 店（store_stock.c） */
void store_maint(void) {}

/* 乱数（core/rnd.c） */
int randint(int maxval) { return maxval; }

/* 名前に付ける冠詞の判定（desc.c）。monsters.c が求めるだけで、desc.c を
 * リンクすると定数表とインベントリまで芋づるで付いてくるので代役を置く。 */
bool is_a_vowel(char ch) { (void)ch; return false; }

/* item_ident.c は「使ったあとに何を学ぶか」（learn_item_effect）と「品目ごとに
 * 何を覚えているか」（品目ごとの覚えの表）の 2 つを持つ。save.c が要るのは
 * 後者の生の窓口 2 つだけだが、リンクは翻訳単位ごとなので前者の行き先も
 * 埋めなければならない。どれもセーブファイルとは関わりがないので代役。
 *
 * 中身を要らないので struct の前方宣言だけ置く（types.h は引かない）。 */
typedef struct inven_type inven_type;

int known1_p(inven_type *i_ptr) { (void)i_ptr; return 0; }
void identify(int *item) { (void)item; }
void sample(inven_type *i_ptr) { (void)i_ptr; }
void prt_experience(void) {}

/* floor_items.c は #18-14-7B⑤ からリンクしている（save.c が床の表を
 * 窓口越しに読み書きするようになった）。あちらは**空の行を作るために**
 * invcopy() を呼ぶ —— 空の行は定義表 object_list の 1 行の写しなので。
 * セーブファイルの読み書きはその道（階の頭・行を返す）を通らないので代役。
 * 本物は src/item/desc.c で、連れてくると desc.c ごと来る。 */
void invcopy(inven_type *to, int from_index) { (void)to; (void)from_index; }
