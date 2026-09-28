// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 床にあるものの置き場と「空の行」の作りかた（テスト用の足場）
 *
 * **3 つ置いている。うち 2 つは #18-14-7C で消える。**
 *
 * 表（t_list）と印（tcptr）—— #18-14-7A/B のあいだ、置き場は treasure.c に
 * あって、src/floor_items.c が `extern` 2 行で見にいっている（17 ファイル
 * 127 参照なので B をファイルごとに割る。割るあいだ器を 2 つにしないため。
 * 所見 52）。テストは treasure.c をリンクしない —— 420 品の定義表が丸ごと
 * 付いてくる —— ので、同じ名前をここに置いて代わりにした。**C で置き場が
 * src/floor_items.c の static に入れば、この 2 行は消える。**
 *
 * invcopy() —— こちらは**残る**。モンスターの表では「空の行」は
 * blank_monster という名前のついた定数だったが、床のものにはそれが無く、
 * **定義表の 1 行（object_list[OBJ_NOTHING]、「nothing」）を写したもの**が
 * 空の行になる。写す関数 invcopy() は src/desc.c:561 にあり、object_list を
 * 読む。テストにあれを連れてくると desc.c と treasure.c ごと来るので、
 * ここで代役を立てる。
 *
 * **代役は「どの品目を写したか」だけを残す。** 本物は 17 の欄を写すが、
 * この module が決めているのは「どの行に、どの品目を写すか」だけで、
 * 写した中身が正しいかは desc.c の受けもち（tests/objdes_test.c が本物の
 * 表で invcopy() を動かしている）。だからテストは index の欄だけを見る。
 *
 * ついでに言うと、object_list[OBJ_NOTHING] は欄がぜんぶ 0 ではない
 * （tchar が ' '、subval が 64）。代役をそこまで似せると「空の行とは
 * treasure.c のあの 1 行だ」とテストが言いだしてしまうので、似せていない。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* treasure.c:553 と :561 の字面をそのまま写す。 */
inven_type t_list[MAX_TALLOC];
int16_t tcptr;

/* src/desc.c:561 の代役。写した品目の番号だけを残す。 */
void invcopy(inven_type *to, int from_index) {
    const inven_type blank = {0};
    *to = blank;
    to->index = (uint16_t)from_index;
}
