// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* shared_stubs.h -- 足場が共有する代役（shared_stubs.c）の記録を消す窓口 */
#ifndef SHARED_STUBS_H
#define SHARED_STUBS_H

/* 代役の記録（画面・メッセージ・キー・乱数・速さ）を消す。
 * randint() が返す値（fixture_set_randint）は戻さない。 */
void shared_stubs_reset(void);

#endif /* SHARED_STUBS_H */
