// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2021-2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Array length helpers.

#ifndef LENGTH_OF_H
#define LENGTH_OF_H

#define LENGTH_OF(a) (sizeof(a) / sizeof((a)[0]))

#define END_OF(a) ((a) + LENGTH_OF(a))

#endif // LENGTH_OF_H
