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

#include <string.h>

#include "core/GP_Pixmap.h"
#include "core/GP_Blit.h"
#include "core/GP_Debug.h"

#include "GP_BackendVirtual.h"

struct virt_priv {
	/* Original backend */
	GP_Backend *backend;

	int flags;
};

static void virt_flip(GP_Backend *self)
{
	struct virt_priv *virt = GP_BACKEND_PRIV(self);

	/* Convert and copy the buffer */
	GP_Blit(self->pixmap, 0, 0, self->pixmap->w, self->pixmap->h,
	        virt->backend->pixmap, 0, 0);

	/* Call blit on original backend */
	virt->backend->Flip(virt->backend);
}

static void virt_update_rect(GP_Backend *self, GP_Coord x0, GP_Coord y0,
                             GP_Coord x1, GP_Coord y1)
{
	struct virt_priv *virt = GP_BACKEND_PRIV(self);

	/* Convert and copy the buffer */
	GP_BlitXYXY(self->pixmap, x0, y0, x1, y1,
	            virt->backend->pixmap, x0, y0);

	/* Call blit on original backend */
	virt->backend->UpdateRect(virt->backend, x0, y0, x1, y1);
}

static int virt_set_attrs(struct GP_Backend *self,
                          uint32_t w, uint32_t h,
                          const char *caption)
{
	struct virt_priv *virt = GP_BACKEND_PRIV(self);
	int ret;

	ret = virt->backend->SetAttributes(virt->backend, w, h, caption);

	if (ret)
		return ret;

	/* If backend was resized, update our buffer as well */
	if (h != 0 && w != 0)
		GP_PixmapResize(self->pixmap, w, h);

	return 0;
}

static void virt_poll(GP_Backend *self)
{
	struct virt_priv *virt = GP_BACKEND_PRIV(self);

	virt->backend->Poll(virt->backend);

	struct GP_Event ev;

	while (GP_BackendGetEvent(virt->backend, &ev))
		GP_EventQueuePut(&self->event_queue, &ev);
}

static void virt_wait(GP_Backend *self)
{
	struct virt_priv *virt = GP_BACKEND_PRIV(self);

	virt->backend->Wait(virt->backend);

	struct GP_Event ev;

	while (GP_BackendGetEvent(virt->backend, &ev))
		GP_EventQueuePut(&self->event_queue, &ev);
}

static void virt_exit(GP_Backend *self)
{
	struct virt_priv *virt = GP_BACKEND_PRIV(self);

	GP_PixmapFree(self->pixmap);

	if (virt->flags & GP_BACKEND_CALL_EXIT)
		virt->backend->Exit(virt->backend);

	free(self);
}

static int virt_resize_ack(GP_Backend *self)
{
	struct virt_priv *virt = GP_BACKEND_PRIV(self);
	int ret;

	ret = virt->backend->ResizeAck(virt->backend);

	if (ret)
		return ret;

	return GP_PixmapResize(self->pixmap,
	                        virt->backend->pixmap->w,
	                        virt->backend->pixmap->h);
}

GP_Backend *GP_BackendVirtualInit(GP_Backend *backend,
                                  GP_PixelType pixel_type, int flags)
{
	GP_Backend *self;
	struct virt_priv *virt;

	self = malloc(sizeof(GP_Backend) +
	              sizeof(struct virt_priv));

	if (self == NULL) {
		GP_DEBUG(1, "Malloc failed :(");
		return NULL;
	}

	memset(self, 0, sizeof(GP_Backend));

	/* Create new buffer with different pixmap type */
	self->pixmap = GP_PixmapAlloc(backend->pixmap->w, backend->pixmap->h,
	                                pixel_type);

	if (self->pixmap == NULL)
		goto err0;

	virt = GP_BACKEND_PRIV(self);
	virt->backend = backend;
	virt->flags = flags;

	/* Initalize new backend */
	self->name          = "Virtual Backend";
	self->Flip          = virt_flip;
	self->UpdateRect    = virt_update_rect;
	self->Exit          = virt_exit;
	self->ResizeAck     = virt_resize_ack;
	self->Poll          = backend->Poll ? virt_poll : NULL;
	self->Wait          = backend->Wait ? virt_wait : NULL;
	self->SetAttributes = backend->SetAttributes ? virt_set_attrs : NULL;
	self->timers        = NULL;

	GP_EventQueueInit(&self->event_queue, backend->pixmap->w,
	                  backend->pixmap->h, 0);

	return self;

err0:
	free(self);
	return NULL;
}
