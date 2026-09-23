// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The hit points this character rolled for every level it can reach

#ifndef HP_TABLE_H
#define HP_TABLE_H

// MAX_PLAYER_LEVEL comes from constant.h, which has to be included before this
// header.

// This is storage and only storage, the same as progress.h and score_death.h.
// The window does not range-check the level and does not roll anything, because
// the old code did neither: the rolling (and the bounds it re-rolls against)
// belongs to character creation, which needs randint and the class tables.
//
// The table is rolled once, at character creation, and never changes again --
// gaining a level does not roll new hit points, it reads the number that was
// decided at birth. The entries accumulate: level 1 holds the hit die, and each
// later level holds the previous total plus that level's roll. So the value at
// the top level is the character's whole life's worth of hit points, which is
// what creation checks against its bounds.

// The total rolled for this level, counting from 1: level 1 is the first entry.
// The old code wrote player_hp[lev - 1] at every reader, so the subtraction
// happens here now, in one place.
//
// The level is not checked. Levels outside 1..MAX_PLAYER_LEVEL read and write
// past the table, exactly as the old array subscript did.
uint16_t hp_total_at_level(int level);
void set_hp_total_at_level(int level, uint16_t total);

// The table itself, for the save file only. The save file reads and writes all
// MAX_PLAYER_LEVEL entries in one call (rd_shorts / wr_shorts), so it needs the
// address rather than the values -- the same reason death_cause() hands back a
// pointer. Nothing else should use this.
uint16_t *hp_table_slots(void);

#endif // HP_TABLE_H
