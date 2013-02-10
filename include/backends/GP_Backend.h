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

/*

  The GP_Backend is overall structure for API for managing
  connection/mmaped memory/... to xserver window/framebuffer/... .

  In contrast to other graphics libraries we do not try to create unified
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

#include "input/GP_EventQueue.h"

struct GP_Backend;

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
	 * If display is not buffered, this is no-op (set to NULL).
	 */
	void (*Flip)(struct GP_Backend *self);

	/*
	 * Updates display rectangle.
	 *
	 * In contrast to flip operation, the context
	 * must not change (this is intended for updating very small areas).
	 *
	 * If display is not buffered, this is no-op (set to NULL).
	 */
	void (*UpdateRect)(struct GP_Backend *self,
	                   GP_Coord x0, GP_Coord y0,
	                   GP_Coord x1, GP_Coord y1);

	/*
	 * Callback to change attributes.
	 *
	 * If w and h are zero, only caption is changed.
	 *
	 * If w is set and h is zero, only w is changed and vice versa.
	 *
	 * If caption is NULL only w and h are changed.
	 *
	 * Use the inline wrappers instead.
	 */
	int (*SetAttributes)(struct GP_Backend *self,
	                     uint32_t w, uint32_t h,
	                     const char *caption);

	/*
	 * Resize acknowledge callback. This must be called
	 * after you got resize event in order to resize
	 * backend buffers.
	 */
	int (*ResizeAck)(struct GP_Backend *self);

	/*
	 * Exits the backend.
	 */
	void (*Exit)(struct GP_Backend *self);

	/* 
	 * Connection fd. Set to -1 if not available 
	 */
	int fd;

	/*
	 * Non-blocking event loop.
	 *
	 * The events are filled into the event queue see GP_Input.h.
	 */
	void (*Poll)(struct GP_Backend *self);

	/*
	 * Blocking event loop. Blocks until events are ready.
	 *
	 * Note that events received by a backend are not necessarily
	 * translated to input events. So input queue may be empty
	 * after Wait has returned.
	 *
	 * The events are filled into the event queue see GP_Input.h.
	 */
	void (*Wait)(struct GP_Backend *self);

	/*
	 * Queue to store input events.
	 */
	struct GP_EventQueue event_queue;

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
void GP_BackendUpdateRectXYXY(GP_Backend *backend,
                              GP_Coord x0, GP_Coord y0,
                              GP_Coord x1, GP_Coord y1);

static inline void GP_BackendUpdateRect(GP_Backend *backend,
                                        GP_Coord x0, GP_Coord y0,
                                        GP_Coord x1, GP_Coord y1)
{
	return GP_BackendUpdateRectXYXY(backend, x0, y0, x1, y1);
}

static inline void GP_BackendUpdateRectXYWH(GP_Backend *backend,
                                            GP_Coord x, GP_Coord y,
                                            GP_Size w, GP_Size h)
{
	GP_BackendUpdateRectXYXY(backend, x, y, x + w, y + h);
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

/*
 * Waits for backend events.
 */
static inline void GP_BackendWait(GP_Backend *backend)
{
	backend->Wait(backend);
}

/*
 * Sets backend caption, if supported.
 *
 * When setting caption is not possible/implemented non zero is returned.
 */
static inline int GP_BackendSetCaption(GP_Backend *backend,
                                       const char *caption)
{
	if (backend->SetAttributes == NULL)
		return 1;

	return backend->SetAttributes(backend, 0, 0, caption);
}

/*
 * Resize backend, if supported.
 *
 * When resizing is not possible/implemented non zero is returned.
 *
 * When the backend size matches the passed width and height,
 * no action is done.
 *
 * Note that after calling this, the backend->context pointer may change.
 */
int GP_BackendResize(GP_Backend *backend, uint32_t w, uint32_t h);


/*
 * Resize acknowledge. You must call this right after you application has
 * received resize event.
 *
 * This will resize backend buffers. After this call returns the backend width
 * height and context pointer are most likely different.
 *
 * This function returns zero on succes. Non zero on failure. If it fails the
 * best action to take is to save application data and exit (as the backend
 * may be in undefined state).
 */
int GP_BackendResizeAck(GP_Backend *self);

/*
 * Event Queue functions.
 */
static inline unsigned int GP_BackendEventsQueued(GP_Backend *self)
{
	return GP_EventQueueEventsQueued(&self->event_queue);
}

static inline int GP_BackendGetEvent(GP_Backend *self, GP_Event *ev)
{
	return GP_EventQueueGet(&self->event_queue, ev);
}

#endif /* BACKENDS_GP_BACKEND_H */
