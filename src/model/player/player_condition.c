// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Player condition: status flags, timed effects, food, resting, running, light,
// glowing hands, pending teleport, and position.

#include "config.h"
#include "constant.h"
#include "types.h"

#include "player_condition.h"

// === Status Flags ===
// From player_status_flags.c

// This module reaches nothing outside itself. It knows nothing about what a mark
// means -- no messages, no counters, no drawing. It keeps the set and hides the
// mask arithmetic.

// THE WORD ITSELF, and the only copy of it. The thirty bits live here and nothing
// outside this file can name them. Zero is where a character starts -- no mark set,
// no request waiting, not searching, not resting. create.c sets the real state when
// it makes a character, and save.c puts the saved word back through
// player_set_status_word().
static uint32_t status;

// The fourteen marks, in the order of the enum. THE MAPPING IS THE ONLY PLACE
// THE BIT VALUES ARE NAMED, which is what lets the callers speak of effects
// instead of bits. The bit order and the enum order are different on purpose:
// nothing outside this table is allowed to depend on either.
static const uint32_t effect_marks[PLAYER_EFFECT_COUNT] = {
    PY_HERO,     PY_SHERO,    PY_BLIND,    PY_CONFUSED, PY_FEAR,
    PY_POISONED, PY_FAST,     PY_SLOW,     PY_INVULN,   PY_BLESSED,
    PY_DET_INV,  PY_TIM_INFRA, PY_HUNGRY,  PY_WEAK,
};

static bool is_set(uint32_t mask) {
    return (status & mask) != 0;
}

static void set_bits(uint32_t mask) {
    status |= mask;
}

static void clear_bits(uint32_t mask) {
    status &= ~mask;
}

// Sets the mask and says whether any of it was new. The callers of the marks and
// of the requests both want this shape: "was this already noted?".
static bool set_and_say_if_new(uint32_t mask) {
    if (is_set(mask)) {
        return false;
    }

    set_bits(mask);

    return true;
}

// Reads the mask and leaves none of it behind.
static bool take(uint32_t mask) {
    if (!is_set(mask)) {
        return false;
    }

    clear_bits(mask);

    return true;
}

static void set_or_clear(uint32_t mask, bool wanted) {
    if (wanted) {
        set_bits(mask);
    } else {
        clear_bits(mask);
    }
}

bool player_effect_in_force(player_effect effect) {
    return is_set(effect_marks[effect]);
}

bool player_note_effect_started(player_effect effect) {
    return set_and_say_if_new(effect_marks[effect]);
}

void player_note_effect_ended(player_effect effect) {
    clear_bits(effect_marks[effect]);
}

void player_note_hunger_satisfied(void) {
    clear_bits(PY_HUNGRY | PY_WEAK);
}

void player_request_strength_check(void) {
    set_bits(PY_STR_WGT);
}

bool player_strength_check_requested(void) {
    return is_set(PY_STR_WGT);
}

void player_clear_strength_check_request(void) {
    clear_bits(PY_STR_WGT);
}

void player_request_study_redraw(void) {
    set_bits(PY_STUDY);
}

bool player_study_redraw_requested(void) {
    return is_set(PY_STUDY);
}

void player_clear_study_redraw_request(void) {
    clear_bits(PY_STUDY);
}

void player_request_speed_redraw(void) {
    set_bits(PY_SPEED);
}

bool player_take_speed_redraw_request(void) {
    return take(PY_SPEED);
}

void player_request_armor_redraw(void) {
    set_bits(PY_ARMOR);
}

bool player_take_armor_redraw_request(void) {
    return take(PY_ARMOR);
}

void player_request_hp_redraw(void) {
    set_bits(PY_HP);
}

bool player_take_hp_redraw_request(void) {
    return take(PY_HP);
}

void player_request_mana_redraw(void) {
    set_bits(PY_MANA);
}

bool player_take_mana_redraw_request(void) {
    return take(PY_MANA);
}

// The six stat requests share one run of adjacent bits, starting at PY_STR and
// masked together as PY_STATS. constant.h warns that the six must stay adjacent;
// THIS IS NOW THE ONLY FILE THAT DEPENDS ON IT (misc3.c shifted the bit,
// app/dungeon.c shifted it again to read it back, and both used the mask).
static uint32_t stat_mark(int stat) {
    return (uint32_t)PY_STR << stat;
}

void player_request_stat_redraw(int stat) {
    set_bits(stat_mark(stat));
}

bool player_any_stat_redraw_requested(void) {
    return is_set(PY_STATS);
}

bool player_stat_redraw_requested(int stat) {
    return is_set(stat_mark(stat));
}

void player_clear_stat_redraw_requests(void) {
    clear_bits(PY_STATS);
}

bool player_is_searching(void) {
    return is_set(PY_SEARCH);
}

void player_start_searching(void) {
    set_bits(PY_SEARCH);
}

void player_stop_searching(void) {
    clear_bits(PY_SEARCH);
}

bool player_is_resting(void) {
    return is_set(PY_REST);
}

void player_start_resting(void) {
    set_bits(PY_REST);
}

void player_stop_resting(void) {
    clear_bits(PY_REST);
}

bool player_status_line_shows_paralysis(void) {
    return is_set(PY_PARALYSED);
}

void player_set_status_line_shows_paralysis(bool shown) {
    set_or_clear(PY_PARALYSED, shown);
}

bool player_status_line_shows_repeat(void) {
    return is_set(PY_REPEAT);
}

void player_set_status_line_shows_repeat(bool shown) {
    set_or_clear(PY_REPEAT, shown);
}

uint32_t player_status_word(void) {
    return status;
}

void player_set_status_word(uint32_t word) {
    status = word;
}

// === Timed Effects ===
// From player_timed_effects.c

// This module reaches nothing outside itself. It does not know what any of the
// eighteen counters leads to -- no messages, no rolls, no drawing.

// THE EIGHTEEN ANSWERS THEMSELVES. This array is the only place they live, and the
// windows below are the only way to reach them. The order is the save file's
// (player_condition.h), but save.c names each one, so nothing here depends on it.
//
// No reset window: a new character starts the program over, the same as the
// status word and the seventeen abilities. Loading a saved game writes all
// eighteen through player_timed_set() (save.c).
static int16_t timed_turns_left[PLAYER_TIMED_COUNT];

// WHICH MARK PAIRS WITH WHICH CLOCK. Twelve of the fourteen marks in
// player_status_flags.c pair with one of the eighteen counters; the other two
// (hungry, weak) pair with the stomach, and six of the counters have no mark at
// all. Swapping two rows here would announce the wrong state -- that is what the
// test file nails one row at a time.
//
// PLAYER_EFFECT_COUNT is not a mark, so it stands for "this one has none", and
// nothing is ever handed to player_status_flags.c without has_mark() first.
#define NO_MARK PLAYER_EFFECT_COUNT

static const player_effect mark_of[PLAYER_TIMED_COUNT] = {
    PLAYER_EFFECT_BLIND,         // blindness
    NO_MARK,                     // paralysis -- the status line shows it instead
    PLAYER_EFFECT_CONFUSED,      // confusion
    PLAYER_EFFECT_HASTED,        // haste
    PLAYER_EFFECT_SLOWED,        // slowness
    PLAYER_EFFECT_AFRAID,        // fear
    PLAYER_EFFECT_POISONED,      // poison
    NO_MARK,                     // hallucination -- the whole map is redrawn
    NO_MARK,                     // protection from evil -- a message only
    PLAYER_EFFECT_INVULNERABLE,  // invulnerability
    PLAYER_EFFECT_HERO,          // heroism
    PLAYER_EFFECT_SUPER_HERO,    // super heroism
    PLAYER_EFFECT_BLESSED,       // blessing
    NO_MARK,                     // heat resistance -- a message only
    NO_MARK,                     // cold resistance -- a message only
    PLAYER_EFFECT_SEE_INVISIBLE, // seeing the invisible
    NO_MARK,                     // word of recall -- fires at one, not at zero
    PLAYER_EFFECT_INFRA_VISION,  // infra-vision
};

// No range check on the effect, the same as player_status_flags.c: THE ENUM IS
// THE ONLY WAY TO NAME ONE, so a value outside the eighteen can only come from a
// cast, and there is no window that turns a number into an effect (the save
// file's run is not addressed by position here -- see the header).
static bool timed_has_mark(player_timed_effect effect) {
    return mark_of[effect] != NO_MARK;
}

bool player_timed_in_force(player_timed_effect effect) {
    return timed_turns_left[effect] > 0;
}

int player_timed_turns(player_timed_effect effect) {
    return timed_turns_left[effect];
}

void player_timed_add(player_timed_effect effect, int turns) {
    timed_turns_left[effect] = (int16_t)(timed_turns_left[effect] + turns);
}

void player_timed_set(player_timed_effect effect, int turns) {
    timed_turns_left[effect] = (int16_t)turns;
}

void player_timed_clear(player_timed_effect effect) {
    player_timed_set(effect, 0);
}

void player_timed_shorten_to(player_timed_effect effect, int turns) {
    if (timed_turns_left[effect] > turns) {
        timed_turns_left[effect] = (int16_t)turns;
    }
}

bool player_timed_beginning(player_timed_effect effect) {
    if (!timed_has_mark(effect)) {
        return false;
    }

    return player_note_effect_started(mark_of[effect]);
}

bool player_timed_count_down(player_timed_effect effect) {
    // Unconditionally: minus one is a state the fear counter really reaches
    // (player_condition.h).
    timed_turns_left[effect] = (int16_t)(timed_turns_left[effect] - 1);

    if (timed_turns_left[effect] != 0) {
        return false;
    }

    if (timed_has_mark(effect)) {
        player_note_effect_ended(mark_of[effect]);
    }

    return true;
}

// === Food ===
// From player_food.c

// A counter and its rate need nothing from the game around them. In particular,
// this module does not know the hunger thresholds, does not touch the status flags,
// does not deal damage and does not print anything.

// Owned here and static: the only way in is through the windows below. The
// initial values are 0, so a program starts with an empty stomach that digests
// nothing; app/main.c puts the 7500 and the 2 in once the character exists.
//
// The names follow the windows: "food" was a counter, not a larder, and
// "food_digested" was a rate.
static int16_t food = 0;
static int16_t digestion = 0;

int16_t player_food(void) {
    return food;
}

void player_gain_food(int amount) {
    // The first bite forgives a starvation debt: what add_food() did before
    // adding anything. Without it a starving character would have to eat back
    // the whole deficit before the counter meant anything again.
    if (food < 0) {
        food = 0;
    }
    food = (int16_t)(food + amount);
}

void player_burn_food(int amount) {
    food = (int16_t)(food - amount);
}

void player_set_food(int16_t amount) {
    food = amount;
}

void player_digest(void) {
    food = (int16_t)(food - digestion);
}

int16_t player_digestion(void) {
    return digestion;
}

void player_set_digestion(int16_t amount) {
    digestion = amount;
}

void player_adjust_digestion(int delta) {
    digestion = (int16_t)(digestion + delta);
}

// === Resting ===
// From player_resting.c

// No player_condition.h either: PY_REST is a different question and its
// callers keep it (player_condition.h explains why this one does not fold the pair
// the way player_timed_effects.c does).

// THE ANSWER ITSELF. This one short is the only place it lives, and the windows
// below are the only way to reach it.
//
// No reset window: zero means "not resting", which is where a new character
// starts, the same as the status word and the eighteen clocks. Loading a saved
// game writes it through player_rest_set() (save.c).
static int16_t rest_turns_left;

bool player_resting(void) {
    return rest_turns_left != 0;
}

int player_rest_turns(void) {
    return rest_turns_left;
}

bool player_rest_is_until_healed(void) {
    return rest_turns_left < 0;
}

void player_rest_set(int rest_turns) {
    rest_turns_left = (int16_t)rest_turns;
}

void player_rest_stop(void) {
    player_rest_set(0);
}

bool player_rest_count_down(void) {
    // Towards zero from whichever side it is on. The "not resting" case answers
    // false without touching the count: app/dungeon.c's old shape was
    // `if (rest > 0) ... else if (rest < 0) ...`, so zero moved nowhere.
    if (rest_turns_left > 0) {
        rest_turns_left = (int16_t)(rest_turns_left - 1);
    } else if (rest_turns_left < 0) {
        rest_turns_left = (int16_t)(rest_turns_left + 1);
    } else {
        return false;
    }

    return rest_turns_left == 0;
}

// === Running ===
// From running.c

// Counting steps needs nothing from the rest of the game. In particular, this
// module does not know how to move anybody, nor which way the run is going
// (find_direction is a static of run_path.c, next to the corridor-following
// rules that are the only user of it).

// The count is owned here and is static: the only way in is through the six
// windows below. The initial value is zero, which is how a game begins with nobody
// running. The name follows the windows: it is a number of steps, not a flag.
static int run_steps = 0;

// How many steps a run is allowed. Stated once, here, and nowhere else: see
// player_condition.h for why it is not at the call site.
#define RUN_STEPS_ALLOWED 100

void begin_run(void) {
    // The first step is the one find_init() is about to take, so the count starts
    // at one rather than zero -- which is also what makes the value a "yes" to
    // player_is_running().
    run_steps = 1;
}

bool player_is_running(void) {
    return run_steps != 0;
}

void stop_running(void) {
    run_steps = 0;
}

void forget_run(void) {
    run_steps = 0;
}

bool keep_running(void) {
    // The step is counted whether or not the run may go on, exactly as the old
    // "find_flag++ > 100" did: the test reads the count from before the step.
    return run_steps++ <= RUN_STEPS_ALLOWED;
}

int running_steps(void) {
    return run_steps;
}

// === Light ===
// From player_light.c

// The flag is owned here and is static: the only way in is through the two
// windows below. It has no initial value: dungeon() writes it from the light
// slot before the first turn.
static bool player_light;

bool player_has_light(void) {
    return player_light;
}

void set_player_has_light(bool lit) {
    player_light = lit;
}

// Whether the glow is on the map, owned here and static as well. The first
// move_light() of a game finds no glow drawn, because nothing has been drawn yet.
static bool player_light_drawn = false;

bool player_light_is_drawn(void) {
    return player_light_drawn;
}

void set_player_light_drawn(bool drawn) {
    player_light_drawn = drawn;
}

// === Glowing Hands ===
// From player_glowing_hands.c

// Note that confuse_monster() is declared elsewhere (player_condition.h
// explains why the names collide).

// THE ANSWER ITSELF. This one byte is the only place it lives, and the windows
// below are the only way to reach it.
//
// No reset window: zero means "the hands are not glowing", which is where every
// character starts and where every blow that connects leaves them. Only scroll
// 11 and a saved file ever put something else here.
static uint8_t the_charge;

int player_glowing_hands(void) {
    return the_charge;
}

void player_glowing_hands_begin(void) {
    // Replacing, not adding. The game never stacks two charges: scrolls.c only
    // gets this far when the hands were dark.
    the_charge = 1;
}

void player_glowing_hands_spend(void) {
    // The whole charge goes at once, however big it was. Both fights ask
    // player_glowing_hands() first, so this is never reached with dark hands.
    the_charge = 0;
}

void player_glowing_hands_restore(int charge_from_file) {
    the_charge = (uint8_t)charge_from_file;
}

// === Pending Teleport ===
// From pending_teleport.c

// One bit needs nothing from the rest of the game. In particular, this
// module does not know how to teleport anybody -- it only holds the note.

// The flag is owned here and is static: the only way in is through the four
// windows below. The initial value is false, so a new game begins with no
// teleport waiting.
static bool teleport_pending = false;

void schedule_teleport(void) {
    teleport_pending = true;
}

bool teleport_is_pending(void) {
    return teleport_pending;
}

void teleport_done(void) {
    teleport_pending = false;
}

void forget_pending_teleport(void) {
    teleport_pending = false;
}

// === Position ===
// From player_pos.c

// This file needs nothing declared at all -- the record it hands out is its own.
// Every caller in src/ and tests/ goes through the four entry points below; the
// record is static, and the compiler is what guarantees nobody reaches around the
// window.
//
// No initialiser, which is what player.c had as well. The game writes the
// position (generate.c, or the save file's reader) before anything reads it.
static int16_t char_row;
static int16_t char_col;

int16_t player_row(void) {
    return char_row;
}

int16_t player_col(void) {
    return char_col;
}

coordinate player_position(void) {
    return (coordinate){char_row, char_col};
}

void player_place(int row, int col) {
    char_row = (int16_t)row;
    char_col = (int16_t)col;
}

void player_pos_forget(void) {
    player_place(-1, -1);
}
