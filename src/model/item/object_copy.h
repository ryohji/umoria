// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Copying an object template from object_list into an inven_type

#ifndef OBJECT_COPY_H
#define OBJECT_COPY_H

// inven_type comes from types.h, which has to be included before this header.

// Copy fields from object_list[from_index] into the inven_type pointed to
// by `to`. Used to initialize inventory slots, floor items, and store items
// from the definition table. Sets index to from_index, clears name2 and
// inscrip, copies all other fields from the template, and sets ident to 0.
void invcopy(inven_type *to, int from_index);

#endif // OBJECT_COPY_H
