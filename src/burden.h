// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// What the character carries is more than they can manage: the wielded weapon
// that is too heavy for their strength, and the pack that is heavy enough to
// slow them down.

#ifndef BURDEN_H
#define BURDEN_H

// This is storage and only storage, the same as progress.h, score_death.h,
// hp_table.h and player_light.h. Both answers are computed from the character's
// strength and the weight they carry -- check_strength() (misc3.c) does that
// once, when PY_STR_WGT says the weight or the strength has changed -- but they
// are *remembered* rather than derived, for the same reason as the light:
// only the transitions carry the messages and the side effects.
//
//   the weapon: going from light enough to too heavy prints "You have trouble
//   wielding such a heavy weapon." and recomputes the bonuses; coming back
//   prints "You are strong enough to wield your weapon."
//
//   the pack: the speed change is applied as a *difference*
//   (change_speed(new - remembered)), so without the remembered number the
//   character's speed cannot be corrected -- it would be reapplied from
//   scratch every time.
//
// The windows do not range-check or clamp, and they decide nothing on the
// caller's behalf, because the old assignments did neither.

// --- the weapon that is too heavy ----------------------------------------

// True while the wielded weapon weighs more than the character's strength can
// handle (use_stat[A_STR] * 15 < weight). While it is true the to-hit shown on
// the character sheet is reduced (moria1.c) and digging is harder (moria4.c).
//
// Wielding something else clears it before check_strength() looks again
// (moria1.c twice), and so does restoring a save file, because the flag is not
// in the save file.
bool weapon_is_too_heavy(void);
void set_weapon_too_heavy(bool too_heavy);

// --- the pack that slows the character down ------------------------------

// How many steps of speed the pack is costing right now; 0 when the pack is
// light enough to be free. It is a count, not a flag: the weight over the limit
// is divided by the limit, so a very heavy pack costs several steps.
//
// Reading it is how the callers answer two different questions: "has the
// penalty changed?" (compared against a freshly computed number, in
// check_strength and in inven_check_weight, which is what makes picking an
// object up refusable) and "how much speed do I have to give back?" (save.c
// hands change_speed the negated number before writing the file, so that the
// speed in the save file is the unencumbered one).
int pack_speed_penalty(void);
void set_pack_speed_penalty(int steps);

#endif // BURDEN_H
