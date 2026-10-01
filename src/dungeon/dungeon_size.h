// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// How tall and how wide this level is

#ifndef DUNGEON_SIZE_H
#define DUNGEON_SIZE_H

// ONE PAIR OF NUMBERS, AND IT ONLY EVER TAKES TWO VALUES. generate_cave()
// picks 22 x 66 for the town and 66 x 198 for a dungeon level, so the pair is
// really a cache of "am I in the town?" -- the same thing the depth says, and
// that question has its own window (player_is_in_town(), dungeon_level.h).
// Nothing else ever writes it, and nothing scales it: a level is one of those two
// shapes for its whole life.
//
// TWO NAMES, ONE ACT. Both writers set both halves, one line after the other
// (generate.c twice, save.c's restore once), and the readers almost always use
// the pair: of 48 reads, 40 are a height and a width together. Keeping the two
// apart meant a writer could set one and forget the other, and no reader could
// tell. So there is one setter that takes both, and no way to change half of
// the size.

// The size of the level being played. Before the first level is made both are
// 0: the game always calls generate_cave() (or restores a save file) before
// anything reads them.
int dungeon_height(void);
int dungeon_width(void);

// A new level is this big. The two callers are generate_cave(), which chooses
// between the town's one panel and the dungeon's nine, and save.c's restore,
// which reads the pair back out of the file. The pair is stored as two shorts
// in the save file and both values fit easily, so a restore is exact.
//
// panel.c has to be told the same numbers (panel_set_dungeon_size), because it
// derives how many panels the level is worth. This window does not tell it:
// the two calls sit next to each other at the call sites, the way they always
// have, and the restore path feeds panel.c from the file instead (the file
// stores panel.c's two counts separately).
void set_dungeon_size(int height, int width);

// WHAT THE 48 READS DO, since the numbers themselves say so little (the other
// 14 of the 62 references are the 4 writes, the 2 in the restore, the one
// definition line, 4 mentions inside comments, and -- for good measure -- two
// fields of the UI snapshot struct that happen to carry the same two names):
//
//  - bounds tests -- in_bounds() and the reachability test in mmove(), both
//    in geometry.c, ask whether a square is inside the level at all. These are the
//    only reads that take the pair apart in an interesting way: the inside of
//    the level is 0 < y < height - 1, because the outermost ring of squares is
//    always wall.
//
//  - walking the whole level -- five hand-written double loops (generate.c
//    twice, object_place.c twice, wizard.c once). They cannot be folded into one
//    iterator: every body uses i and j themselves, and two of them keep a
//    pointer they step along by hand. Only the two bounds come through a
//    window.
//
//  - picking a random square -- in TWO DIFFERENT FORMS, which is worth
//    knowing: `randint(height - 2)` gives 1..height-2 (monster_place.c twice,
//    generate.c once) and `randint(height) - 1` gives 0..height-1 (teleport()
//    in player_move.c, twice). The second can land on the boundary wall and the first cannot.
//    Both are left exactly as they were: folding them into one window would
//    quietly change where things appear.
//
//  - how many panels -- generate.c divides by SCREEN_HEIGHT/SCREEN_WIDTH to
//    lay out rooms, and panel.c divides the same way for its own counts.
//
//  - walling in the level -- place_boundary() in generate.c is the only reader
//    that takes the halves one at a time: six reads, walking the right edge
//    down and the bottom edge across, with an asserted pointer walk beside
//    each. Every other read wants both numbers.
//
//  - one comparison is wrong, and it stays wrong for now: the 'L'ocate command
//    in dungeon.c tests the new row against the WIDTH, not the height. With a
//    66-row, 198-column dungeon it lets a row up to 197 through. This is an
//    upstream bug, not something the move to a window should decide, so it was
//    carried over unchanged and written down here instead.

#endif // DUNGEON_SIZE_H
