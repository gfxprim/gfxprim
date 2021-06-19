// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#include "../../config.h"

#include <core/gp_debug.h>

#ifdef HAVE_LIBSDL2

#include <input/gp_input.h>
#include "gp_input_driver_sdl2.h"
#include <backends/gp_backend.h>
#include <backends/gp_sdl2.h>
#include <backends/gp_sdl2_pixmap.h>

#include <SDL2/SDL.h>

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Surface *surface;
static SDL_mutex *mutex;
static gp_pixmap pixmap;

/* To hold size from resize event */
static unsigned int new_w, new_h;

/* Backend API funcitons */
static struct gp_backend backend;

static void sdl2_flip(struct gp_backend *self __attribute__((unused)))
{
	SDL_LockMutex(mutex);

	pixmap.pixels = surface->pixels;
	SDL_UpdateWindowSurface(window);

	SDL_UnlockMutex(mutex);
}

static void sdl2_update_rect(struct gp_backend *self __attribute__((unused)),
                            gp_coord x0, gp_coord y0, gp_coord x1, gp_coord y1)
{
	SDL_LockMutex(mutex);

	const SDL_Rect rect = (SDL_Rect){x0, y0, GP_ABS(x1 - x0) + 1, GP_ABS(y1 - y0) + 1};
	SDL_UpdateWindowSurfaceRects(window, &rect, 1);

	SDL_UnlockMutex(mutex);
}

static void sdl2_put_event(SDL_Event *ev)
{
	switch (ev->type) {
		case SDL_WINDOWEVENT:
			if (ev->window.event == SDL_WINDOWEVENT_SIZE_CHANGED || ev->window.event == SDL_WINDOWEVENT_RESIZED) {
				new_w = ev->window.data1;
				new_h = ev->window.data2;
			}
		break;
	}

	gp_input_driver_sdl2_event_put(&backend.event_queue, ev);
}

static void sdl2_poll(struct gp_backend *self __attribute__((unused)))
{
	SDL_Event ev;
	SDL_LockMutex(mutex);

	while (SDL_PollEvent(&ev))
		sdl2_put_event(&ev);

	SDL_UnlockMutex(mutex);
}

static void sdl2_wait(struct gp_backend *self __attribute__((unused)))
{
	SDL_Event ev;
	if (SDL_WaitEvent(&ev) == 1) {
		sdl2_put_event(&ev);
	}
}

int gp_pixmap_from_sdl2_surface(gp_pixmap *pixmap, const SDL_Surface *surf)
{
	enum gp_pixel_type pixeltype;

	/* Sanity checks */
	if (surf->format->BytesPerPixel == 0) {
		GP_WARN("Surface->BytesPerPixel == 0");
		return 1;
	}
	if (surf->format->BytesPerPixel > 4) {
		GP_WARN("Surface->BytesPerPixel > 4");
		return 1;
	}

	pixeltype = gp_pixel_rgb_match(
		surf->format->Rmask,
		surf->format->Gmask,
		surf->format->Bmask,
		surf->format->Ashift,
		surf->format->BitsPerPixel
	);

	if (pixeltype == GP_PIXEL_UNKNOWN)
		return 1;

	/* Basic structure and size */
	pixmap->pixels = surf->pixels;
	pixmap->bpp = 8 * surf->format->BytesPerPixel;
	pixmap->pixel_type = pixeltype;
	pixmap->bytes_per_row = surf->pitch;
	pixmap->w = surf->w;
	pixmap->h = surf->h;

	return 0;
}

static int sdl2_set_attr(struct gp_backend *self, enum gp_backend_attrs attr,
                        const void *vals)
{
	SDL_LockMutex(mutex);

	switch (attr) {
	case GP_BACKEND_TITLE:
		SDL_SetWindowTitle(window, (const char *)vals);
	break;
	case GP_BACKEND_SIZE: {
		const int *size = vals;
		/* Send only resize event, the actual resize is done in resize_ack */
		gp_event_queue_push_resize(&self->event_queue, size[0], size[1], NULL);
	}
	break;
	case GP_BACKEND_FULLSCREEN: {
		const int fullscreen = *(const int *)vals;
		Uint32 flags = 0;
		if (fullscreen == 1) {
			flags = SDL_WINDOW_FULLSCREEN;
		}
		SDL_SetWindowFullscreen(window, flags);
	}
	break;
	default:
		GP_DEBUG(1, "Unimplemented backend attr %i", attr);
	break;
	}

	SDL_UnlockMutex(mutex);

	return 0;
}

static int sdl2_resize_ack(struct gp_backend *self __attribute__((unused)))
{
	GP_DEBUG(2, "Resizing the buffer to %ux%u", new_w, new_h);

	SDL_LockMutex(mutex);

	SDL_SetWindowSize(window, new_w, new_h);
	surface = SDL_GetWindowSurface(window);
	gp_pixmap_from_sdl2_surface(backend.pixmap, surface);
	gp_event_queue_set_screen_size(&backend.event_queue, backend.pixmap->w, backend.pixmap->h);

	SDL_UnlockMutex(mutex);

	return 0;
}

static void sdl2_exit(struct gp_backend *self __attribute__((unused)));

static struct gp_backend backend = {
	.name = "SDL2",
	.flip = sdl2_flip,
	.update_rect = sdl2_update_rect,
	.set_attr = sdl2_set_attr,
	.resize_ack = sdl2_resize_ack,
	.exit = sdl2_exit,
	.fd = -1,
	.poll = sdl2_poll,
	.wait = sdl2_wait,
};

static void sdl2_exit(struct gp_backend *self __attribute__((unused)))
{
	SDL_LockMutex(mutex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_UnlockMutex(mutex);
	SDL_DestroyMutex(mutex);
	SDL_Quit();

	backend.pixmap = NULL;
}

gp_backend *gp_sdl2_init(gp_size w, gp_size h, uint8_t bpp __attribute__((unused)), uint8_t flags,
                        const char *caption)
{
	Uint32 sdl2_flags = 0;

	/* SDL was already initalized */
	if (backend.pixmap != NULL)
		return &backend;

	/* SDL System */
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		GP_WARN("SDL_Init: %s", SDL_GetError());
		return NULL;
	}

	/* Window */
	if (flags & GP_SDL2_RESIZABLE) {
		sdl2_flags |= SDL_WINDOW_RESIZABLE;
	}
	if (flags & GP_SDL2_FULLSCREEN) {
		sdl2_flags |= SDL_WINDOW_FULLSCREEN;
	}
	window = SDL_CreateWindow(caption, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, sdl2_flags);
	if (!window) {
		GP_WARN("SDL_CreateWindow: %s", SDL_GetError());
		SDL_Quit();
		return NULL;
	}

	/* Surface */
	surface = SDL_GetWindowSurface(window);
	if (!surface) {
		GP_WARN("SDL_GetWindowSurface: %s", SDL_GetError());
		SDL_DestroyWindow(window);
		SDL_Quit();
		return NULL;
	}

	/* Renderer */
	renderer = SDL_CreateSoftwareRenderer(surface);
	if (!renderer) {
		GP_WARN("SDL_CreateRenderer: %s", SDL_GetError());
		SDL_DestroyWindow(window);
		SDL_Quit();
		return NULL;
	}

	/* Mutex */
	mutex = SDL_CreateMutex();

	/* Pixmap */
	if (gp_pixmap_from_sdl2_surface(&pixmap, surface)) {
		GP_WARN("Failed to match pixel_type");
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return NULL;
	}

	gp_event_queue_init(&backend.event_queue, w, h, 0);
	backend.pixmap = &pixmap;

	return &backend;
}

#else

#include <backends/gp_backend.h>

gp_backend *gp_sdl2_init(gp_size w __attribute__((unused)),
                        gp_size h __attribute__((unused)),
                        uint8_t bpp __attribute__((unused)),
                        uint8_t flags __attribute__((unused)),
                        const char *caption __attribute__((unused)))
{
	GP_WARN("SDL2 support not compiled in.");
	return NULL;
}

#endif /* HAVE_LIBSDL2 */
