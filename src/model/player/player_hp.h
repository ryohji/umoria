// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The player's hit points: what is left, what it can reach, and the part of a
// point that has not finished coming back yet

#ifndef PLAYER_HP_H
#define PLAYER_HP_H

// THE QUESTION. Three numbers answer it together: what is left, what it can reach
// and the part of a point still on its way back.
//
// The fraction is here for the reason it is in player_mana.h -- regeneration
// hands back a few hundred 65536ths of the maximum each turn, so without
// somewhere to keep the remainder nothing would ever come back. That part of
// this module is the mana's part with different names.
//
// WHAT IS NEW HERE IS THAT A NEGATIVE NUMBER IS NOT A QUANTITY. It is the mark
// of a dead character, and it is the only mark there is: nothing clamps hit
// points at zero, so a killing blow leaves them negative and they STAY negative
// all the way through the save file. Four places read the mark:
//
//   - take_hit() subtracts the damage and asks the question straight away
//     (player_damage.c),
//   - main.c asks it after loading, because a character can be put away dead by
//     a signal or a hangup and has to be marked dead again on the way back in,
//   - save.c asks it to decide whether to overwrite the "killed by" line with
//     "(alive and well)",
//   - save.c asks it again for the wizard resurrection, which is the one place
//     that brings the number back up to zero.
//
// So a window that tidied the number up to zero would silently break all four.
// The mark is the reason player_take_hp_damage() returns a bool and the reason
// player_hp_marks_death() exists at all: the callers should be asking a question
// with a name, not comparing a number with zero.
//
// Where this module stops is also set by the mark. Whether the wound killed the
// character is a question about the number, so it is answered here. What death
// then DOES -- the death flag, the string naming what killed them, leaving the
// level -- is the caller's, and none of it is mentioned in this file.
//
// Five rules live inside:
//
//   - regeneration (player_regenerate_hp) is the fixed-point sum, the guard for
//     a maximum so large the short wraps round, and the stop at the top where
//     the fraction is cleared ("must set frac to zero even if equal"),
//   - taking damage (player_take_hp_damage) subtracts and reports whether the
//     result is the mark of death. It does NOT clamp,
//   - healing (player_heal_hp) refuses to do anything to a character already at
//     the top, and caps at the top when it overshoots,
//   - a new maximum (player_change_max_hp) carries what is left across in
//     proportion, fraction included. UNLIKE THE MANA, A MAXIMUM OF ZERO MEANS
//     "DO NOTHING AT ALL", not "start full": mhp is zero only while the
//     character is still being made, and calc_hitpoints() deliberately does not
//     even store the new maximum then (create.c fills both in a moment later).
//     This is the first place the mana's windows cannot simply be copied,
//   - a temporary maximum going up and coming back down (the two
//     player_*_temporary_max_hp windows). Going up adds to both numbers and
//     leaves the fraction; coming down takes the maximum away first and then
//     caps what is left. Heroism (ten points) and super heroism (twenty) wrote
//     the same four lines each (dungeon.c).
//
// What the rules deliberately do NOT do:
//
//   - how fast regeneration runs is the caller's number, and so is the decision
//     not to regenerate at all while poisoned (dungeon.c),
//   - what the maximum SHOULD be comes from the level, the hit-point table and
//     the constitution (calc_hitpoints in level_ops.c), and raising PY_HP afterwards
//     belongs to the status flags, which are another question,
//   - "is it full?" is not a window. The three places that ask are not asking
//     the same thing -- twice it is `chp < mhp` (regenerate, heal) and once it
//     is `chp == mhp` (stop resting) -- and folding them would change what
//     happens if chp ever exceeded mhp,
//   - printing. prt_chp() and prt_mhp() stay with the callers, which watch for a
//     change themselves.

// What is left. Read by the status line, the character sheet, the character
// dump, the save file, the high-score record and the rest-until-recovered test.
// A NEGATIVE VALUE MEANS THE CHARACTER IS DEAD -- see player_hp_marks_death().
int16_t player_hp(void);

// What it can reach. Zero only while a character is being made.
int16_t player_max_hp(void);

// The part of a point still on its way back. Only the save file asks.
uint16_t player_hp_fraction(void);

// Whether what is left is the mark of a dead character: less than zero.
bool player_hp_marks_death(void);

// Takes a wound and says whether it was fatal. Nothing is clamped: a fatal
// wound leaves the number negative on purpose, because that is what the mark of
// death is made of. The caller decides what dying involves.
bool player_take_hp_damage(int damage);

// Heals, capping at the top, and says whether anything happened -- a character
// already at the top is not healed at all, and the caller uses that to stay
// quiet. The fraction survives a partial heal and is cleared when the top is
// reached.
bool player_heal_hp(int amount);

// One turn of coming back, at the caller's rate (a factor in 65536ths).
// Everything the fraction needs is in here.
void player_regenerate_hp(int percent);

// Sets a new maximum, carrying what is left across in proportion, and says
// whether the maximum moved. A maximum of zero (a character still being made)
// is left alone entirely -- this is where the mana's window does not fit.
bool player_change_max_hp(int16_t new_max);

// A temporary maximum arriving: both numbers go up together, so what is left
// stays as far from the top as it was. Heroism and super heroism.
void player_gain_temporary_max_hp(int16_t bonus);

// The same temporary maximum wearing off: the maximum comes down and what is
// left is capped to it. Says whether the cap bit, because the callers only
// redraw when it did.
bool player_lose_temporary_max_hp(int16_t bonus);

// A brand new maximum, filled to the top. Character creation and the wizard
// command wrote the same three lines.
void player_reset_hp(int16_t max);

// Brings a dead character back to nothing at all and says whether there was
// anything to bring back. The wizard resurrection is the only caller, and the
// only place the mark of death is ever rubbed out.
bool player_resurrect_hp(void);

// The three parts set outright, one at a time, because the save file's reader
// meets them one at a time and its order cannot move.
void player_set_hp(int16_t value);
void player_set_max_hp(int16_t value);
void player_set_hp_fraction(uint16_t value);

#endif
