// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* stub_unreached.h -- 呼ばれないはずの代役を止める
 *
 * 定数を返すだけの代役は、呼ばれても黙って同じ値を返す。いまどのテストも
 * 届かない代役はこれで止め、届いたときに気づけるようにする。止まったら、
 * 記録・操作できる代役にするか本物をリンクする。
 */
#ifndef STUB_UNREACHED_H
#define STUB_UNREACHED_H

#include <stdio.h>
#include <stdlib.h>

static inline _Noreturn void stub_unreached(const char *name)
{
    fprintf(stderr, "stub reached: %s\n", name);
    abort();
}

#endif /* STUB_UNREACHED_H */
