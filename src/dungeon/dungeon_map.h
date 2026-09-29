// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Every square of the level the player is walking around

#ifndef DUNGEON_MAP_H
#define DUNGEON_MAP_H

// THE LAST OF THE ELEVEN, and the one the other ten pointed at. This was
// `cave_type cave[MAX_HEIGHT][MAX_WIDTH];` in variable.c: 258 references in
// fifteen files, more than any other global left in externs.h. It is last
// because two of the seven fields in a square are indices into tables that had
// to be closed first -- cave[y][x].cptr says which monster stands there
// (#18-14-4) and cave[y][x].tptr says which thing lies there (#18-14-7) -- and
// until those two had windows there was no way to say what handing out a square
// ought to mean. #18-14-8.
//
// IT HOLDS THE TOWN TOO, so the name was narrower than the thing. The table is
// always MAX_HEIGHT x MAX_WIDTH (66 x 198); the town is generated into the top
// left 22 x 66 of the same table, and generate_cave() blanks the whole of it
// before either kind of level is built. What varies is how much of the table is
// in play, and that is dungeon_size.c's question, not this one. (The same shape
// of misnaming as t_list in #18-14-7: a name that says less than the container
// holds.)
//
// WHAT ONE SQUARE HOLDS -- seven fields, and the counts of how often the tree
// touches each (measured over src/*.c with comments stripped):
//
//     fval  214   what the square is made of: floor, corridor, rubble-blocked
//                 corridor, one of four kinds of wall, or one of two temporary
//                 values generate.c uses while it is still deciding
//     tptr  161   the row of floor_items.c's table lying here, 0 for nothing
//     cptr  118   the row of monster_list.c's table standing here, 0 for
//                 nobody and 1 for the player
//     pl     44   permanent light: this square is lit by the room it is in
//     lr     35   this square belongs to a room that should be lit
//     fm     25   field mark: the player has seen the trap/door/stairs here,
//                 and the thing on the square is hidden while this is false
//     tl     18   temporary light: the player's lamp reaches here right now
//
// THE WINDOW HANDS OUT A SQUARE, NOT A FIELD. That is not a convenience: of
// the 258 references, 141 were already `&cave[y][x]` being put into a local
// `cave_type *c_ptr`, so "give me that square, I will read my own fields off
// it" is the shape the game already speaks in. The seven questions above are
// seven further questions, and this module answers none of them; the notes at
// the bottom of this file say what was measured about them.
//
// NOTHING IS CHECKED, exactly as `&cave[y][x]` checked nothing. The tree does
// have a guard, in_bounds() in misc1.c, but it asks a different and stricter
// question -- is (y, x) inside the level's boundary ring, 0 < y < height - 1 --
// and generate.c has to write that ring, so the ring cannot be refused here.
// Callers that need the guard already call it.
//
// NO CALLER WALKS THE TABLE WITH A POINTER ANY MORE. Nineteen loops used to --
// eighteen in generate.c and one in save.c -- in three shapes:
//
//   * ALONG A ROW -- seventeen of the eighteen: take `&cave[i][x_left]`, then
//     carry `c_ptr++` across the columns the caller means to touch. fill_cave(),
//     every room builder and the town's two lighting sweeps drew their floors
//     and walls this way, and six of the seventeen carried a second pointer
//     along the opposite edge of the room at the same time. An upstream comment
//     said why the row and not the column ("the x dim of rooms tends to be much
//     larger than the y dim, so don't bother rewriting the y loop") -- the long
//     dimension got the pointer. With a window there is nothing to save, so
//     that comment went too.
//
//   * DOWN A COLUMN, once, in place_boundary(): a `cave_type (*)[MAX_WIDTH]`
//     stepped row by row behind two casts, with two DEBUG asserts whose only
//     job was to prove the arithmetic still landed on `&cave[i][0]`.
//
//   * OVER THE WHOLE TABLE, once, in save.c's loader, to unpack a run-length
//     encoding against an end pointer, with a comment explaining why that end
//     could not be written as `&cave[MAX_HEIGHT][0]`.
//
// All nineteen became loops with two subscripts, so the promise that the table
// is one contiguous row-major block is now this file's business and nobody
// else's. The casts, the asserts and both comments went with them.

// A new level: every square blank. Called by generate_cave() in generate.c for
// the town and for every dungeon level, before anything is built. It was a
// static wrapper there, blank_cave(), whose whole body was one memset -- the
// third such wrapper this group has absorbed, after mlink() (#18-14-4) and
// tlink() (#18-14-7).
//
// THE WHOLE TABLE IS BLANKED, not the part the level in play uses. The town
// occupies 22 x 66 and the next dungeon level 66 x 198, so a reset that stopped
// at dungeon_height()/dungeon_width() would leave the town's walk-out rows
// holding the last level's walls.
//
// A BLANK SQUARE IS NOT EMPTY FLOOR. fval 0 is NULL_WALL, which is neither
// floor nor wall but "not decided yet": fill_cave() comes along afterwards and
// turns every NULL_WALL and both temporary values into the rock the level
// wanted. A blank square also says "no monster here" (cptr 0), "nothing lying
// here" (tptr 0) and unlit, unseen, unmarked (pl, tl, fm, lr all false).
void dungeon_map_reset(void);

// The square at (y, x). Row first, then column -- this is the plain
// `&cave[y][x]` it replaces, with the same two subscripts in the same order and
// the same absence of any check.
//
// The square is handed out to be written through: about a fifth of the uses set
// a field, and they set it on the caller's side of this window.
cave_type *square_at(int y, int x);

// WHAT THIS MODULE DOES NOT ANSWER. The seven fields are seven questions, and
// the tree asks each of them in the same few words over and over. Measured,
// with comments stripped, the repeated forms are:
//
//     tptr != 0                    51   is something lying here?
//     cptr > 1                     29   is a monster standing here?
//     cptr == 1                     5   is the player standing here?
//     fval >= MIN_CLOSED_SPACE     28   is the way blocked?
//     fval <= MAX_OPEN_SPACE       15   is the way open?
//     fval >= MIN_CAVE_WALL        15   is this rock the player could tunnel?
//     fval <= MAX_CAVE_FLOOR        9   is this a floor of some kind?
//
// Those are the same question written in two directions (open/blocked) and the
// same boundary spelled two ways, which is the shape #18-14-6 found for "is the
// player in town". They are not folded here: each of them is a question about
// ONE SQUARE, answerable from the struct alone, and none of them needs this
// table to be private. Naming them is a separate unit's work, and the numbers
// above are written down so that unit does not have to measure again.
//
// TWO THINGS THIS QUESTION INHERITED AND KEPT:
//
//  1. THE SWEEP IN pusht(). A floor row does not know which square it lies on,
//     so when misc1.c moves the last row down into a hole it sweeps the level
//     looking for the square whose tptr is the row it moved. #18-14-7 left that
//     sweep here on purpose, because it needs this table. It is still a sweep;
//     it now walks through this window. Removing it means putting y and x in
//     the row, which upstream already wrote down as a wish at the struct
//     itself (types.h:136), and that is a change to what is stored, not to who
//     may reach it.
//
//  2. FOUR BITS FOR fval IN THE SAVE FILE. save.c packs fval into the low
//     nibble of a byte and lr, fm, pl, tl into four of the high bits, then
//     run-length encodes the table. constant.h says so at the fval list: "if
//     numbers above 15 are ever used, then the test against MIN_CAVE_WALL will
//     have to be changed, also the save routines will have to be changed."
//     Both halves of that warning are still true and neither is touched here.

#endif // DUNGEON_MAP_H
