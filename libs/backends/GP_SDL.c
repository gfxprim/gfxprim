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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include "../../config.h"

#include <core/gp_debug.h>

#ifdef HAVE_LIBSDL

#include "input/GP_Input.h"
#include "GP_InputDriverSDL.h"
#include <backends/gp_backend.h>
#include <backends/gp_sdl.h>

#include <backends/gp_sdl_pixmap.h>

#include <SDL/SDL.h>
#include <SDL/SDL_mutex.h>

static SDL_Surface *sdl_surface;
static SDL_mutex *mutex;
static gp_pixmap pixmap;

static uint32_t sdl_flags = SDL_SWSURFACE;

/* To hold size from resize event */
static unsigned int new_w, new_h;

/* Backend API funcitons */
static struct gp_backend backend;

static void sdl_flip(struct gp_backend *self __attribute__((unused)))
{
	SDL_mutexP(mutex);

	SDL_Flip(sdl_surface);
	pixmap.pixels = sdl_surface->pixels;

	SDL_mutexV(mutex);
}

static void sdl_update_rect(struct gp_backend *self __attribute__((unused)),
                            gp_coord x0, gp_coord y0, gp_coord x1, gp_coord y1)
{
	SDL_mutexP(mutex);

	/*
	 * SDL_UpdateRect() with all x0, y0, x1 and y1 zero updates whole
	 * screen we avoid such behavior as it will break other backends.
	 */
	if (x1 != 0 && y1 != 0)
		SDL_UpdateRect(sdl_surface, x0, y0,
		               GP_ABS(x1 - x0) + 1, GP_ABS(y1 - y0) + 1);

	SDL_mutexV(mutex);
}

static void sdl_put_event(SDL_Event *ev)
{
	if (ev->type == SDL_VIDEORESIZE) {
		new_w = ev->resize.w;
		new_h = ev->resize.h;
	}

	gp_input_driver_sdl_event_put(&backend.event_queue, ev);
}

static void sdl_poll(struct gp_backend *self __attribute__((unused)))
{
	SDL_Event ev;

	SDL_mutexP(mutex);

	while (SDL_PollEvent(&ev))
		sdl_put_event(&ev);

	SDL_mutexV(mutex);
}

static void sdl_wait(struct gp_backend *self __attribute__((unused)))
{
	SDL_Event ev;

	//SDL_WaitEvent(&ev);
	//sdl_put_event(&ev);

	for (;;) {
		if (gp_event_queue_events_queued(&self->event_queue))
			return;

		SDL_mutexP(mutex);

		while (SDL_PollEvent(&ev))
			sdl_put_event(&ev);

		SDL_mutexV(mutex);

		usleep(10000);
	}
}

int gp_pixmap_from_sdl_surface(gp_pixmap *pixmap, const SDL_Surface *surf)
{
	/* sanity checks on the SDL surface */
	if (surf->format->BytesPerPixel == 0) {
		GP_WARN("Surface->BytesPerPixel == 0");
		return 1;
	}

	if (surf->format->BytesPerPixel > 4) {
		GP_WARN("Surface->BytesPerPixel > 4");
		return 1;
	}

	enum gp_pixel_type pixeltype = gp_pixel_rgb_match(surf->format->Rmask,
							  surf->format->Gmask,
							  surf->format->Bmask,
							  surf->format->Ashift,
							  surf->format->BitsPerPixel);

	if (pixeltype == GP_PIXEL_UNKNOWN)
		return 1;

	/* basic structure and size */
	pixmap->pixels = surf->pixels;
	pixmap->bpp = 8 * surf->format->BytesPerPixel;
	pixmap->pixel_type = pixeltype;
	pixmap->bytes_per_row = surf->pitch;
	pixmap->w = surf->w;
	pixmap->h = surf->h;

	return 0;
}

static int sdl_set_attr(struct gp_backend *self, enum gp_backend_attrs attr,
                        const void *vals)
{
	SDL_mutexP(mutex);

	switch (attr) {
	case GP_BACKEND_TITLE:
		SDL_WM_SetCaption(vals, vals);
	break;
	case GP_BACKEND_SIZE: {
		const int *size = vals;
		/* Send only resize event, the actual resize is done in resize_ack */
		gp_event_queue_push_resize(&self->event_queue, size[0], size[1], NULL);
	}
	break;
	default:
		GP_DEBUG(1, "Unimplemented backend attr %i", attr);
	break;
	}

	SDL_mutexV(mutex);

	return 0;
}

static int sdl_resize_ack(struct gp_backend *self __attribute__((unused)))
{
	GP_DEBUG(2, "Resizing the buffer to %ux%u", new_w, new_h);

	SDL_mutexP(mutex);

	sdl_surface = SDL_SetVideoMode(new_w, new_h, 0, sdl_flags);
	gp_pixmap_from_sdl_surface(backend.pixmap, sdl_surface);

	gp_event_queue_set_screen_size(&backend.event_queue,
	                               backend.pixmap->w, backend.pixmap->h);

	SDL_mutexV(mutex);

	return 0;
}

static void sdl_exit(struct gp_backend *self __attribute__((unused)));

static struct gp_backend backend = {
	.name = "SDL",
	.flip = sdl_flip,
	.update_rect = sdl_update_rect,
	.set_attr = sdl_set_attr,
	.resize_ack = sdl_resize_ack,
	.exit = sdl_exit,
	.fd = -1,
	.poll = sdl_poll,
	.wait = sdl_wait,
};

static void sdl_exit(struct gp_backend *self __attribute__((unused)))
{
	SDL_mutexP(mutex);

	SDL_Quit();

	SDL_DestroyMutex(mutex);

	backend.pixmap = NULL;
}

gp_backend *gp_sdl_init(gp_size w, gp_size h, uint8_t bpp, uint8_t flags,
                        const char *caption)
{
	/* SDL was already initalized */
	if (backend.pixmap != NULL)
		return &backend;

	if (SDL_Init(SDL_INIT_VIDEO)) {
		GP_WARN("SDL_Init: %s", SDL_GetError());
		return NULL;
	}

	if (flags & GP_SDL_FULLSCREEN)
		sdl_flags |= SDL_FULLSCREEN;

	if (flags & GP_SDL_RESIZABLE)
		sdl_flags |= SDL_RESIZABLE;

	sdl_surface = SDL_SetVideoMode(w, h, bpp, sdl_flags);

	if (caption != NULL)
		SDL_WM_SetCaption(caption, caption);

	if (sdl_surface == NULL) {
		GP_WARN("SDL_SetVideoMode: %s", SDL_GetError());
		SDL_Quit();
		return NULL;
	}

	mutex = SDL_CreateMutex();

	if (gp_pixmap_from_sdl_surface(&pixmap, sdl_surface)) {
		GP_WARN("Failed to match pixel_type");
		SDL_Quit();
		return NULL;
	}

	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,
	                    SDL_DEFAULT_REPEAT_INTERVAL);

	gp_event_queue_init(&backend.event_queue, w, h, 0);

	backend.pixmap = &pixmap;

	return &backend;
}

#else

#include <backends/gp_backend.h>

gp_backend *gp_sdl_init(gp_size w __attribute__((unused)),
                        gp_size h __attribute__((unused)),
                        uint8_t bpp __attribute__((unused)),
                        uint8_t flags __attribute__((unused)),
                        const char *caption __attribute__((unused)))
{
	GP_FATAL("SDL support not compiled in.");
	return NULL;
}

#endif /* HAVE_LIBSDL */
