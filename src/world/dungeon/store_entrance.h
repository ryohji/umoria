// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
// Copyright (c) 2026 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Store entrance handler registration: allows world/ to trigger store entry
// without depending on command/ layer

#ifndef STORE_ENTRANCE_H
#define STORE_ENTRANCE_H

// Register the store entrance handler. Called once at startup before any
// gameplay begins. The handler is invoked when the player steps onto a
// store entrance trap.
void set_store_entrance_handler(void (*handler)(int store_num));

#endif // STORE_ENTRANCE_H
