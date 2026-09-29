// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 「空の行」の作りかた（テスト用の足場）
 *
 * **もとは 3 つ置いていた。** #18-14-7A/B のあいだ、表（t_list）と印（tcptr）
 * の置き場は treasure.c にあって、src/floor_items.c が `extern` 2 行で
 * 見にいっていた。テストは treasure.c をリンクしない —— 420 品の定義表が
 * 丸ごと付いてくる —— ので、同じ名前をここに置いて代わりにした。
 *
 * **#18-14-7C で表と印は src/floor_items.c の static に入り、その 2 行は
 * 消えた。残ったのは invcopy() 1 つだけ** —— #18-14-4 の
 * tests/monster_list_fixture.c とまったく同じ形で、あちらに残ったのも
 * 「空の行とは何か」の 1 つ（blank_monster）だった。
 *
 * **ただし残りかたが違う。** モンスターの表では「空の行」は blank_monster
 * という**名前のついた定数**で、定数表の区分がそれを動かせばあの足場は
 * 消える。床のものにはその定数が無く、**定義表の 1 行
 * （object_list[OBJ_NOTHING]、「nothing」）を写したもの**が空の行になる。
 * 写す invcopy() は src/desc.c:561 にある**関数**で、object_list を読む。
 * テストにあれを連れてくると desc.c と treasure.c ごと来るので、ここで
 * 代役を立てる。**だからこのファイルは、定数表の区分が済んでも消えない**
 * —— 消えるのは invcopy() が定義表を読まなくなったときだけ。
 *
 * **代役は「どの品目を写したか」だけを残す。** 本物は 17 の欄を写すが、
 * この module が決めているのは「どの行に、どの品目を写すか」だけで、
 * 写した中身が正しいかは desc.c の受けもち（tests/objdes_test.c が本物の
 * 表で invcopy() を動かしている）。だからテストは index の欄だけを見る。
 *
 * ついでに言うと、object_list[OBJ_NOTHING] は欄がぜんぶ 0 ではない
 * （tchar が ' '、subval が 64）。代役をそこまで似せると「空の行とは
 * treasure.c のあの 1 行だ」とテストが言いだしてしまうので、似せていない。
 *
 * **#18-14-1〜3・5・6 に足場が要らなかった理由。** あの 5 問は A の段で
 * 置き場を module の static に入れた（所見 52）。あの形が使えるのは
 * 「B を 1 コミットで通しきれる」ときだけで、この問いは 17 ファイル
 * 127 参照なので B をファイルごとに 5 つに割った。割るあいだ器が 2 つ
 * あると**半分が古い器を、半分が新しい器を読む**ので、置き場は最後まで
 * 1 つにしておいた。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* src/desc.c:561 の代役。写した品目の番号だけを残す。 */
void invcopy(inven_type *to, int from_index) {
    const inven_type blank = {0};
    *to = blank;
    to->index = (uint16_t)from_index;
}
