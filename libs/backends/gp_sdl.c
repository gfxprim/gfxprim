// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

#include "../../config.h"

#include <core/gp_debug.h>

#ifdef HAVE_LIBSDL

#if LIBSDL_VERSION == 1
# include <SDL/SDL.h>
# include <SDL/SDL_mutex.h>
#elif LIBSDL_VERSION == 2
# include <SDL2/SDL.h>
#endif

#include <input/gp_input.h>
#include <backends/gp_backend.h>
#include <backends/gp_clipboard.h>
#include <backends/gp_sdl.h>

#include <backends/gp_sdl_pixmap.h>

#include "gp_input_driver_sdl.h"

static SDL_Surface *sdl_surface;
static SDL_mutex *mutex;
static gp_pixmap pixmap;

#if LIBSDL_VERSION == 2
static SDL_Window *window;

static SDL_Cursor *cursor_arrow;
static SDL_Cursor *cursor_edit;
static SDL_Cursor *cursor_crosshair;
static SDL_Cursor *cursor_hand;
#endif

static uint32_t sdl_flags = SDL_SWSURFACE;

/* To hold size from resize event */
static unsigned int new_w, new_h;

static struct gp_backend backend;

static void sdl_flip(struct gp_backend *self __attribute__((unused)))
{
	SDL_mutexP(mutex);

#if LIBSDL_VERSION == 1
	SDL_Flip(sdl_surface);
#elif LIBSDL_VERSION == 2
	SDL_UpdateWindowSurface(window);
#endif
	pixmap.pixels = sdl_surface->pixels;

	SDL_mutexV(mutex);
}

static void sdl_update_rect(struct gp_backend *self __attribute__((unused)),
                            gp_coord x0, gp_coord y0, gp_coord x1, gp_coord y1)
{
	SDL_mutexP(mutex);

	gp_size w = GP_ABS(x1 - x0) + 1;
	gp_size h = GP_ABS(y1 - y0) + 1;

#if LIBSDL_VERSION == 1
	/*
	 * SDL_UpdateRect() with all x0, y0, x1 and y1 zero updates whole
	 * screen we avoid such behavior as it will break other backends.
	 */
	if (x1 != 0 && y1 != 0)
		SDL_UpdateRect(sdl_surface, x0, y0, w, h);
#elif LIBSDL_VERSION == 2
	const SDL_Rect rect = {x0, y0, w, h};
	SDL_UpdateWindowSurfaceRects(window, &rect, 1);
#endif

	SDL_mutexV(mutex);
}

//TODO: Move to priv!
static gp_ev_queue ev_queue;

static void sdl_put_event(SDL_Event *ev)
{
#if LIBSDL_VERSION == 1
	if (ev->type == SDL_VIDEORESIZE) {
		new_w = ev->resize.w;
		new_h = ev->resize.h;
	}
#endif
	gp_input_driver_sdl_event_put(&backend, backend.event_queue, ev);
}

static void sdl_poll(struct gp_backend *self __attribute__((unused)))
{
	SDL_Event ev;

	SDL_mutexP(mutex);

	while (SDL_PollEvent(&ev) && !gp_ev_queue_full(backend.event_queue))
		sdl_put_event(&ev);

	SDL_mutexV(mutex);
}

static void sdl_wait(struct gp_backend *self)
{
	SDL_Event ev;

	for (;;) {
		if (gp_ev_queue_events(self->event_queue))
			return;

		SDL_mutexP(mutex);

		while (SDL_PollEvent(&ev))
			sdl_put_event(&ev);

		SDL_mutexV(mutex);

		usleep(10000);
	}
}

static enum gp_backend_ret sdl_set_cursor(enum gp_backend_cursor_req cursor)
{
#if LIBSDL_VERSION == 2
	switch (cursor) {
	case GP_BACKEND_CURSOR_HIDE:
		SDL_ShowCursor(SDL_DISABLE);
	break;
	case GP_BACKEND_CURSOR_SHOW:
		SDL_ShowCursor(SDL_ENABLE);
	break;
	case GP_BACKEND_CURSOR_ARROW:
		if (!cursor_arrow)
			cursor_arrow = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);

		if (cursor_arrow)
			SDL_SetCursor(cursor_arrow);
	break;
	case GP_BACKEND_CURSOR_TEXT_EDIT:
		if (!cursor_edit)
			cursor_edit = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);

		if (cursor_edit)
			SDL_SetCursor(cursor_edit);
	break;
	case GP_BACKEND_CURSOR_CROSSHAIR:
		if (!cursor_crosshair)
			cursor_crosshair = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);

		if (cursor_crosshair)
			SDL_SetCursor(cursor_crosshair);
	break;
	case GP_BACKEND_CURSOR_HAND:
		if (!cursor_hand)
			cursor_hand = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);

		if (cursor_hand)
			SDL_SetCursor(cursor_hand);
	break;
	default:
		return GP_BACKEND_NOTSUPP;
	}

	return GP_BACKEND_OK;
#else
	(void) cursor;

	return GP_BACKEND_NOTSUPP;
#endif
}

static enum gp_backend_ret sdl_set_fullscreen(enum gp_backend_fullscreen_req req)
{
#if LIBSDL_VERSION == 2
	uint32_t flags = 0;
	int fullscreen = SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN;

	switch (req) {
	case GP_BACKEND_FULLSCREEN_ON:
		flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
	break;
	case GP_BACKEND_FULLSCREEN_OFF:
		flags = 0;
	break;
	case GP_BACKEND_FULLSCREEN_TOGGLE:
		flags = fullscreen ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP;
	break;
	case GP_BACKEND_FULLSCREEN_QUERY:
		return !!fullscreen;
	}

	if (SDL_SetWindowFullscreen(window, flags))
		return GP_BACKEND_CONNERR;

	return GP_BACKEND_OK;
#else
	(void) req;
	return GP_BACKEND_NOTSUPP;
#endif
}

static enum gp_backend_ret sdl_set_attr(struct gp_backend *self,
                                        enum gp_backend_attr attr,
                                        const void *vals)
{
	SDL_mutexP(mutex);
	enum gp_backend_ret ret = GP_BACKEND_NOTSUPP;

	switch (attr) {
	case GP_BACKEND_ATTR_TITLE:
#if LIBSDL_VERSION == 1
		SDL_WM_SetCaption(vals, vals);
#elif LIBSDL_VERSION == 2
		SDL_SetWindowTitle(window, (const char *)vals);
#endif
		ret = GP_BACKEND_OK;
	break;
	case GP_BACKEND_ATTR_SIZE: {
		const int *size = vals;
		/* Send only resize event, the actual resize is done in resize_ack */
		gp_ev_queue_push_resize(self->event_queue, size[0], size[1], 0);
		ret = GP_BACKEND_OK;
	}
	break;
	case GP_BACKEND_ATTR_FULLSCREEN:
		ret = sdl_set_fullscreen(*(int *)vals);
	break;
	case GP_BACKEND_ATTR_CURSOR:
		ret = sdl_set_cursor(*(enum gp_backend_cursor_req *)vals);
	break;
	default:
		GP_WARN("Unsupported backend attribute %i", (int) attr);
	break;
	}

	SDL_mutexV(mutex);

	return ret;
}

static int sdl_resize_ack(struct gp_backend *self __attribute__((unused)))
{
	GP_DEBUG(2, "Resizing the buffer to %ux%u", new_w, new_h);

	SDL_mutexP(mutex);

#if LIBSDL_VERSION == 1
	sdl_surface = SDL_SetVideoMode(new_w, new_h, 0, sdl_flags);
#elif LIBSDL_VERSION == 2
	SDL_FreeSurface(sdl_surface);
	sdl_surface = SDL_GetWindowSurface(window);
#endif

	gp_pixmap_from_sdl_surface(backend.pixmap, sdl_surface);
	gp_ev_queue_set_screen_size(backend.event_queue,
	                            backend.pixmap->w, backend.pixmap->h);

	SDL_mutexV(mutex);

	return 0;
}

#if LIBSDL_VERSION == 2
static int sdl_clipboard(gp_backend *self, gp_clipboard *op)
{
	char *tmp;

	switch (op->op) {
	case GP_CLIPBOARD_SET:
		if (op->len) {
			tmp = strndup(op->str, op->len);
			if (!tmp)
				return 1;
			SDL_SetClipboardText(tmp);
			free(tmp);
		} else {
			SDL_SetClipboardText(op->str);
		}
	break;
	case GP_CLIPBOARD_REQUEST:
		gp_backend_clipboard_ready(self);
	break;
	case GP_CLIPBOARD_GET:
		op->ret = SDL_GetClipboardText();
	break;
	case GP_CLIPBOARD_CLEAR:
	break;
	}

	return 0;
}
#endif

static void sdl_exit(struct gp_backend *self __attribute__((unused)));

static struct gp_backend backend = {
	.name = "SDL",
	.flip = sdl_flip,
	.update_rect = sdl_update_rect,
	.set_attr = sdl_set_attr,
	.resize_ack = sdl_resize_ack,
#if LIBSDL_VERSION == 2
	.clipboard = sdl_clipboard,
#endif
	.exit = sdl_exit,
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

gp_backend *gp_sdl_init(gp_size w, gp_size h, uint8_t bpp,
                        enum gp_sdl_flags flags, const char *caption)
{
	/* SDL was already initalized */
	if (backend.pixmap != NULL)
		return &backend;

	if (SDL_Init(SDL_INIT_VIDEO)) {
		GP_WARN("SDL_Init: %s", SDL_GetError());
		return NULL;
	}

	if (flags & GP_SDL_FULLSCREEN)
#if LIBSDL_VERSION == 1
		sdl_flags |= SDL_FULLSCREEN;
#elif LIBSDL_VERSION == 2
		sdl_flags |= SDL_WINDOW_FULLSCREEN;
#endif

	if (flags & GP_SDL_RESIZABLE)
#if LIBSDL_VERSION == 1
		sdl_flags |= SDL_RESIZABLE;
#elif LIBSDL_VERSION == 2
		sdl_flags |= SDL_WINDOW_RESIZABLE;
#endif

#if LIBSDL_VERSION == 1
	sdl_surface = SDL_SetVideoMode(w, h, bpp, sdl_flags);

	if (caption != NULL)
		SDL_WM_SetCaption(caption, caption);

	if (!sdl_surface) {
		GP_WARN("SDL_SetVideoMode: %s", SDL_GetError());
		SDL_Quit();
		return NULL;
	}

	SDL_EnableUNICODE(1);
#elif LIBSDL_VERSION == 2
	(void) bpp;

	window = SDL_CreateWindow(caption, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, sdl_flags);
	if (!window) {
		GP_WARN("SDL_CreateWindow: %s", SDL_GetError());
		SDL_Quit();
		return NULL;
	}

	sdl_surface = SDL_GetWindowSurface(window);
	if (!sdl_surface) {
		GP_WARN("SDL_GetWindowSurface: %s", SDL_GetError());
		SDL_DestroyWindow(window);
		SDL_Quit();
		return NULL;
	}
#endif

	if (gp_pixmap_from_sdl_surface(&pixmap, sdl_surface)) {
		GP_WARN("Failed to match pixel_type");
		SDL_Quit();
		return NULL;
	}

	mutex = SDL_CreateMutex();

#if LIBSDL_VERSION == 1
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,
	                    SDL_DEFAULT_REPEAT_INTERVAL);
#endif

	backend.event_queue = &ev_queue;

	gp_ev_queue_init(backend.event_queue, w, h, 0, NULL, NULL,
#if LIBSDL_VERSION == 1
			0
#else
			GP_EVENT_QUEUE_LOAD_KEYMAP
#endif
			);

	backend.pixmap = &pixmap;

#if LIBSDL_VERSION == 1
	backend.dpi = 0;
#else
	float dpi_v, dpi_h;

	SDL_GetDisplayDPI(0, &dpi_v, &dpi_h, NULL);
	backend.dpi = (dpi_v + dpi_h + 1)/2;
#endif

	return &backend;
}

#else

#include <backends/gp_sdl.h>

gp_backend *gp_sdl_init(gp_size w __attribute__((unused)),
                        gp_size h __attribute__((unused)),
                        uint8_t bpp __attribute__((unused)),
                        enum gp_sdl_flags flags __attribute__((unused)),
                        const char *caption __attribute__((unused)))
{
	GP_FATAL("Neither of SDL or SDL2 support not compiled in.");
	return NULL;
}

#endif /* HAVE_LIBSDL */
