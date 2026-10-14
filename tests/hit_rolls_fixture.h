// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* hit_rolls_fixture.h -- disturb() の代役（hit_rolls_fixture.c）の記録を読む窓口 */
#ifndef HIT_ROLLS_FIXTURE_H
#define HIT_ROLLS_FIXTURE_H

/* disturb() が呼ばれた回数と、それを 0 に戻す。 */
int fixture_disturb_count(void);
void fixture_disturb_reset(void);

#endif /* HIT_ROLLS_FIXTURE_H */
