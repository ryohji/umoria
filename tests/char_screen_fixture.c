// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* char_screen_fixture.c -- src/ui/char_screen.c を試すテストの足場
 * （put_misc3_test）
 *
 * fixture_reset() は、リンクされる窓口の状態を 0 に戻す。
 * 代役は、対象とその呼び先が呼ぶ名前のうち、ライブラリに無いもの
 * （fixture.h の窓口など）と、本物を引くと画面（ncurses）などが
 * 芋づるで付いてくるもの。
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "config.h"
#include "constant.h"
#include "types.h"

#include "fixture.h"
#include "player_attack_bonuses.h"
#include "player_base_to_hit.h"
#include "player_bio.h"
#include "player_body_weight.h"
#include "player_class.h"
#include "player_disarm.h"
#include "player_level.h"
#include "player_race.h"
#include "player_saving_throw.h"
#include "player_search_skill.h"
#include "player_speed.h"
#include "player_spells_to_learn.h"
#include "player_status_flags.h"
#include "player_stealth.h"
#include "player_timed_effects.h"

bool display_counts;
void prt(const char *s, int r, int c) { (void)s; (void)r; (void)c; }
void prt_map(void) {}

/* put_buffer は書かれた文字を画面を模した配列に記録する。画面に書くだけの
 * 関数を観測するため。 */
#define FIXTURE_SCREEN_ROWS 24
#define FIXTURE_SCREEN_COLS 80
static char fixture_screen[FIXTURE_SCREEN_ROWS][FIXTURE_SCREEN_COLS + 1];

void put_buffer(const char *s, int r, int c)
{
    if (s == NULL || r < 0 || r >= FIXTURE_SCREEN_ROWS || c < 0 ||
        c >= FIXTURE_SCREEN_COLS) {
        return;
    }
    for (int i = 0; s[i] != '\0' && c + i < FIXTURE_SCREEN_COLS; i++) {
        fixture_screen[r][c + i] = s[i];
    }
}

/* 指定位置から、put_buffer が書いた分だけの文字列を返す。 */
const char *fixture_screen_text(int row, int col)
{
    if (row < 0 || row >= FIXTURE_SCREEN_ROWS || col < 0 ||
        col >= FIXTURE_SCREEN_COLS) {
        return "";
    }
    return &fixture_screen[row][col];
}

void clear_screen(void) {}
void clear_from(int row) { (void)row; }
void bell(void) {}
char inkey(void) { return ' '; }

bool get_string(char *s, int r, int c, int l) {
    (void)s; (void)r; (void)c; (void)l;
    return false;
}

bool file_character(char *f) { (void)f; return false; }
void user_name(char *b) { (void)b; }

/* 各テストの前に呼ぶ。窓口の状態と代役の記録を 0 に戻す。 */
void fixture_reset(void)
{
    extern player_type py;

    memset(&py, 0, sizeof py);
    player_set_level(0);
    player_set_experience(0);
    player_set_max_experience(0);
    player_set_experience_fraction(0);
    player_set_experience_factor(0);
    player_set_status_word(0);
    for (int effect = 0; effect < PLAYER_TIMED_COUNT; effect++) {
        player_timed_clear((player_timed_effect)effect);
    }
    player_speed_set(0);
    player_spells_to_learn_set(0);
    player_base_to_hit_set(0, 0);
    player_disarm_set(0);
    player_saving_throw_set(0);
    player_race_set(0);
    player_body_weight_set(0);
    player_attack_bonuses_set(0, 0);
    player_search_chance_set(0);
    player_search_frequency_set(0);
    player_name_set("");
    player_set_male(false);
    player_age_set(0);
    player_height_set(0);
    player_social_class_set(0);
    player_history_clear();
    player_stealth_set(0);
    player_class_set(0);
    memset(fixture_screen, 0, sizeof fixture_screen);
}
