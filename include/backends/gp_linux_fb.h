// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

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
	/*
	 * Use Linux input
	 *
	 * Cannot be enabled together with KBD!
	 */
	GP_FB_INPUT_LINUX = 0x08,
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
