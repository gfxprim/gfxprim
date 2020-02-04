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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#ifndef BACKENDS_GP_FRAMEBUFFER_H
#define BACKENDS_GP_FRAMEBUFFER_H

#include <backends/gp_backend.h>

enum gp_linux_fb_flags {
	/*
	 * Use KBD to get input events
	 */
	GP_FB_INPUT_KBD = 0x01,
	/*
	 * Use shadow framebuffer for drawing.
	 */
	GP_FB_SHADOW = 0x02,
	/*
	 * Allocate new console, if not set current is used.
	 */
	GP_FB_ALLOC_CON = 0x04,
};

/*
 * Initalize framebuffer.
 *
 * The path should point to framebuffer device eg. "/dev/fb0" for first
 * framebuffer device.
 *
 * The gp_backend structure is allocated and returned, the resources are
 * deinitalized and the structure is freed by backed->Exit(backend); call.
 *
 * Upon failure NULL is returned.
 *
 * If flag is set, the konsole kbd is used to push events into event queue.
 */
gp_backend *gp_linux_fb_init(const char *path, int flags);

#endif /* BACKENDS_GP_FRAMEBUFFER_H */
