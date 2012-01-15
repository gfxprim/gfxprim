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

/*

  The GP_Backend is overall structure for API for managing
  connection/mmaped memory/... to xserver window/framebuffer/... .

  In contrast to other graphics libraries we do not try to create overall
  initalization interface that would match specialities for every possible
  backend. Rather than that we are trying to create API that is the same
  for all backends, once initalization is done.

  So once you initalize, for example, framebuffer driver, the GP_Backend
  structure is returned, which then could be used with generic code for
  backend drawing.

 */

#ifndef BACKENDS_GP_BACKEND_H
#define BACKENDS_GP_BACKEND_H

#include "core/GP_Context.h"

struct GP_Backend;

/*
 * Linked list of file descriptors with callbacks. 
 */
typedef struct GP_BackendFD {
	int fd;
	void (*Callback)(struct GP_BackendFD *self, struct GP_Backend *backend);
	struct GP_BackendFD *next;
} GP_BackendFD;

typedef struct GP_Backend {
	/*
	 * Backend name.
	 */
	const char *name;

	/* 
	 * Pointer to context app should draw to.
	 *
	 * This MAY change upon a flip operation.
	 */
	GP_Context *context;

	/*
	 * If display is buffered, this copies content
	 * of context onto display.
	 *
	 * If display is not buffered, this is no-op.
	 */
	void (*Flip)(struct GP_Backend *self);

	/*
	 * Updates display rectangle.
	 *
	 * In contrast to flip operation, the context
	 * must not change (this is intended for updating very small areas).
	 *
	 * If display is not buffered, this is no-op.
	 */
	void (*UpdateRect)(struct GP_Backend *self,
	                   GP_Coord x1, GP_Coord y1,
	                   GP_Coord x2, GP_Coord y2);

	/*
	 * Exits the backend.
	 */
	void (*Exit)(struct GP_Backend *self);

	/* 
	 * Linked List of file descriptors with callbacks to poll.
	 *
	 * May be NULL.
	 */
	GP_BackendFD *fd_list;

	/*
	 * Some of the backends doesn't expose file descriptor
	 * (for example SDL is broken that way).
	 *
	 * In that case the fd_list is NULL and this function
	 * is non NULL.
	 */
	void (*Poll)(struct GP_Backend *self);

	/* Backed private data */
	char priv[];
} GP_Backend;

#define GP_BACKEND_PRIV(backend) ((void*)(backend)->priv)

/*
 * Calls backend->Flip().
 */
static inline void GP_BackendFlip(GP_Backend *backend)
{
	backend->Flip(backend);
}

/*
 * Calls backend->UpdateRect().
 */
static inline void GP_BackendUpdateRect(GP_Backend *backend,
                                        GP_Coord x1, GP_Coord y1,
					GP_Coord x2, GP_Coord y2)
{
	backend->UpdateRect(backend, x1, y1, x2, y2);
}

/*
 * Calls backend->Exit().
 */
static inline void GP_BackendExit(GP_Backend *backend)
{
	backend->Exit(backend);
}

/*
 * Polls backend, the events are filled into event queue.
 */
static inline void GP_BackendPoll(GP_Backend *backend)
{
	backend->Poll(backend);
}

#endif /* BACKENDS_GP_BACKEND_H */
