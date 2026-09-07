/* save_stubs.c -- src/save.c をリンクするための代役
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

/* 終了処理（death.c / misc1.c） */
_Noreturn void exit_game(void) { (void)fflush(NULL); abort(); }
int32_t total_points(void) { return 0; }

/* プレイヤーの状態更新（misc3.c / moria1.c） */
void change_speed(int num) { (void)num; }
void check_strength(void) {}
void disturb(int stop_search, int flush_input) { (void)stop_search; (void)flush_input; }

/* 店（store1.c / store2.c） */
void store_maint(void) {}
bool general_store(int action) { (void)action; return false; }
bool armory(int action) { (void)action; return false; }
bool weaponsmith(int action) { (void)action; return false; }
bool temple(int action) { (void)action; return false; }
bool alchemist(int action) { (void)action; return false; }
bool magic_shop(int action) { (void)action; return false; }

/* 乱数（misc1.c） */
int randint(int maxval) { return maxval; }

/* 名前に付ける冠詞の判定（desc.c）。monsters.c が求めるだけで、desc.c を
 * リンクすると定数表とインベントリまで芋づるで付いてくるので代役を置く。 */
bool is_a_vowel(char ch) { (void)ch; return false; }
