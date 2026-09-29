// Copyright (c) 2025 Umoria Contributors
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Unified ncurses backend interface
// Provides both rendering and input backends for ncurses

#ifndef BACKEND_NCURSES_H
#define BACKEND_NCURSES_H

#include "render.h"
#include "input.h"

// Get ncurses rendering backend
RenderBackend *render_ncurses_backend(void);

// Get ncurses input backend
InputBackend *input_ncurses_backend(void);

#endif // BACKEND_NCURSES_H
