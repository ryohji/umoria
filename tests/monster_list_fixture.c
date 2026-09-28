// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* 空の行が持つ値（テスト用の足場）
 *
 * **もとは 3 つ置いていた。** #18-14-4A/B のあいだ、表（m_list）と印（mfptr）
 * の置き場は monsters.c にあって、src/monster_list.c が `extern` 3 行で
 * 見にいっていた。テストは monsters.c をリンクしない —— 279 体の定義表と
 * 窓口 10 本ぶんの世界が丸ごと付いてくる —— ので、同じ名前をここに置いて
 * 代わりにした。
 *
 * **#18-14-4C で表と印は src/monster_list.c の static に入り、その 2 行は
 * 消えた。残ったのは blank_monster 1 つだけ** —— これはこの問いの主題では
 * なく定数表の側で、monsters.c に残っている。module にとっては「空の行とは
 * 何か」なので、本来の置き場はあちらではなくこちらだが、名前を動かすのは
 * 定数表の区分の仕事（module の註にも書いた）。**それが済めば
 * src/monster_list.c は extern を 1 つも持たなくなり、このファイルも消える。**
 *
 * **#18-14-1〜3 に足場が要らなかった理由。** あの 3 問は A の段で置き場を
 * module の static に入れた（所見 52）。あの形が使えるのは「B を 1 コミットで
 * 通しきれる」ときだけで、この問いは 12 ファイル 101 参照なので B を
 * ファイルごとに 5 つに割った。割るあいだ器が 2 つあると**半分が古い器を、
 * 半分が新しい器を読む**ので、置き場は最後まで 1 つにしておいた。
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* monsters.c の字面をそのまま写す（欄はすべてゼロ）。 */
monster_type blank_monster = {0, 0, 0, {0}, 0, 0, 0, false, 0, false};
