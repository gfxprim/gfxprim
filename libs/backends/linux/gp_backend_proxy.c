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

	gp_ev_queue ev_queue;

	gp_fd fd;

	int visible;

	/* mapped memory backing the pixmap */
	void *map;
	size_t map_size;
};

static int proxy_set_attr(gp_backend *self, enum gp_backend_attrs attr, const void *vals)
{
	switch (attr) {
	case GP_BACKEND_TITLE:
//		gp_proxy_send(priv->fd.fd, GP_PROXY_NAME, vals);
	break;
	default:
	break;
	}

	return 0;
}

static void proxy_exit(gp_backend *self)
{
	struct proxy_priv *priv = GP_BACKEND_PRIV(self);

	gp_proxy_send(priv->fd.fd, GP_PROXY_EXIT, NULL);
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

	gp_proxy_send(priv->fd.fd, GP_PROXY_MAP, NULL);
}

static void visible(gp_backend *self)
{
	struct proxy_priv *priv = GP_BACKEND_PRIV(self);

	self->pixmap = &priv->shm_pixmap;

	priv->visible = 1;

	gp_ev_queue_push_resize(self->event_queue, self->pixmap->w, self->pixmap->h, 0);
}

static void hidden(gp_backend *self)
{
	struct proxy_priv *priv = GP_BACKEND_PRIV(self);

	//TODO: remap GP_PROXY_NONE?

	priv->visible = 0;

	self->pixmap = &priv->dummy;
}


static void unmap_buffer(gp_backend *self)
{
	struct proxy_priv *priv = GP_BACKEND_PRIV(self);

	munmap(priv->map, priv->map_size);

	priv->map = NULL;
	priv->map_size = 0;

//	hidden(self);

	gp_proxy_send(priv->fd.fd, GP_PROXY_UNMAP, NULL);
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

	gp_ev_queue_set_screen_size(self->event_queue, msg->pix.pix.w, msg->pix.pix.h);
}

static enum gp_poll_event_ret proxy_process_fd(gp_fd *self)
{
	gp_backend *backend = self->priv;
	struct proxy_priv *priv = GP_BACKEND_PRIV(backend);
	union gp_proxy_msg *msg;
	int ret;

	while ((ret = gp_proxy_buf_recv(priv->fd.fd, &priv->buf)) > 0) {
		while (gp_proxy_next(&priv->buf, &msg)) {
			switch (msg->type) {
			case GP_PROXY_PIXEL_TYPE:
				priv->dummy.pixel_type = msg->ptype.ptype;
			break;
			case GP_PROXY_EVENT:
				gp_ev_queue_put(backend->event_queue, &msg->ev.ev);
			break;
			case GP_PROXY_MAP:
				map_buffer(backend, msg);
			break;
			case GP_PROXY_UNMAP:
				unmap_buffer(backend);
			break;
			case GP_PROXY_PIXMAP:
				init_pixmap(backend, msg);
			break;
			case GP_PROXY_SHOW:
				visible(backend);
			break;
			case GP_PROXY_HIDE:
				hidden(backend);
			break;
			case GP_PROXY_CURSOR_POS:
				gp_ev_queue_set_cursor_pos(backend->event_queue,
				                           msg->cursor.pos.x,
				                           msg->cursor.pos.y);
			break;
			case GP_PROXY_EXIT:
				gp_ev_queue_push(backend->event_queue, GP_EV_SYS,
				                 GP_EV_SYS_QUIT, 0, 0);
			break;
			}
		}
	}

	if (ret == 0) {
		GP_WARN("Connection closed");
		gp_ev_queue_push(backend->event_queue, GP_EV_SYS, GP_EV_SYS_QUIT, 0, 0);
	}

	return 0;
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

	gp_proxy_send(priv->fd.fd, GP_PROXY_UPDATE, &rect);
}

static void proxy_flip(gp_backend *self)
{
	proxy_update_rect(self, 0, 0, self->pixmap->w-1, self->pixmap->h-1);
}

gp_backend *gp_proxy_init(const char *path, const char *title)
{
	int fd;
	gp_backend *ret;
	size_t size = sizeof(gp_backend) + sizeof(struct proxy_priv);

	ret = malloc(size);
	if (!ret) {
		GP_WARN("Malloc failed :-(");
		return NULL;
	}

	memset(ret, 0, size);

	fd = gp_proxy_client_connect(NULL);
	if (fd < 0) {
		free(ret);
		return NULL;
	}

	struct proxy_priv *priv = GP_BACKEND_PRIV(ret);

	priv->fd = (gp_fd) {
		.fd = fd,
		.event = proxy_process_fd,
		.events = GP_POLLIN,
		.priv = ret,
	};

	if (gp_poll_add(&ret->fds, &priv->fd)) {
		close(fd);
		free(ret);
		return NULL;
	}

	ret->name = "proxy";
	ret->set_attr = proxy_set_attr;
	ret->exit = proxy_exit;
	ret->update_rect = proxy_update_rect;
	ret->flip = proxy_flip;


	priv->map = NULL;
	priv->map_size = 0;
	priv->visible = 0;

	gp_proxy_buf_init(&priv->buf);

	ret->event_queue = &priv->ev_queue;

	gp_ev_queue_init(ret->event_queue, 1, 1, 0, 0);

	ret->pixmap = &priv->dummy;
	ret->pixmap->pixel_type = 0;

	gp_proxy_send(fd, GP_PROXY_NAME, title);

	/* Wait for the pixel type */
	while (!priv->dummy.pixel_type)
		gp_poll_wait(&ret->fds, -1);

	gp_pixmap_init(&priv->dummy, 0, 0, priv->dummy.pixel_type, NULL, 0);

	return ret;
}
