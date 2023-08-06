// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>

#include "core/gp_pixmap.h"
#include <core/gp_blit.h>
#include <core/gp_debug.h>

#include <backends/gp_backend_virtual.h>

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

	return 0;
}

static void virt_poll(gp_backend *self)
{
	struct virt_priv *virt = GP_BACKEND_PRIV(self);

	virt->backend->poll(virt->backend);

	gp_event *ev;

	while ((ev = gp_backend_get_event(virt->backend)))
		gp_ev_queue_put(self->event_queue, ev);
}

static void virt_wait(gp_backend *self)
{
	struct virt_priv *virt = GP_BACKEND_PRIV(self);

	virt->backend->wait(virt->backend);

	gp_event *ev;

	while ((ev = gp_backend_get_event(virt->backend)))
		gp_ev_queue_put(self->event_queue, ev);
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
	size_t size = sizeof(gp_backend) + sizeof(struct virt_priv);

	self = malloc(size);
	if (!self) {
		GP_DEBUG(1, "Malloc failed :(");
		return NULL;
	}

	memset(self, 0, size);

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
	self->fds = backend->fds;
	self->event_queue = backend->event_queue;

	return self;

err0:
	free(self);
	return NULL;
}
