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
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include "../../config.h"

#include "core/GP_Debug.h"
#include "input/GP_InputDriverSDL.h"
#include "GP_Backend.h"
#include "GP_SDL.h"

#ifdef HAVE_LIBSDL

#include <SDL/SDL.h>
#include <SDL/SDL_mutex.h>

static SDL_Surface *sdl_surface;
static SDL_mutex *mutex;
static GP_Context context;

/* Backend API funcitons */

static void sdl_flip(struct GP_Backend *self __attribute__((unused)))
{
	SDL_mutexP(mutex);

	SDL_Flip(sdl_surface);
	context.pixels = sdl_surface->pixels;

	SDL_mutexV(mutex);
}

static void sdl_update_rect(struct GP_Backend *self __attribute__((unused)),
                            GP_Coord x1, GP_Coord y1, GP_Coord x2, GP_Coord y2)
{
	SDL_mutexP(mutex);
	
	SDL_UpdateRect(sdl_surface, x1, y1, x2, y2);
	
	SDL_mutexV(mutex);
}

static void sdl_poll(struct GP_Backend *self __attribute__((unused)))
{
	SDL_Event ev;

	SDL_mutexP(mutex);
	
	while (SDL_PollEvent(&ev))
		GP_InputDriverSDLEventPut(&ev);
	
	SDL_mutexV(mutex);
}

static void sdl_exit(struct GP_Backend *self __attribute__((unused)));

static struct GP_Backend backend = {
	.name       = "SDL",
	.context    = NULL,
	.Flip       = sdl_flip,
	.UpdateRect = sdl_update_rect,
	.Exit       = sdl_exit,
	.fd_list    = NULL,
	.Poll       = sdl_poll,
};

static void sdl_exit(struct GP_Backend *self __attribute__((unused)))
{
	SDL_mutexP(mutex);
	
	SDL_Quit();
	
	SDL_DestroyMutex(mutex);

	backend.context = NULL;
}

int context_from_surface(GP_Context *context, SDL_Surface *surf)
{
	/* sanity checks on the SDL surface */
	if (surf->format->BytesPerPixel == 0) {
		GP_DEBUG(1, "ERROR: Surface->BytesPerPixel == 0");
		return 1;
	}

	if (surf->format->BytesPerPixel > 4) {
		GP_DEBUG(1, "ERROR: Surface->BytesPerPixel > 4");
		return 1;
	}

	enum GP_PixelType pixeltype = GP_PixelRGBMatch(surf->format->Rmask,
	                                               surf->format->Gmask,
						       surf->format->Bmask,
						       surf->format->Ashift,
						       surf->format->BitsPerPixel);

	if (pixeltype == GP_PIXEL_UNKNOWN)
		return 1;

	/* basic structure and size */
	context->pixels = surf->pixels;
	context->bpp = 8 * surf->format->BytesPerPixel;
	context->pixel_type = pixeltype;
	context->bytes_per_row = surf->pitch;
	context->w = surf->w;
	context->h = surf->h;

	/* orientation */
	context->axes_swap = 0;
	context->x_swap = 0;
	context->y_swap = 0;

	return 0;
}

GP_Backend *GP_BackendSDLInit(GP_Size w, GP_Size h, uint8_t bpp, uint8_t flags,
                              const char *caption)
{
	/* SDL not yet initalized */
	if (backend.context == NULL) {
		if (SDL_Init(SDL_INIT_VIDEO)) {
			GP_DEBUG(1, "ERROR: SDL_Init: %s", SDL_GetError());
			return NULL;
		}

		uint32_t sdl_flags = SDL_SWSURFACE;

		if (flags & GP_SDL_FULLSCREEN)
			sdl_flags |= SDL_FULLSCREEN;

		if (flags & GP_SDL_RESIZABLE)
			sdl_flags |= SDL_RESIZABLE;

		sdl_surface = SDL_SetVideoMode(w, h, bpp, sdl_flags);

		if (caption != NULL)
			SDL_WM_SetCaption(caption, caption);

		if (sdl_surface == NULL) {
			GP_DEBUG(1, "ERROR: SDL_SetVideoMode: %s", SDL_GetError());
			SDL_Quit();
			return NULL;
		}

		mutex = SDL_CreateMutex();

		if (context_from_surface(&context, sdl_surface)) {
			GP_DEBUG(1, "ERROR: Failed to match pixel_type");
			SDL_Quit();
			return NULL;
		}

		backend.context = &context;
	}

	return &backend;
}

#else

GP_Backend *GP_BackendSDLInit(GP_Size w, GP_Size h, uint8_t bpp, uint8_t flags,
                              const char *caption)
{
	return NULL;
}

#endif /* HAVE_LIBSDL */
