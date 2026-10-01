// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How far the character can see warm-blooded creatures

#ifndef PLAYER_INFRA_RANGE_H
#define PLAYER_INFRA_RANGE_H

// One question, one module -- the twelfth question to leave `py`, after the
// purse (player_gold.c), the stomach (player_food.c), the four numbers the
// character sheet shows (player_display_numbers.c), the mana (player_mana.c),
// the hit points (player_hp.c), how far the character has come
// (player_level.c), the status word (player_status_flags.c), what the equipment
// grants (player_abilities.c), the eighteen clocks (player_timed_effects.c),
// the rest (player_resting.c) and the speed (player_speed.c). ONE field
// answered it -- py.flags.see_infra, touched from nine places in six files
// (creature.c and dungeon.c two each, abilities.c one, create.c one, player_bonuses.c
// one, save.c two).
//
// THE QUESTION IS "HOW FAR AWAY CAN THE CHARACTER MAKE OUT A WARM-BLOODED
// CREATURE". THE UNIT IS SQUARES, one square being ten feet. Zero means no
// infra-vision at all, which is where a Human starts.
//
// WHY "RANGE" AND NOT "VISION": PLAYER_TIMED_INFRA_VISION
// (player_timed_effects.h) is the potion's clock, counted in TURNS, and it is
// one of the three things that move this number. Naming this one
// player_infra_vision() would sit one letter away from that clock and could be
// read as a yes/no ("does the character have infra-vision") when it is a
// distance.
//
// THREE THINGS PUT A NUMBER HERE, and they are the reason the windows are
// shaped the way they are:
//
//   - THE RACE, once, when the character is made (create.c). Human 0,
//     Half-Elf 2, Elf 3, Halfling 4, Gnome 4, Dwarf 5, Half-Orc 3. This is the
//     first question to leave `py` whose STARTING VALUE IS NOT ZERO, which is
//     why the deciding window below has two callers rather than only the save
//     file's reader.
//   - THE EQUIPMENT, by however much the item is worth (player_bonuses.c). py_bonuses()
//     is called with factor 1 when something is put on and -1 when it comes
//     off, so ONE LINE THERE COVERS BOTH DIRECTIONS.
//   - THE POTION, by exactly one square while it lasts (dungeon.c, at the turn
//     the clock starts and again at the turn it runs out).
//
// What this module deliberately does NOT do:
//
//   - THE CLOCK. The potion's turns belong to player_timed_effects.c and the
//     nudging stays in dungeon.c, which is the file that already knows the
//     clock has started or run out. The clock and this number are NOT two
//     halves of one fact -- this number has three sources and the clock is one
//     of them (player_resting.c's rest/PY_REST pair was the other way round;
//     see the ledger's observation 33).
//   - WHETHER A PARTICULAR MONSTER SHOWS UP. creature.c asks
//     "is it within range AND is it warm-blooded (CD_INFRA)", and the second
//     half is about the monster, not the character. Folding the first half in
//     here would close half a question and would invite tidying up the
//     `> 0` guard, which is redundant in practice (a monster is never placed on
//     the player's own square, so its distance is at least one) but which we do
//     not remove, because that would be a behaviour change.
//   - FEET. The character sheet multiplies by ten and writes "%d feet"
//     (abilities.c). That is how the number is shown, not what it is.
//   - ANY LIMIT. Nothing in the game checks this number against a range, so
//     neither does the window (the same rule as the mana, the hit points and
//     the speed).

// No includes: the callers bring config.h, constant.h and types.h in first, the
// same as player_speed.h and the ten before it.

// How far, IN SQUARES. Zero means warm blood alone never gives a monster away.
// Four of the nine places wanted the number itself: the visibility test
// (creature.c, which reads it twice in one condition), the character sheet's
// "%d feet" (abilities.c) and the save file's writer.
int player_infra_range(void);

// Further or nearer by this many squares. NEGATIVE TAKES IT BACK: the
// equipment's window (player_bonuses.c) passes a negative amount when an item comes
// off, and dungeon.c passes -1 when the potion runs out.
void player_infra_range_adjust(int squares);

// How far, as an outright answer rather than a change. TWO CALLERS: the race,
// when the character is made (create.c), and the save file's reader.
void player_infra_range_set(int squares);

#endif
