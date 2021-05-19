// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef BACKENDS_GP_SDL_H
#define BACKENDS_GP_SDL_H

#include <stdint.h>
#include <backends/gp_backend.h>

enum gp_sdl_flags {
	GP_SDL_FULLSCREEN = 0x01,
	GP_SDL_RESIZABLE  = 0x02,
};

/*
 * Initalize SDL as drawing backend.
 *
 * * SDL doesn't expose file descriptors.
 *
 * * The backend is thread safe (the critical parts are guarded with a mutex)
 *
 * * The backend is singleton, you can't have two SDL backends running at the
 *   same time.
 *
 * * When backend is allready initalized, this function ignores it's parameters
 *   and returns pointer to allready initalized SDL backend.
 *
 * * The SDL backends (upon calling gp_backend_poll()) feeds keyboard and mouse
 *   events into global GP event queue (see input/GP_Event.h).
 *
 *
 * The parameters w h and bpp are directly passed to SDL_SetVideoMode().
 *
 * * If w, h and/or bpp are set to zero, SDL tries to do best fit.
 *
 * * The gp_sdl_flags are converted into SDL equivalents.
 *
 * Upon failure, or if SDL wasn't compiled in, NULL is returned.
 */
gp_backend *gp_sdl_init(gp_size w, gp_size h, uint8_t bpp, uint8_t flags,
			const char *caption);

#endif /* BACKENDS_GP_SDL_H */
