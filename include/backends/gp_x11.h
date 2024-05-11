// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_x11.h
 * @brief A X11 backend.
 *
 * X11 backend supports multiple windows, you can call the init function
 * multiple times, each time you get a backend representing a new X11 window
 * that will share the underlying X11 server connection.
 *
 * @include{c} demos/c_simple/x11_windows.c
 */
#ifndef BACKENDS_GP_X11_H
#define BACKENDS_GP_X11_H

#include <backends/gp_backend.h>

/** @brief A X11 backend init flags. */
enum gp_x11_flags {
	/** @brief When set, w and h is ignored and root window is used. */
	GP_X11_USE_ROOT_WIN = 0x01,
	/** @brief Create new borderless window above the root window. */
	GP_X11_CREATE_ROOT_WIN = 0x02,
	/** @brief Start fullscreen. */
	GP_X11_FULLSCREEN = 0x04,
	/** @brief Do not use MIT SHM even if available. */
	GP_X11_DISABLE_SHM = 0x08,
};

/**
 * @brief X11 backend initialization.
 *
 * @param display A X11 display. If NULL default is used.
 * @param x A window X offset.
 * @param y A window Y offset.
 * @param w A window width.
 * @param h A window height.
 * @param caption A window caption.
 * @param flags A bitwise combination of enum #gp_x11_flags.
 * @return A newly allocated backend, or NULL in a case of a failure, or if X11
 *         support wasn't compiled in.
 */
gp_backend *gp_x11_init(const char *display, int x, int y,
                        unsigned int w, unsigned int h,
			const char *caption,
			enum gp_x11_flags flags);

#endif /* BACKENDS_GP_X11_H */
