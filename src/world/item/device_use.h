// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Using a charged device (a staff or a wand): the steps the two commands share.

#ifndef DEVICE_USE_H
#define DEVICE_USE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint8_t tval;
    int penalty; // DEVICE_PENALTY_* (device.h)
    const char *none_carried;
    const char *prompt;
    const char *failed;
    const char *empty;
    // Called once the device is chosen, before the roll. Returns false to give
    // up; the turn is spent all the same. NULL when the device takes no
    // direction.
    bool (*choose_direction)(int *dir);
    // One effect, for bit `pos` (from 1) of the device's flags. Sets `*ident`
    // the way the effect wants. May replace `*flags`, the bits still to do.
    void (*effect)(int pos, int dir, uint32_t *flags, bool *ident);
} device_kind;

// Asks which device of the kind to use, rolls for it, then spends a charge on
// every effect it carries, or says it is empty.
void use_charged_device(const device_kind *kind);

#endif // DEVICE_USE_H
