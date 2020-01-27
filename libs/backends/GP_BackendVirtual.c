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

#include "core/gp_pixmap.h"
#include <core/gp_blit.h>
#include <core/gp_debug.h>

#include "GP_BackendVirtual.h"

struct virt_priv {
	/* Original backend */
	gp_backend *backend;

	int flags;
};

static void virt_flip(gp_backend *self)
{
	struct virt_priv *virt = GP_BACKEND_PRIV(self);

	/* Convert and copy the buffer */
	gp_blit(self->pixmap, 0, 0, self->pixmap->w, self->pixmap->h,
		virt->backend->pixmap, 0, 0);

	/* Call blit on original backend */
	virt->backend->flip(virt->backend);
}

static void virt_update_rect(gp_backend *self, gp_coord x0, gp_coord y0,
                             gp_coord x1, gp_coord y1)
{
	struct virt_priv *virt = GP_BACKEND_PRIV(self);

	/* Convert and copy the buffer */
	gp_blit_xyxy(self->pixmap, x0, y0, x1, y1,
	             virt->backend->pixmap, x0, y0);

	/* Call blit on original backend */
	virt->backend->update_rect(virt->backend, x0, y0, x1, y1);
}

static int virt_set_attr(struct gp_backend *self,
                         enum gp_backend_attrs attr,
                         const void *vals)
{
	struct virt_priv *virt = GP_BACKEND_PRIV(self);

	int ret = virt->backend->set_attr(virt->backend, attr, vals);
	if (ret)
		return ret;

	if (attr == GP_BACKEND_SIZE) {
		gp_pixmap_resize(self->pixmap,
		                 ((const uint32_t *)vals)[0],
		                 ((const uint32_t *)vals)[1]);
	}

	return 0;
}

static void virt_poll(gp_backend *self)
{
	struct virt_priv *virt = GP_BACKEND_PRIV(self);

	virt->backend->poll(virt->backend);

	gp_event ev;

	while (gp_backend_get_event(virt->backend, &ev))
		gp_event_queue_put(&self->event_queue, &ev);
}

static void virt_wait(gp_backend *self)
{
	struct virt_priv *virt = GP_BACKEND_PRIV(self);

	virt->backend->wait(virt->backend);

	gp_event ev;

	while (gp_backend_get_event(virt->backend, &ev))
		gp_event_queue_put(&self->event_queue, &ev);
}

static void virt_exit(gp_backend *self)
{
	struct virt_priv *virt = GP_BACKEND_PRIV(self);

	gp_pixmap_free(self->pixmap);

	if (virt->flags & GP_BACKEND_CALL_EXIT)
		virt->backend->exit(virt->backend);

	free(self);
}

static int virt_resize_ack(gp_backend *self)
{
	struct virt_priv *virt = GP_BACKEND_PRIV(self);
	int ret;

	ret = virt->backend->resize_ack(virt->backend);

	if (ret)
		return ret;

	return gp_pixmap_resize(self->pixmap, virt->backend->pixmap->w,
				virt->backend->pixmap->h);
}

gp_backend *gp_backend_virt_init(gp_backend *backend,
                                 gp_pixel_type pixel_type,
                                 enum gp_backend_virt_flags flags)
{
	gp_backend *self;
	struct virt_priv *virt;

	self = malloc(sizeof(gp_backend) +
	              sizeof(struct virt_priv));

	if (self == NULL) {
		GP_DEBUG(1, "Malloc failed :(");
		return NULL;
	}

	memset(self, 0, sizeof(gp_backend));

	/* Create new buffer with different pixmap type */
	self->pixmap = gp_pixmap_alloc(backend->pixmap->w, backend->pixmap->h,
				       pixel_type);

	if (self->pixmap == NULL)
		goto err0;

	virt = GP_BACKEND_PRIV(self);
	virt->backend = backend;
	virt->flags = flags;

	/* Initalize new backend */
	self->update_rect = virt_update_rect;
	self->resize_ack = virt_resize_ack;
	self->set_attr = backend->set_attr ? virt_set_attr : NULL;
	self->name = "Virtual Backend";
	self->flip = virt_flip;
	self->poll = backend->poll ? virt_poll : NULL;
	self->wait = backend->wait ? virt_wait : NULL;
	self->exit = virt_exit;
	self->timers = NULL;

	gp_event_queue_init(&self->event_queue, backend->pixmap->w,
	                    backend->pixmap->h, 0);

	return self;

err0:
	free(self);
	return NULL;
}
