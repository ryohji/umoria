// Copyright (c) 2025 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Platform abstraction layer
// Provides unified initialization for rendering, input, and system services

#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>

// Initialize platform services (rendering + input)
// Returns true on success, false on failure
bool platform_init(void);

// Shutdown platform services
void platform_shutdown(void);

#endif // PLATFORM_H
