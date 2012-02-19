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

#ifndef BACKENDS_GP_SDL_H
#define BACKENDS_GP_SDL_H

#include <stdint.h>
#include "backends/GP_Backend.h"

enum GP_BackendSDLFlags {
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
 * * The SDL backends (upon calling GP_BackendPoll()) feeds keyboard and mouse
 *   events into global GP event queue (see input/GP_Event.h).
 *
 *
 * The parameters w h and bpp are directly passed to SDL_SetVideoMode().
 *
 * * If w, h and/or bpp are set to zero, SDL tries to do best fit.
 * 
 * * The GP_BackendSDLFlags are converted into SDL equivalents.
 *
 * Upon failure, or if SDL wasn't compiled in, NULL is returned.
 */
GP_Backend *GP_BackendSDLInit(GP_Size w, GP_Size h,
                              uint8_t bpp, uint8_t flags,
			      const char *caption);

#endif /* BACKENDS_GP_SDL_H */
