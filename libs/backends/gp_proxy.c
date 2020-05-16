//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2019-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <core/gp_debug.h>
#include <core/gp_pixmap.h>
#include <backends/gp_backend.h>
#include <backends/gp_proxy_proto.h>
#include <backends/gp_proxy_conn.h>
#include <backends/gp_proxy.h>

struct proxy_priv {
	struct gp_proxy_buf buf;
	gp_pixmap dummy;

	gp_pixmap shm_pixmap;

	int visible;

	/* mapped memory backing the pixmap */
	void *map;
	size_t map_size;
};

static int proxy_set_attr(gp_backend *self, enum gp_backend_attrs attr, void *vals)
{
	switch (attr) {
	case GP_BACKEND_TITLE:
//		gp_proxy_send(self->fd, GP_PROXY_NAME, vals);
	break;
	default:
	break;
	}

	return 0;
}

static void proxy_exit(gp_backend *self)
{
	gp_proxy_send(self->fd, GP_PROXY_EXIT, NULL);
}

static void map_buffer(gp_backend *self, union gp_proxy_msg *msg)
{
	struct proxy_priv *priv = GP_BACKEND_PRIV(self);
	void *p;
	int fd;

	GP_DEBUG(1, "Mapping buffer '%s' size %zu",
	         msg->map.map.path, msg->map.map.size);

	fd = open(msg->map.map.path, O_RDWR);
	if (!fd) {
		GP_WARN("Invalid path for map event");
		return;
	}

	size_t size = msg->map.map.size;

	p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);

	if (p == MAP_FAILED) {
		GP_WARN("mmap() failed :-(");
		return;
	}

	priv->map = p;
	priv->map_size = msg->map.size;
}

static void unmap_buffer(gp_backend *self)
{
	struct proxy_priv *priv = GP_BACKEND_PRIV(self);

	munmap(priv->map, priv->map_size);

	priv->map = NULL;
	priv->map_size = 0;
}

static void init_pixmap(gp_backend *self, union gp_proxy_msg *msg)
{
	struct proxy_priv *priv = GP_BACKEND_PRIV(self);

	if (!priv->map) {
		GP_WARN("Buffer not mapped!");
		return;
	}

	priv->shm_pixmap = msg->pix.pix;
	priv->shm_pixmap.pixels = priv->map;

	GP_DEBUG(1, "Pixmap %ux%u initialized", msg->pix.pix.w, msg->pix.pix.h);

	//TODO: check that the buffer is large enough!

	gp_event_queue_set_screen_size(&self->event_queue, msg->pix.pix.w, msg->pix.pix.h);
}

static void visible(gp_backend *self)
{
	struct proxy_priv *priv = GP_BACKEND_PRIV(self);

	self->pixmap = &priv->shm_pixmap;

	priv->visible = 1;

	gp_event_queue_push_resize(&self->event_queue, self->pixmap->w, self->pixmap->h, NULL);
}

static void hidden(gp_backend *self)
{
	struct proxy_priv *priv = GP_BACKEND_PRIV(self);

	//TODO: remap GP_PROXY_NONE?

	priv->visible = 0;

	self->pixmap = &priv->dummy;
}

static void proxy_poll(gp_backend *self)
{
	struct proxy_priv *priv = GP_BACKEND_PRIV(self);
	union gp_proxy_msg *msg;
	int ret;

	while ((ret = gp_proxy_buf_recv(self->fd, &priv->buf)) > 0) {
		while (gp_proxy_next(&priv->buf, &msg)) {
			switch (msg->type) {
			case GP_PROXY_PIXEL_TYPE:
				priv->dummy.pixel_type = msg->ptype.ptype;
			break;
			case GP_PROXY_EVENT:
				gp_event_queue_put(&self->event_queue, &msg->ev.ev);
			break;
			case GP_PROXY_MAP:
				map_buffer(self, msg);
			break;
			case GP_PROXY_UNMAP:
				unmap_buffer(self);
			break;
			case GP_PROXY_PIXMAP:
				init_pixmap(self, msg);
			break;
			case GP_PROXY_SHOW:
				visible(self);
			break;
			case GP_PROXY_HIDE:
				hidden(self);
			break;
			case GP_PROXY_EXIT:
				gp_event_queue_push(&self->event_queue, GP_EV_SYS,
				                    GP_EV_SYS_QUIT, 0, NULL);
			break;
			}
		}
	}

	if (ret == 0) {
		GP_WARN("Connection closed");
		gp_event_queue_push(&self->event_queue, GP_EV_SYS, GP_EV_SYS_QUIT, 0, NULL);
	}
}

static void proxy_wait(gp_backend *self)
{
	struct pollfd fd = {.fd = self->fd, .events = POLLIN, .revents = 0};

	if (poll(&fd, 1, -1) > 0)
		proxy_poll(self);
}

static void proxy_update_rect(gp_backend *self, gp_coord x0, gp_coord y0,
                             gp_coord x1, gp_coord y1)
{
	struct proxy_priv *priv = GP_BACKEND_PRIV(self);

	if (!priv->visible)
		return;

	struct gp_proxy_rect_ rect = {
		.x = x0,
		.y = y0,
		.w = x1 - x0 + 1,
		.h = y1 - y0 + 1,
	};

	gp_proxy_send(self->fd, GP_PROXY_UPDATE, &rect);
}

static void proxy_flip(gp_backend *self)
{
	proxy_update_rect(self, 0, 0, self->pixmap->w-1, self->pixmap->h-1);
}

gp_backend *gp_proxy_init(const char *path, const char *title)
{
	int fd;
	gp_backend *ret = malloc(sizeof(gp_backend) + sizeof(struct proxy_priv));

	if (!ret) {
		GP_WARN("Malloc failed :-(");
		return NULL;
	}

	memset(ret, 0, sizeof(*ret));

	fd = gp_proxy_client_connect(NULL);
	if (fd < 0) {
		free(ret);
		return NULL;
	}

	ret->name = "proxy";
	ret->fd = fd;
	ret->set_attr = proxy_set_attr;
	ret->exit = proxy_exit;
	ret->wait = proxy_wait;
	ret->poll = proxy_poll;
	ret->update_rect = proxy_update_rect;
	ret->flip = proxy_flip;

	struct proxy_priv *priv = GP_BACKEND_PRIV(ret);

	priv->map = NULL;
	priv->map_size = 0;

	priv->visible = 0;

	gp_proxy_buf_init(&priv->buf);

	gp_event_queue_init(&ret->event_queue, 1, 1, 0);

	ret->pixmap = &priv->dummy;
	ret->pixmap->pixel_type = 0;

	gp_proxy_send(fd, GP_PROXY_NAME, title);

	/* Wait for the pixel type */
	while (!priv->dummy.pixel_type)
		proxy_wait(ret);

	gp_pixmap_init(&priv->dummy, 0, 0, priv->dummy.pixel_type, NULL);

	return ret;
}
