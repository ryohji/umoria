// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Which of the eight races this character is

#ifndef PLAYER_RACE_H
#define PLAYER_RACE_H

// THE QUESTION. Which race did the player pick? The answer is A ROW NUMBER into
// race[] (src/data/player.c:107), nothing more:
//
//   0 Human   1 Half-Elf   2 Elf        3 Halfling
//   4 Gnome   5 Dwarf      6 Half-Orc   7 Half-Troll
//
// THIS ONE IS NOT A QUANTITY. NOTHING IN THE GAME DOES ARITHMETIC ON THIS ONE
// except to find a row. There is no
// `_adjust` window here and there never will be -- A CHARACTER DOES NOT BECOME
// MORE OF A DWARF.
//
// WRITTEN EXACTLY TWICE IN A CHARACTER'S LIFE: once when the race menu is
// answered (create.c) and once when a saved game is read back (save.c). After
// that it never changes, which is why the three readers below can cache nothing
// and need nothing.
//
// BEWARE THE NAME. WHAT IS CROWDED IS `race`: the constant table race[], the shop
// owner's own owner_race, race_type.trace, and high_scores.race in death.c are
// four different things spelled with the same word.

// The row number. EIGHT CALLERS: the race's own row at creation three times
// (create.c), the start of the history chart (create.c), the class menu's mask
// (create.c), the high-score entry (death.c), and the shop prices twice
// (store_price.c, store_haggle.c).
//
// An int, though the field was a uint8_t: every caller immediately uses it as an
// array subscript, where an int is what C wants anyway.
int player_race(void);

// The row outright. TWO CALLERS: the race menu (create.c) and the saved file's
// byte put back (save.c).
//
// THAT IS THE SIXTH ANSWER to the question the fifteenth unit raised -- "does
// loading a saved game use the same window the race does?" -- and it is "the
// same", agreeing with the hit die, the base to-hit, the disarming skill and the
// saving throw rather than with the armour class.
//
// IT REFUSES NOTHING (findings.md 24).
// The menu can only produce 0..MAX_RACES-1 and a saved file can only hold a byte,
// but neither end was ever checked and this window does not start checking.
// WIDTH IS STILL A BYTE, so 256 lands on 0 and -1 lands on 255.
void player_race_set(int row);

// The race's name, as the game spells it -- "Human", "Half-Elf", ... THREE
// CALLERS, all of them display: the character sheet's side panel
// (status_line.c), the name/race/sex/class block (char_screen.c) and the dumped
// character file (files.c).
//
// THIS IS THE ONE PLACE THE MODULE REACHES OUT: race[] is one of the read-only
// constant tables declared in externs.h.
//
// WHY THE NAME IS IN HERE AND THE REST OF THE ROW IS NOT: what the callers want
// is the race's name. The other readers of race[] want the race's *stats*
// (the age spread, the height, the classes it may take), which is a different
// question and stays in create.c where the character is built.
//
// NOTE: this does not fold three calls into one. Each of the three sites still
// calls once, so the call count does not move -- the gain is that none of the
// three spells out a subscript.
//
// const char *, which is what race_type.trace already is (types.h:331) and what
// both receivers -- prt_field() and put_buffer() -- already take, so not one cast
// is needed anywhere.
const char *player_race_name(void);

// WHAT THIS MODULE DOES NOT ANSWER -- three things, all still in the callers:
//
//   1. WHAT THE SHOPS CHARGE. store_price.c:147 and store_haggle.c:499 index
//      rgold_adj[owner's race][this race]. That table is about A PAIR, not about
//      one race, and the two expressions differ anyway (one is `x / 100`, the
//      other `(200 - x) / 100`), so there is nothing to fold and the pricing
//      stays the shop's business.
//   2. WHERE THE LIFE STORY STARTS. create.c:194 computes `prace * 3 + 1` to find
//      the first row of background[]. That is knowledge about how background[] is
//      laid out -- the comment above it (create.c:185) says so -- not about the
//      race, and there is only one such site.
//   3. WHAT THE RACE IS LIKE. create.c reads the row whole three times, for the
//      stat adjustments, the age/height/weight spreads and the mask of classes
//      the race may take. "Which race is this?" and "what does that race give?"
//      are two questions, and only the first one is in here.

#endif // PLAYER_RACE_H
