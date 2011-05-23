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
 * Copyright (C) 2009-2010 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include "GP_Backend.h"
#include "config.h"

#ifdef GP_HAVE_SDL

#include <dlfcn.h>
#include <SDL/SDL.h>

struct GP_Backend GP_SDL_backend;
static SDL_Surface *GP_SDL_display = NULL;
static GP_Context GP_SDL_context;

/* Functions from the SDL library, dynamically loaded in GP_SDL_InitFn(). */
static void (*dyn_SDL_Quit)(void);
static int (*dyn_SDL_Init)(int);
static SDL_Surface *(*dyn_SDL_SetVideoMode)(int, int, int, uint32_t);
static void (*dyn_SDL_UpdateRect)(SDL_Surface *, int, int, int, int);
static int (*dyn_SDL_WaitEvent)(SDL_Event *);

/* User callbacks. */
static void (*GP_SDL_update_video_callback)(void) = NULL;

/* 
 * Checks whether pixel color component masks in the given surface are equal
 * to specified. Returns nonzero if they match, zero otherwise.
 */
static int GP_SDL_CheckPixelMasks(SDL_Surface *surf, unsigned int rmask,
	unsigned int gmask, unsigned int bmask, unsigned int amask)
{
	return (surf->format->Rmask == rmask
		&& surf->format->Gmask == gmask
		&& surf->format->Bmask == bmask
		&& surf->format->Ashift == amask);
}

/* 
 * Detects the pixel type of the SDL surface.
 * Returns the pixel type, or GP_PIXEL_UNKNOWN if the type was not recognized.
 */
static enum GP_PixelType GP_SDL_FindSurfacePixelType(SDL_Surface *surf)
{
	switch (surf->format->BytesPerPixel) {
		case 1:
			if (GP_SDL_CheckPixelMasks(surf, 0, 0, 0, 0)) {
				return GP_PIXEL_PAL8;
			}
			break;
		case 2:
			if (GP_SDL_CheckPixelMasks(surf, 0x7c00, 0x03e0, 0x001f, 0)) {
				return GP_PIXEL_RGB555;
			}
			if (GP_SDL_CheckPixelMasks(surf, 0xf800, 0x07e0, 0x001f, 0)) {
				return GP_PIXEL_RGB565;
			}
			break;
		case 3:
			if (GP_SDL_CheckPixelMasks(surf, 0xff0000, 0xff00, 0xff, 0)) {
				return GP_PIXEL_RGB888;
			}
			if (GP_SDL_CheckPixelMasks(surf, 0xff, 0xff00, 0xff0000, 0)) {
				return GP_PIXEL_BGR888;
			}
			break;
		case 4:
			if (GP_SDL_CheckPixelMasks(surf, 0xff0000, 0xff00, 0xff, 0)) {
				return GP_PIXEL_XRGB8888;
			}
			if (GP_SDL_CheckPixelMasks(surf, 0xff, 0xff00, 0xff0000, 0)) {
				return GP_PIXEL_XBGR8888;
			}
			if (GP_SDL_CheckPixelMasks(surf, 0xff000000, 0xff0000, 0xff00, 0)) {
				return GP_PIXEL_RGBX8888;
			}
			if (GP_SDL_CheckPixelMasks(surf, 0xff00, 0xff0000, 0xff000000, 0)) {
				return GP_PIXEL_BGRX8888;
			}
			if (GP_SDL_CheckPixelMasks(surf, 0xff0000, 0xff00, 0xff, 0xff000000)) {
				return GP_PIXEL_ARGB8888;
			}
			if (GP_SDL_CheckPixelMasks(surf, 0xff, 0xff00, 0xff0000, 0xff000000)) {
				return GP_PIXEL_ABGR8888;
			}
			if (GP_SDL_CheckPixelMasks(surf, 0xff000000, 0xff0000, 0xff00, 0xff)) {
				return GP_PIXEL_RGBA8888;
			}
			if (GP_SDL_CheckPixelMasks(surf, 0xff00, 0xff0000, 0xff000000, 0xff)) {
				return GP_PIXEL_BGRA8888;
			}
			break;

	}
	return GP_PIXEL_UNKNOWN;
}

inline GP_RetCode GP_SDL_ContextFromSurface(
		GP_Context *context, SDL_Surface *surf)
{
	GP_CHECK(context, "context is NULL");
	GP_CHECK(surf, "surface is NULL");

	/* sanity checks on the SDL surface */
	if (surf->format->BytesPerPixel == 0 || surf->format->BytesPerPixel > 4) {
		return GP_ENOIMPL;
	}
	enum GP_PixelType pixeltype = GP_SDL_FindSurfacePixelType(surf);
	if (pixeltype == GP_PIXEL_UNKNOWN) {
		return GP_ENOIMPL;
	}

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

	/* clipping */
	context->clip_h_min = surf->clip_rect.y;
	context->clip_h_max = surf->clip_rect.y + surf->clip_rect.h - 1;
	context->clip_w_min = surf->clip_rect.x;
	context->clip_w_max = surf->clip_rect.x + surf->clip_rect.w - 1;

	return GP_ESUCCESS;
}

static void GP_SDL_ShutdownFn(void)
{
	dyn_SDL_Quit();
}

static struct GP_Backend *GP_SDL_InitFn(void)
{
	void *library = dlopen("libSDL.so", RTLD_LAZY);
	if (!library)
		return NULL;

	dyn_SDL_Init = dlsym(library, "SDL_Init");
	dyn_SDL_Quit = dlsym(library, "SDL_Quit");
	dyn_SDL_UpdateRect = dlsym(library, "SDL_UpdateRect");
	dyn_SDL_SetVideoMode = dlsym(library, "SDL_SetVideoMode");
	dyn_SDL_WaitEvent = dlsym(library, "SDL_WaitEvent");

	if (dyn_SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0)
		return NULL;

	return &GP_SDL_backend;
}

static GP_Context *GP_SDL_OpenVideoFn(int w, int h, int flags)
{
	GP_SDL_display = dyn_SDL_SetVideoMode(w, h, 0,
			SDL_SWSURFACE|SDL_DOUBLEBUF);

	if (GP_SDL_display == NULL)
		return NULL;

	GP_RetCode retcode = GP_SDL_ContextFromSurface(
			&GP_SDL_context, GP_SDL_display);
	if (retcode != GP_ESUCCESS)
		return NULL;

	return &GP_SDL_context;
}

static GP_Context *GP_SDL_VideoContextFn(void)
{
	return &GP_SDL_context;
}

static void GP_SDL_UpdateVideoFn(void)
{
	dyn_SDL_UpdateRect(GP_SDL_display, 0, 0,
			GP_SDL_display->w, GP_SDL_display->h);
}

static int GP_SDL_GetEventFn(struct GP_BackendEvent *event)
{
	SDL_Event sdl_event;

	if (dyn_SDL_WaitEvent(&sdl_event) == 0)
		return 0;

	switch (sdl_event.type) {
		case SDL_VIDEOEXPOSE:
			event->type = GP_BACKEND_EVENT_UPDATE_VIDEO;
			return 1;
		case SDL_QUIT:
			event->type = GP_BACKEND_EVENT_QUIT_REQUEST;
			return 1;
	}

	/* for the time being, unknown events are simply ignored */
	return 0;
}

struct GP_Backend GP_SDL_backend = {
	.name = "SDL",
	.init_fn = GP_SDL_InitFn,
	.shutdown_fn = GP_SDL_ShutdownFn,
	.open_video_fn = GP_SDL_OpenVideoFn,
	.video_context_fn = GP_SDL_VideoContextFn,
	.update_video_fn = GP_SDL_UpdateVideoFn,
	.get_event_fn = GP_SDL_GetEventFn,
};

#endif /* GP_HAVE_SDL */
