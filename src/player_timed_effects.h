// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How much longer each of the eighteen temporary states lasts

#ifndef PLAYER_TIMED_EFFECTS_H
#define PLAYER_TIMED_EFFECTS_H

// One question, one module -- the ninth question to leave `py`, after the purse
// (player_gold.c), the stomach (player_food.c), the four numbers the character
// sheet shows (player_display_numbers.c), the mana (player_mana.c), the hit
// points (player_hp.c), how far the character has come (player_level.c), the
// status word (player_status_flags.c) and what the equipment grants
// (player_abilities.c). EIGHTEEN fields answered it -- py.flags.blind through
// py.flags.tim_infra, touched from two hundred and eighty-one places in eighteen
// files (dungeon.c seventy-five of them, save.c thirty-eight, creature.c
// thirty-two). Since #18-12-9C they are a static array in
// player_timed_effects.c and the windows below are the only way to reach them.
//
// THE QUESTION IS "HOW MANY TURNS ARE LEFT". A cause adds turns (a potion, a
// bite, a trap, a prayer), one block in dungeon.c takes one turn off every
// eighteen counters once per turn, and reaching zero ends the state. Nothing
// else in the game counts anything down like this: it is the only clock the
// character carries.
//
// THIS MODULE IS ONE HALF OF A PAIR. player_status_flags.c keeps the mark that
// says an effect "has begun and has been announced" (its family 1), and twelve
// of those fourteen marks pair with a counter here. The two halves are separate
// because the marks are bits in one word THAT IS THE SAVE FILE'S FORMAT, while
// the counters are eighteen shorts. spells.c:829 is the place that proves they
// are different questions: drawing a breath weapon asks the MARK, not the
// clock, because a monster may have just added turns of blindness that have not
// taken effect yet.
//
// WHAT #18-12-7 LEFT HERE. That unit wrote down that dungeon.c repeats one
// skeleton twelve times and that folding it needs both halves, so it belongs to
// whichever module owns the counters -- this one. The skeleton is
//
//     if (f_ptr->hero > 0) {
//         if (player_note_effect_started(PLAYER_EFFECT_HERO)) { ...once... }
//         f_ptr->hero--;
//         if (f_ptr->hero == 0) { player_note_effect_ended(...); ...once... }
//     }
//
// and it folds into player_timed_in_force() + player_timed_beginning() +
// player_timed_count_down(). The bookkeeping (which mark goes with which
// counter, when the mark is cleared) comes in here; THE BODIES STAY WITH THE
// CALLER, because the eighteen bodies differ -- messages, hit points, to-hit,
// armour, redraws, lighting the monsters again.
//
// TWO WINDOWS AND NOT ONE. A single window returning "started / running /
// ended" cannot express what happens when a state is entered with one turn left:
// it begins AND ends in the same turn, and both bodies run. So the beginning and
// the counting down are asked separately, in that order.
//
// THE COUNTER CAN GO BELOW ZERO, and one caller depends on it. dungeon.c cancels
// new-found fear while the character is a hero by putting zero in the counter --
// and then the same turn's count-down takes it to MINUS ONE, which misses the
// "== 0" ending, so no message is printed and the mark is never set. The minus
// one then stays (the block only runs while the counter is above zero) and makes
// the next fright one turn shorter, because creature.c adds to whatever is
// there. player_timed_count_down() SUBTRACTS UNCONDITIONALLY to keep that
// behaviour exactly.
//
// What this module deliberately does NOT do:
//
//   - the marks (player_status_flags.c owns the word; this module only asks it
//     to set and clear the twelve that pair with a counter),
//   - anything a beginning or an ending leads to: the messages, the screen, the
//     hit points a hero gains, the hundred points of armour, change_speed(),
//     creatures() being called again, the damage poison does in a turn (that one
//     reads the constitution, which is another question),
//   - the three fields that sit among the eighteen in `struct flags` and in the
//     save file but are not clocks: `rest` (WHAT the character is doing),
//     `speed` and `see_infra` (current values that these clocks nudge by one).

// No includes: the callers bring config.h, constant.h and types.h in first, the
// same as player_status_flags.h and the seven before it.

// The eighteen, IN THE ORDER THE SAVE FILE KEEPS THEM. Nothing depends on the
// order -- save.c names each one, because the file's run of twenty-four shorts
// also holds the stomach's two, `rest`, `protection`, `speed` and `see_infra`,
// so this module cannot be handed a position in the file the way
// player_abilities.c is. The order is the file's anyway, so that the two can be
// read side by side.
typedef enum {
    PLAYER_TIMED_BLINDNESS,
    PLAYER_TIMED_PARALYSIS,
    PLAYER_TIMED_CONFUSION,
    PLAYER_TIMED_HASTE,
    PLAYER_TIMED_SLOWNESS,
    PLAYER_TIMED_FEAR,
    PLAYER_TIMED_POISON,
    PLAYER_TIMED_HALLUCINATION,
    PLAYER_TIMED_PROTECTION_FROM_EVIL,
    PLAYER_TIMED_INVULNERABILITY,
    PLAYER_TIMED_HEROISM,
    PLAYER_TIMED_SUPER_HEROISM,
    PLAYER_TIMED_BLESSING,
    PLAYER_TIMED_HEAT_RESISTANCE,
    PLAYER_TIMED_COLD_RESISTANCE,
    PLAYER_TIMED_SEEING_INVISIBLE,
    PLAYER_TIMED_WORD_OF_RECALL,
    PLAYER_TIMED_INFRA_VISION,
    PLAYER_TIMED_COUNT
} player_timed_effect;

// Whether the state is in force at all: more than zero turns left. A hundred and
// thirty-six of the two hundred and eighty-one places were this comparison.
bool player_timed_in_force(player_timed_effect effect);

// The number itself, for the four places that need more than "is it in force":
// the chance of using a staff or a wand (confusion), the "Paralysed" line (which
// shows only above one), the prayer that tops invulnerability up to three, and
// the save file's writer.
int player_timed_turns(player_timed_effect effect);

// A cause makes the state last longer. THE COMMONEST WAY IN -- fifty of the
// places -- because most causes add to whatever is left rather than replacing
// it: two bites of a confusing monster last longer than one.
void player_timed_add(player_timed_effect effect, int turns);

// A cause says exactly how long, throwing away what was left. Twenty-eight
// places, most of them a trap or a spell that fixes the length.
void player_timed_set(player_timed_effect effect, int turns);

// No turns left. Not the same as the state ending: the mark is untouched, which
// is what the two callers want -- dungeon.c cancelling fear a hero should not
// feel, and moria4.c stepping onto a trap without being pushed sideways by
// confusion (it puts the turns back afterwards).
void player_timed_clear(player_timed_effect effect);

// A cure shortens the state without being able to lengthen it: the four "cure"
// windows in spells.c leave one turn so that the ending still comes through the
// count-down with its message, eat.c does the same to fear, and save.c stops a
// resurrected character dying of the same poison at once.
void player_timed_shorten_to(player_timed_effect effect, int turns);

// dungeon.c's turn, first half: the state is in force and HAS NOT BEEN
// ANNOUNCED, so whatever the caller does once at the beginning is due now. False
// for the six that have no mark (paralysis, hallucination, protection from evil,
// the two resistances and the word of recall) -- there is nothing to announce and
// none of those six callers has a beginning.
bool player_timed_beginning(player_timed_effect effect);

// dungeon.c's turn, second half: ONE TURN PASSES. Subtracts one whatever is
// there (see the note about minus one above) and answers whether the state ran
// out in this turn, which is when the caller's ending is due. Clears the mark
// when it does.
bool player_timed_count_down(player_timed_effect effect);

#endif
