// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* armor_selection_test 専用の代役。
 * randint だけを代役にして、テストが台本で答えを制御できるようにする。 */

#include <stdint.h>

/* テストが台本を設定する配列とインデックス。 */
int mock_randint_answer[32];
int mock_randint_index;

int randint(int n) {
    (void)n;
    return mock_randint_answer[mock_randint_index++];
}
