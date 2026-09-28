// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* この階にいるモンスターの置き場（テスト用の足場）
 *
 * **この 3 つは #18-14-4C で src/monster_list.c の static に入り、そのとき
 * このファイルは消える。** それまでのあいだ、置き場は monsters.c にある
 * （src/monster_list.c の `extern` 3 行）。テストは monsters.c をリンクしない
 * ので —— 279 体の定義表と窓口 10 本ぶんの世界が丸ごと付いてくる ——
 * 同じ名前を 3 つここに置いて代わりにする。
 *
 * **#18-14-1〜3 とはここが違う。** あの 3 問は A の段で置き場を module の
 * static に入れたので足場が要らなかった（所見 52）。あの形が使えるのは
 * 「B を 1 コミットで通しきれる」ときだけで、この問いは 12 ファイル 101 参照
 * なので B をファイルごとに割る。割るあいだ器が 2 つあると**半分が古い器を、
 * 半分が新しい器を読む**ので、置き場は最後まで 1 つにしておく。
 *
 * **置き場を閉じる人（#18-14-4C）がこのファイルを消す。**
 */
#include "config.h"
#include "constant.h"
#include "types.h"

/* 初期値は monsters.c:764・:776・:778 に合わせる。m_list と mfptr は
 * ゼロ初期化で、**mfptr の 0 は「まだ 1 行も使っていない」ではない** ——
 * 走りだしの正しい値は MIN_MONIX（= 2）で、それを書くのは
 * monster_list_reset() だけ。ゲームでは generate.c の mlink() が最初の階を
 * 作るときに必ず通る。 */
monster_type m_list[MAX_MALLOC];
int16_t mfptr;

/* 空の行が持つ値。monsters.c:776 の字面をそのまま写す。 */
monster_type blank_monster = {0, 0, 0, {0}, 0, 0, 0, false, 0, false};
