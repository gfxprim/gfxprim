/*****************************************************************************
 * This file is part of gfxprim library.                                     *
 *                                                                           *
 * Gfxprim is free software; you can redistribute it and/or                  *
 * modify it under the terms of the GNU Lesser General Public                *
 * License as published by the Free Software Foundation; either              *
 * version 2.1 of the License, or (at your option) any later version.        *
 *                                                                           *
 * Gfxprim is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with gfxprim; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                        *
 * Boston, MA  02110-1301  USA                                               *
 *                                                                           *
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#ifndef BACKENDS_GP_X11_H
#define BACKENDS_GP_X11_H

#include <backends/GP_Backend.h>

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

/*
 * Changes full screen mode.
 *
 * 0 = off
 * 1 = on
 * 2 = toggle
 */
void gp_x11_fullscreen(gp_backend *self, int mode);

#endif /* BACKENDS_GP_X11_H */
