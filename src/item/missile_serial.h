// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// The serial number that tells one batch of missiles from another

#ifndef MISSILE_SERIAL_H
#define MISSILE_SERIAL_H

// Every stack of missiles made in the dungeon gets a number of its own, kept in
// the item's p1 field. Stacking compares it: items_can_stack() (misc3.c) lets
// two lots of the same arrow merge only when their p1 agree, so two batches
// found in different places stay apart even though they look identical. The
// numbers mean nothing beyond "not the same batch" -- nothing reads them as a
// count or an order.
//
// The next number, and the only way to make one. The counter walks the whole
// range of int16_t and wraps from the top back to the bottom, which is why the
// old code spelled the step out instead of writing ++: at MAX_SHORT the
// increment would leave the range, and the value has to stay something p1 can
// hold. Wrapping is safe for what the number is used for -- by the time it comes
// round, the batch that first had it is long gone.
int16_t next_missile_serial(void);

// The counter itself, for the save file only (save.c writes it as a short and
// reads it back). Everything else asks for the next number instead.
int16_t missile_serial_value(void);
void set_missile_serial(int16_t serial);

#endif // MISSILE_SERIAL_H
