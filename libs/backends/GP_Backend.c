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

#include "core/GP_Core.h"
#include "GP_Backend.h"
#include "../../config.h"

#include <string.h>

/*
 * The currently active backend (NULL if none).
 */
static struct GP_Backend *current_backend = NULL;

#ifdef HAVE_LIBSDL

extern struct GP_Backend GP_SDL_backend;

#endif

struct GP_Backend *GP_InitBackend(const char *name)
{
	if (current_backend)
		return current_backend;

#ifdef HAVE_LIBSDL

	if (!name || strcasecmp(name, "sdl") == 0) {
		current_backend = GP_SDL_backend.init_fn();
		return current_backend;
	}

#endif

	return NULL;
}

struct GP_Backend *GP_GetCurrentBackend(void)
{
	return current_backend;
}

GP_Context *GP_OpenBackendVideo(int w, int h, int flags)
{
	GP_CHECK(current_backend, "no current backend");
	return current_backend->open_video_fn(w, h, flags);
}

struct GP_Context *GP_GetBackendVideoContext(void)
{
	GP_CHECK(current_backend, "no current backend");
	return current_backend->video_context_fn();
}

void GP_UpdateBackendVideo(void)
{
	GP_CHECK(current_backend, "no current backend");
	return current_backend->update_video_fn();
}

int GP_GetBackendEvent(struct GP_BackendEvent *event)
{
	GP_CHECK(current_backend, "no current backend");
	return current_backend->get_event_fn(event);
}
