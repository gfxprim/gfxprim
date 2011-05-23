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

#ifndef GP_BACKEND_H
#define GP_BACKEND_H

#include "core/GP_Context.h"

/*
 * Types of events provided by the backend.
 */
enum GP_BackendEventType {
	GP_BACKEND_EVENT_NONE = 0,
	GP_BACKEND_EVENT_UPDATE_VIDEO = 1,	/* video redraw is needed */
	GP_BACKEND_EVENT_QUIT_REQUEST = 2,	/* user requests quitting */
};

/*
 * Structure describing an event reported by a backend.
 */
struct GP_BackendEvent {
	enum GP_BackendEventType type;
};

/* Describes a backend and holds all its API functions. */
struct GP_Backend {
	const char *name;
	struct GP_Backend *(*init_fn)(void);
	void (*shutdown_fn)(void);
	GP_Context *(*open_video_fn)(int w, int h, int flags);
	GP_Context *(*video_context_fn)(void);
	void (*update_video_fn)(void);
	int (*get_event_fn)(struct GP_BackendEvent *event);
};

/*
 * Attempts to initialize a backend.
 * 
 * If name is specified, only that backend is tried; if name is NULL,
 * all known backends are tried and the first usable one is picked.
 *
 * Returns the backend structure, or NULL on failure.
 */
struct GP_Backend *GP_InitBackend(const char *name);

/*
 * Opens the backend video and returns its context.
 */
GP_Context *GP_OpenBackendVideo(int w, int h, int flags);

/*
 * Returns a pointer to context that represents the backend's video.
 */
GP_Context *GP_GetBackendVideoContext(void);

/*
 * Calls the backend to update its video to reflect new changes.
 * If the backend uses double buffering, this causes a buffer flip.
 * If the backend uses direct-to-screen drawing, this call
 * has no effect.
 */
void GP_UpdateBackendVideo(void);

/*
 * Reads the first pending backend event.
 * Returns 0 if no events were pending, 1 on success.
 */
int GP_GetBackendEvent(struct GP_BackendEvent *event);

#endif /* GP_BACKEND_H */
