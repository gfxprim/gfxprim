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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include "core/GP_Common.h"
#include "core/GP_Transform.h"
#include "core/GP_Debug.h"

#include "backends/GP_Backend.h"

void GP_BackendFlip(GP_Backend *backend)
{
	if (backend->Flip != NULL)
		backend->Flip(backend);
}

void GP_BackendUpdateRectXYXY(GP_Backend *backend,
                              GP_Coord x0, GP_Coord y0,
                              GP_Coord x1, GP_Coord y1)
{
	if (backend->UpdateRect == NULL)
		return;

	GP_TRANSFORM_POINT(backend->context, x0, y0);
	GP_TRANSFORM_POINT(backend->context, x1, y1);

	if (x1 < x0)
		GP_SWAP(x0, x1);

	if (y1 < y0)
		GP_SWAP(y0, y1);

	if (x0 < 0) {
		GP_WARN("Negative x coordinate %i, clipping to 0", x0);
		x0 = 0;
	}
	
	if (y0 < 0) {
		GP_WARN("Negative y coordinate %i, clipping to 0", y0);
		y0 = 0;
	}

	GP_Coord w = backend->context->w;

	if (x1 >= w) {
		GP_WARN("Too large x coordinate %i, clipping to %u", x1, w - 1);
		x1 = w - 1;
	}
	
	GP_Coord h = backend->context->h;

	if (y1 >= h) {
		GP_WARN("Too large y coordinate %i, clipping to %u", y1, h - 1);
		y1 = h - 1;
	}
	
	backend->UpdateRect(backend, x0, y0, x1, y1);
}


int GP_BackendResize(GP_Backend *backend, uint32_t w, uint32_t h)
{
	if (backend->SetAttributes == NULL)
		return 1;
	
	if (w == 0)
		w = backend->context->w;

	if (h == 0)
		h = backend->context->h;

	if (backend->context->w == w && backend->context->h == h)
		return 0;

	return backend->SetAttributes(backend, w, h, NULL);
}

int GP_BackendResizeAck(GP_Backend *self)
{
	GP_DEBUG(2, "Calling backend %s ResizeAck()", self->name);
	
	if (self->ResizeAck)
		return self->ResizeAck(self);

	return 0;
}

int GP_BackendWaitEvent(GP_Backend *self, GP_Event *ev)
{
	int ret;

	for (;;) {
		if ((ret = GP_BackendGetEvent(self, ev)))
			return ret;
		
		GP_BackendWait(self);
	}
}

int GP_BackendPollEvent(GP_Backend *self, GP_Event *ev)
{
	int ret;
		
	if ((ret = GP_BackendGetEvent(self, ev)))
		return ret;

	GP_BackendPoll(self);
	
	if ((ret = GP_BackendGetEvent(self, ev)))
		return ret;

	return 0;
}
