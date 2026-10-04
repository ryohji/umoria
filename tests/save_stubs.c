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
 * どれも呼ばれない。呼ばれたら本物が必要だという合図なので、値を返す代役は
 * stub_unreached() で止まる。
 *
 * 本体の宣言（externs.h）は include しない。save.c 側が include するので、
 * 型が合わなければリンクではなくコンパイルで落ちる。
 *
 * 画面・入力・乱数・速さの代役は shared_stubs.c のものを一緒にリンクする。
 * randint はこの 3 本のテストからは呼ばれない。
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "stub_unreached.h"

/* 画面表示・入力（io.c） */
void put_qio(void) {}

/* ファイル入出力の差しかえ層（io.c）。externs.h が fopen / open を
 * これらに置きかえている。ここは externs.h を引かないので、中で呼ぶのは本物。
 * get_char() が名前でファイルを開くテスト（savefile_layout_test）が通る。 */
FILE *tfopen(const char *file, const char *mode) { return fopen(file, mode); }
int topen(char *file, int flags, int mode) { return open(file, flags, mode); }

/* シグナル（signals.c） */
void nosignals(void) {}
void signals(void) {}

/* 終了処理（death.c） */
_Noreturn void exit_game(void) { (void)fflush(NULL); abort(); }
/* sv_write() が書く得点。テストが決める（savefile_layout_test）。 */
static int32_t stub_total_points;
int32_t total_points(void) { return stub_total_points; }
void save_stubs_set_total_points(int32_t points) { stub_total_points = points; }

/* プレイヤーの状態更新（inven_ops.c / player_bonuses.c / rest_command.c） */
void check_strength(void) {}
void disturb(int stop_search, int flush_input) { (void)stop_search; (void)flush_input; }

/* 店（store_stock.c） */
void store_maint(void) {}

/* 名前に付ける冠詞の判定（desc.c）。monsters.c が求めるだけで、desc.c を
 * リンクすると定数表とインベントリまで芋づるで付いてくるので代役を置く。 */
bool is_a_vowel(char ch) { stub_unreached(__func__); }

/* item_ident.c は「使ったあとに何を学ぶか」（learn_item_effect）と「品目ごとに
 * 何を覚えているか」（品目ごとの覚えの表）の 2 つを持つ。save.c が要るのは
 * 後者の生の窓口 2 つだけだが、リンクは翻訳単位ごとなので前者の行き先も
 * 埋めなければならない。どれもセーブファイルとは関わりがないので代役。
 *
 * 中身を要らないので struct の前方宣言だけ置く（types.h は引かない）。 */
typedef struct inven_type inven_type;

int known1_p(inven_type *i_ptr) { stub_unreached(__func__); }
void identify(int *item) { (void)item; }
void sample(inven_type *i_ptr) { (void)i_ptr; }
void prt_experience(void) {}

/* floor_items.c は #18-14-7B⑤ からリンクしている（save.c が床の表を
 * 窓口越しに読み書きするようになった）。あちらは**空の行を作るために**
 * invcopy() を呼ぶ —— 空の行は定義表 object_list の 1 行の写しなので。
 * セーブファイルの読み書きはその道（階の頭・行を返す）を通らないので代役。
 * 本物は src/item/desc.c で、連れてくると desc.c ごと来る。 */
void invcopy(inven_type *to, int from_index) { (void)to; (void)from_index; }
