// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Which level the game is on now

#ifndef DUNGEON_LEVEL_H
#define DUNGEON_LEVEL_H

#include <stdbool.h>

// ONE NUMBER, AND ALMOST EVERYTHING THAT SCALES WITH DEPTH COMES OUT OF IT.
// Level 0 is the town; 1 and deeper are the dungeon. Thirteen of the thirty
// reads use it as a difficulty: which monsters may be rolled, how good the
// objects are, how much gold is in a pile, whether a room is lit, whether the
// level gets a Balrog. The number itself is never checked against a ceiling
// here -- the limits belong to the commands that name a new level (the wizard's
// prompt says 0-99; the deep-descent scroll stops at 1), the same division
// leave_for_level() already makes (level_exit.h).
//
// THE SAME QUESTION WAS WRITTEN THREE WAYS. Six reads only want to know whether
// the game is in the town, and they wrote it as `!= 0` (whether the stores turn
// over), as `> 0` (word of recall yanking upwards, lighting a room, the spell of
// destruction) and as `== 0` (which size the next level is). They are all one
// question, so there is one window for it. Nothing ever puts a negative depth
// in, so `!= 0` and `> 0` really are the same test: going up from the town is
// impossible because the town has no up staircase, not because anybody compares.
//
// This was `int16_t dun_level = 0;` in variable.c. #18-14-6.

// Which level the game is on. 0 is the town.
int dungeon_level(void);

// Is the game in the town? The six callers listed above.
bool player_is_in_town(void);

// The game is on this level now. Three callers write a level here directly --
// becoming a King and resurrecting a dead character from a save file both mean
// "back to the town", and restoring a save file puts back the level that was
// being played -- and leave_for_level() (level_exit.c) writes the rest, because
// leaving a level is a depth and a finished flag in one breath (level_exit.h).
void set_dungeon_level(int level);

// WHAT THE 30 READS DO, in four groups (the other 10 of the 40 references are
// the 3 direct writes, the restore, the definition line, the extern that
// level_exit.c kept and its note, and -- because the high score table has a
// field of its own by the same name -- three mentions that are not this number
// at all):
//
//  - am I in the town -- the six above.
//
//  - how deep, therefore how hard -- 13 reads, and the only group that does
//    arithmetic on the number: `<= randint(25)` four times (a room is lit or
//    dark), `> randint(DUN_UNUSUAL)` (an unusual room), `/ 3` (how much is
//    scattered on the floor), `>= WIN_MON_APPEAR` (the Balrog), and six reads
//    that hand the depth to a table: which monster, which object, how much
//    gold. Two of them add a constant first (summoning rolls a monster from
//    `+ MON_SUMMON_ADJ`, deeper than the level really is).
//
//  - which level comes next -- four reads, all of them `depth + 1` or
//    `depth - 1` or, for the deep-descent scroll, `depth - 3 + 2 * randint(2)`.
//    Each one is handed straight to leave_for_level().
//
//  - saying it, scoring it, saving it -- seven reads: the status line, the
//    death certificate, the high score entry, the score itself, the record of
//    how deep the character has ever been (player_max_depth.h), the snapshot
//    the 2025 UI layer takes, and the save file.
//
// `depth * 50` IS WRITTEN TWICE AND MEANS TWO DIFFERENT THINGS. The status line
// turns it into feet (50 feet per level, and level 0 prints "Town level"); the
// score adds it as points. Same arithmetic, different units, so neither one is
// a copy of the other and there is no window for it -- the second time this
// group has found look-alike expressions that are not the same expression.

#endif // DUNGEON_LEVEL_H
