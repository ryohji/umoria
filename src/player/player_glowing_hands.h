// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Whether the character's hands are glowing, ready to confuse what they touch

#ifndef PLAYER_GLOWING_HANDS_H
#define PLAYER_GLOWING_HANDS_H

// THE NAME. externs.h declares a function named confuse_monster(dir, row, col)
// in spells.c, the spell a wand or a scroll aims at one monster. That function
// is something the character DOES to a monster now, and this is something the
// character HAS until the next blow lands. The game's own messages name the
// hands ("Your hands begin to glow.", "Your hands stop glowing."), so the
// module is named after the hands too.
//
// THE QUESTION. Are the hands glowing? There is no clock. Scroll 11 lights
// them, and they stay lit for as many turns as it takes -- resting, walking,
// going down stairs, all of it -- until one blow actually connects. Then they
// go out, whether the monster was confused or not.

// Is a charge waiting, and how big is it? Zero means the hands are not glowing.
//
// The game only ever stores one charge, so every caller in the game treats this
// as a yes-or-no. It is an int rather than a bool because a saved file holds a
// byte (see player_glowing_hands_restore() below).
int player_glowing_hands(void);

// Scroll 11: the hands light up. Reading a second scroll while they are still
// glowing does nothing at all -- not even identify the scroll -- so the caller
// asks player_glowing_hands() first.
void player_glowing_hands_begin(void);

// A blow connected, so the charge is gone. Called from both sides of a fight:
// the character's own blow (player_melee.c) and a monster's blow that actually hits
// the character (monster_melee.c -- a repelled attack does not spend the
// charge).
void player_glowing_hands_spend(void);

// Put back the byte a saved file holds. Separate from _begin() so the byte
// travels through unchanged: _begin() always stores one, and this stores
// whatever was written, including values the game itself never produces.
void player_glowing_hands_restore(int charge);

// WHAT THIS MODULE DOES NOT ANSWER -- four things, all of them still in the
// callers:
//
//   1. THE TWO MESSAGES. "Your hands begin to glow." and "Your hands stop
//      glowing." belong to msg_print(), which lives behind externs.h, and this
//      module does not include externs.h. The callers say it.
//   2. WHETHER THE MONSTER IS CONFUSED. The resistance roll (its level against
//      randint(MAX_MONS_LEVEL), and CD_NO_SLEEP), the turns added to
//      m_ptr->confused, and the note taken in recall are all about the
//      monster, not about the character's hands. Folding them in here would
//      close only half of one question and open half of another.
//   3. WHICH BLOW IT WAS. monster_melee.c asks adesc != 99 beside this charge.
//      That is about the kind of attack, not about the hands.
//   4. WHETHER THE SCROLL IS IDENTIFIED. scrolls.c sets ident only when the
//      hands were dark, so reading a second scroll leaves the scroll unknown.
//      That is the scroll's rule; this module only reports the charge.

#endif // PLAYER_GLOWING_HANDS_H
