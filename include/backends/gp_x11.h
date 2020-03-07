// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef BACKENDS_GP_X11_H
#define BACKENDS_GP_X11_H

#include <backends/gp_backend.h>

enum gp_x11_flags {
	/* When set, w and h is ignored and root window is used */
	GP_X11_USE_ROOT_WIN = 0x01,

	/* Create new borderless window above the root window */
	GP_X11_CREATE_ROOT_WIN = 0x02,

	/* Start fullscreen */
	GP_X11_FULLSCREEN = 0x04,

	/* Do not use MIT SHM even if available */
	GP_X11_DISABLE_SHM = 0x08,
};


/*
 * Initalize X11 backend.
 *
 * The display may be NULL for default display ($DISPLAY shell variable will
 * be used).
 *
 * The coordinates are position and geometry for newly created window.
 *
 * Upon failure NULL is returned.
 */
gp_backend *gp_x11_init(const char *display, int x, int y,
                        unsigned int w, unsigned int h,
			const char *caption,
			enum gp_x11_flags flags);

/*
 * Returns non-zero if backend is X11 backend
 */
int gp_backend_is_x11(gp_backend *self);

#endif /* BACKENDS_GP_X11_H */
