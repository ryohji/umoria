// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

/* summon_undead（world/monster/monster_place.c）がどのモンスターを呼ぶか
 *
 * summon_undead は表の途中の乱数の位置から上へ数えて、最初のアンデッドを呼ぶ。
 * 上限（monsters_up_to_level(MAX_MONS_LEVEL)、その 1 つ先まで読む）を越えたら
 * 乱数を引きなおす。この選びかたの並びを legacy_pick() に写しとり、同じ種から
 * 引いたとき本物が置いたモンスターと突きあわせる。
 *
 * 選んだあと summon() が置き場の乱数を引くが、選ぶ乱数はその前に引ききるので、
 * 写しの選択と本物の選択は同じ乱数の列を読む。
 */
#include "config.h"
#include "constant.h"
#include "types.h"
#include "dungeon_state.h"
#include "monster_state.h"
#include "monsters.h"
#include "rnd.h"
#include "world_monster.h"

#include "minunit.h"

/* 変更前の選びかたの写し（字面のまま。添字を返す）。 */
static int legacy_pick(void)
{
    int l = monsters_up_to_level(MAX_MONS_LEVEL);
    int found = -1;

    do {
        int m = randint(l) - 1;
        int ctr = 0;
        do {
            if (monster_creature_get(monster_creature_at((uint16_t)m))->cdefense & CD_UNDEAD) {
                found = m;
                l = 0;
                ctr = 20;
            } else {
                m += 1;
                ctr = m > l ? 20 : ctr + 1;
            }
        } while (ctr != 20);
    } while (l != 0);

    return found;
}

/* 一面の床（開いた場所）。モンスターは置かれていない。 */
static void open_floor(void)
{
    set_dungeon_size(SCREEN_HEIGHT, SCREEN_WIDTH);
    dungeon_map_reset();
    for (int y = 0; y < dungeon_height(); y++) {
        for (int x = 0; x < dungeon_width(); x++) {
            square_at(y, x)->fval = 1;
            square_at(y, x)->cptr = 0;
        }
    }
    monster_list_reset();
}

#define SEEDS 400
static int placed;
static int mismatch_seed;

static void run_seeds(void)
{
    placed = 0;
    mismatch_seed = -1;
    for (uint32_t seed = 1; seed <= SEEDS; seed++) {
        open_floor();
        set_rnd_seed(seed);
        const int expected = legacy_pick();

        open_floor();
        set_rnd_seed(seed);
        int y = SCREEN_HEIGHT / 2;
        int x = SCREEN_WIDTH / 2;
        if (!summon_undead(&y, &x)) {
            continue;
        }
        placed++;
        const int got = monster_list_at(square_at(y, x)->cptr)->creature.place;
        if (got != expected && mismatch_seed < 0) {
            mismatch_seed = (int)seed;
        }
    }
}

TEST(most_seeds_place_a_monster)
{
    /* 置けなかった種は比べられないので、ほとんどの種で置けていることを見る。 */
    ASSERT_TRUE(placed > SEEDS * 9 / 10);
}

TEST(summons_the_same_undead_as_the_legacy_choice)
{
    ASSERT_EQ_INT(mismatch_seed, -1);
}

int main(void)
{
    monster_levels_init();
    run_seeds();
    RUN_TEST(most_seeds_place_a_monster);
    RUN_TEST(summons_the_same_undead_as_the_legacy_choice);
    return TEST_SUMMARY();
}
