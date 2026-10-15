// Copyright (c) 1989-94 James E. Wilson, Robert A. Koeneke
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Fixed-size text buffers used throughout the game.

#ifndef TEXT_BUFFERS_H
#define TEXT_BUFFERS_H

#define VTYPESIZ 80
#define BIGVTYPESIZ 160
typedef char vtype[VTYPESIZ];
// note that since its output can easily exceed 80 characters, objdes must
// always be called with a bigvtype as the first paramter
typedef char bigvtype[BIGVTYPESIZ];

// For formatted messages like "Your %s glows faintly!" Needs extra space beyond
// the embedded string (bigvtype) to hold prefix and suffix text. Sized to hold
// bigvtype + vtype to prevent overflow.
#define MSGTYPESIZ (BIGVTYPESIZ + VTYPESIZ)
typedef char msgtype[MSGTYPESIZ];

#endif // TEXT_BUFFERS_H
