/* inventory_fixture.c -- src/inventory.c をリンクするための足場
 *
 * src/inventory.c を書いたあと、リンカに未解決シンボルを列挙させたら 4 個
 * しか残らなかった:
 *   gcc -std=c17 -Isrc -Itests -c -o /tmp/inv.o src/inventory.c
 *   gcc -o /tmp/t /tmp/inv.o 2>&1 | grep 'undefined reference'
 *   -> inventory, inven_ctr, inven_weight, equip_ctr の 4 個（と main）
 *
 * 4 個はどれも本体では treasure.c にある。あのファイルは 550 行の巨大な
 * アイテム定数表と同居していて、そこだけをリンクすることはできない
 * （object_list[] ごと引くことになる）。だからテスト側で定義する。
 *
 * ここには代役（スタブ）が 1 つもない。inventory.c が誰も呼ばないからで、
 * それが窓口の薄さそのものである。
 *
 * fixture_reset() を置いていない。本体（treasure.c）のグローバルと同じく
 * 0 初期化された状態を「走りだしの状態」としてテストで見るので、毎テストの
 * 前に消してしまうとその 1 件が意味を失う。かわりに inventory_test.c の
 * main() の並び順で、走りだしを見るテストを先に走らせている
 * （tests/stores_test.c と同じやりかた）。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 本体では treasure.c:558,561-563。名前と型はそこと同じにしてある
 * （型がずれると窓口の読み書きが別の場所に当たる）。 */
inven_type inventory[INVEN_ARRAY_SIZE];
int16_t inven_ctr = 0;
int16_t inven_weight = 0;
int16_t equip_ctr = 0;
