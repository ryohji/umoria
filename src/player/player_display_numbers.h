// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The four combat numbers the character sheet shows

#ifndef PLAYER_DISPLAY_NUMBERS_H
#define PLAYER_DISPLAY_NUMBERS_H

// One question, one module -- the third question to leave `py`, after the purse
// (player_gold.c) and the stomach (player_food.c). Four numbers answer it
// together: the +To Hit, +To Damage, +To AC and Total AC printed on the status
// panel, the character sheet and the character dump. They came from
// py.misc.dis_th, dis_td, dis_tac and dis_ac, which forty-three places used to
// touch.
//
// These are *not* the numbers the game fights with. Those are the real plusses,
// and none of them is in `py` any more: the aim and the force went to
// player_attack_bonuses.c in #18-12-24, the two halves of the armour class to
// player_armour_class.c in #18-12-18. This question is only about what the
// character is told. Four things make the
// difference real, and all four are reasons the copies cannot become a
// derivation of the real numbers:
//
//   - **They are filtered by what the character knows.** calc_bonuses() adds
//     every worn item's plusses to the real numbers, but adds them here only
//     when the item is identified (known2_p). An unidentified item still shows
//     its base AC -- unless it is cursed, in which case even that is withheld.
//   - **One penalty exists only here.** A weapon too heavy to wield properly
//     costs the *shown* to-hit (use_stat[A_STR] * 15 - weight) and nothing
//     else; the real aim never carries it (player_to_hit_bonus), because the
//     attack code takes it
//     off separately (attack_blows(), hit_rolls.c). So the shown to-hit is not a
//     filtered real number -- it is assembled in its own right.
//   - **The AC is shown twice, summed differently.** The real numbers keep the
//     armour (pac) and the bonus (ptoac) apart; the sheet prints "+ To AC"
//     *and* "Total AC", so the visible bonus has to be folded into the visible
//     total (player_display_fold_to_ac).
//   - **They are remembered, not recomputed.** All four go into the save file
//     and come back out of it, and nothing calls calc_bonuses() on the way in.
//     The sheet a restored character sees is the one that was saved. Like the
//     lit circle in player_light.c, this stays a place to put a number rather
//     than becoming a derivation.

// --- what the sheet says --------------------------------------------------

// Read by the status panel (status_line.c) and the character sheet
// (char_screen.c), the character dump (files.c) and the save file's writer
// (save.c). Nothing else asks.
int16_t player_display_to_hit(void);
int16_t player_display_to_dam(void);
int16_t player_display_to_ac(void);
int16_t player_display_ac(void);

// --- putting the sheet together again -------------------------------------

// Start over from the real plusses: the three bonuses shown are the real ones
// before any equipment is counted, and nothing of the armour is visible yet.
// This is the rule that opens calc_bonuses() (moria1.c) and closes character
// creation (create.c) -- the same four lines in both places, which is why they
// are in here.
//
// The caller passes the real numbers in because the real numbers are not this
// module's business; what is this module's business is that the sheet begins
// as a copy of them.
void player_display_start_from_real(int16_t to_hit, int16_t to_dam, int16_t to_ac);

// A worn item's contribution, one number at a time, as calc_bonuses() walks the
// equipment. The amount may be negative: a known cursed item lowers what the
// sheet says, and so does a weapon too heavy to wield (that one goes through
// player_display_add_to_hit).
void player_display_add_to_hit(int amount);
void player_display_add_to_dam(int amount);
void player_display_add_to_ac(int amount);

// Changes the Total AC. Worn armour goes on here as the equipment is walked,
// and so do the temporary blessings: invulnerability is worth 100 and a
// blessing 2, added when the spell takes hold and taken off when it lapses
// (dungeon.c does that without recalculating anything else).
void player_display_add_ac(int amount);

// The "+ To AC" the sheet shows is part of the "Total AC" it shows. Fold it in
// once the equipment has been counted. Calling this twice would count the bonus
// twice -- the one caller calls it once, at the end of the walk.
void player_display_fold_to_ac(void);

// --- putting a number in outright -----------------------------------------

// Replaces what was there. Only the save file's reader needs these: the four
// numbers are stored one by one, in their own order, and each one goes back
// where it came from.
void player_display_set_to_hit(int16_t value);
void player_display_set_to_dam(int16_t value);
void player_display_set_to_ac(int16_t value);
void player_display_set_ac(int16_t value);

#endif
