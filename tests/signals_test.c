// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* セーブのあいだシグナルを止める nosignals() と、戻す signals() のテスト
 * （src/platform/signals.c）。
 *
 * 端末の backend は起動時に signals_set_suspend_handler() で ^Z（SIGTSTP）の
 * 手を付ける。nosignals() は SIGTSTP を無視にし、signals() が付けた手に戻す。
 * 戻さないと、セーブのあと ^Z が効かなかった（バグ候補 B15）。
 * 手は自分で raise() して、呼ばれた回数を数える。 */

#include "platform_signals.h"

#include <signal.h>

#include "minunit.h"

static volatile sig_atomic_t suspended;

static void count_suspend(int sig) {
    (void)sig;
    suspended++;
}

TEST(the_suspend_handler_is_called_after_it_is_set) {
    suspended = 0;
    signals_set_suspend_handler(count_suspend);
    (void)raise(SIGTSTP);
    ASSERT_EQ_INT(suspended, 1);
}

TEST(nosignals_ignores_suspend) {
    suspended = 0;
    signals_set_suspend_handler(count_suspend);
    nosignals();
    (void)raise(SIGTSTP);
    signals();
    ASSERT_EQ_INT(suspended, 0);
}

TEST(signals_puts_the_suspend_handler_back) {
    suspended = 0;
    signals_set_suspend_handler(count_suspend);
    nosignals();
    signals();
    (void)raise(SIGTSTP);
    ASSERT_EQ_INT(suspended, 1);
}

int main(void) {
    RUN_TEST(the_suspend_handler_is_called_after_it_is_set);
    RUN_TEST(nosignals_ignores_suspend);
    RUN_TEST(signals_puts_the_suspend_handler_back);
    return TEST_SUMMARY();
}
